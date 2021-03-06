#include "LowpassFilter.cuh"
#include "Common.h"
#include "gl\glew.h"
#include <cuda_gl_interop.h>

//@ Butterworth lowpass filter. Almost same result(function shape) with bilateral filter.
__global__ void d_butterworth(cufftComplex *data, int width, int height, float radius){
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int offset = x + y * ((width >> 1) + 1);

	if (x < (width >> 1) + 1 && y < height){
		int fx = x + (width >> 1);
		if (fx > width){
			fx -= width;
		}
		int fy = y + (height >> 1);
		if (fy > height){
			fy -= height;
		}
		float r = sqrtf((float)(((width >> 1) - fx) * ((width >> 1) - fx) + ((height >> 1) - fy) * ((height >> 1) - fy)));
		float t = r / radius;
		float B = 1 / (1 + t * t * t * t);

		data[offset].x *= B;
		data[offset].y *= B;
	}
}

//@ Normalize the data.
__global__ void d_normalize(float *data, int width, int height){
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int offset = x + y * width;
	int imageSize = width * height;

	if (offset < imageSize){
		data[offset] /= (float)imageSize;
	}
}

LowpassFilter::LowpassFilter(int width, int height, float radius, bool normalize)
	: m_dev_complex(NULL), m_plan_fwd(0), m_plan_inv(0),
	m_width(width), m_height(height),
	m_radius(radius),
	m_normalize(normalize),
	m_grids2D((width + 16 - 1) / 16, (height + 16 - 1) / 16), m_threads2D(16, 16),
	m_dev_data(NULL)
{
	checkCUFFT(cufftPlan2d(&m_plan_fwd, height, width, CUFFT_R2C),
		"Error : Allocating cufft foward plan");
	checkCUFFT(cufftPlan2d(&m_plan_inv, height, width, CUFFT_C2R),
		"Error : Allocating cufft invers plan");
	checkCUDA(cudaMalloc((void **)&m_dev_complex, sizeof(cufftComplex) * (width / 2 + 1) * height),
		"Error : Allocating device memory for cufft");
}

LowpassFilter::~LowpassFilter() {
	/* This will be needed if cudaDeviceReset() would not be called */
	/*
	if (m_dev_complex != NULL)	checkCUDA(cudaFree(m_dev_complex), "Error : Deallocating device memory for cufft");
	if (m_plan_fwd != 0)		checkCUFFT(cufftDestroy(m_plan_fwd), "Error : Deallocating cufft foward plan");
	if (m_plan_inv != 0)		checkCUFFT(cufftDestroy(m_plan_inv), "Error : Deallocating cufft inverse plan");
	if (m_dev_data != NULL)
	{
		checkCUDA(cudaFree(m_dev_data), "Error : Deallocating device memory for cufft");
		checkCUDA(cudaGraphicsUnregisterResource(m_graphicResource), "Error : Unregistering GL resource.");
	}
	*/
}

void
LowpassFilter::run(float *dev_data) {
	// FFT forward.
	checkCUFFT(cufftExecR2C(m_plan_fwd, (cufftReal *)dev_data, m_dev_complex),
		"Error : Executing cufft foward");

	// Butterworth lowpass filter.
	d_butterworth << < m_grids2D, m_threads2D >> > (m_dev_complex, m_width, m_height, m_radius);
	checkCUDA(cudaGetLastError(),
		"Error : Butterworth kernel function");

	// FFT inverse.
	checkCUFFT(cufftExecC2R(m_plan_inv, m_dev_complex, (cufftReal *)dev_data),
		"Error : Executing cufft inverse");

	// CUFFT does not normalize the result, so it should be done manually.
	if (m_normalize){
		d_normalize << <m_grids2D, m_threads2D >> > (dev_data, m_width, m_height);
		checkCUDA(cudaGetLastError(), "Error : Normalizing the result of the lowpass filter.");
	}
}

void
LowpassFilter::setTexture(unsigned int textureID)
{
	// Register GL textures to CUDA.
	checkCUDA(cudaMalloc((void **)&m_dev_data, sizeof(float) * m_width * m_height), "Malloc device memory.");
	checkCUDA(cudaGraphicsGLRegisterImage(&m_graphicResource, textureID, GL_TEXTURE_2D, cudaGraphicsMapFlagsNone),
		"Register GL texture.");
}

void LowpassFilter::run()
{
	// Map device memory between GL and CUDA.
	cudaArray *arr_tex = NULL;
	checkCUDA(cudaGraphicsMapResources(1, &m_graphicResource),
		"Mapping graphic resources.");
	checkCUDA(cudaGraphicsSubResourceGetMappedArray(&arr_tex, m_graphicResource, 0, 0),
		"Getting mapped cudaArray for thickness texture.");

	checkCUDA(cudaMemcpyFromArray(m_dev_data, arr_tex, 0, 0, m_width * m_height * sizeof(float), cudaMemcpyDeviceToDevice),
		"Memcpy from arr_tex to device memory.");

	// Run
	run(m_dev_data);

	// Write to GL texture from CUDA memory.
	checkCUDA(cudaMemcpyToArray(arr_tex, 0, 0, m_dev_data, m_width * m_height* sizeof(float), cudaMemcpyDeviceToDevice),
		"Memcpy from device memory to arr_tex");

	// Unmap device memory.
	checkCUDA(cudaGraphicsUnmapResources(1, &m_graphicResource),
		"Error : Unmapping graphic resources");
}