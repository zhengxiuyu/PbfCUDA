#pragma once

#include <vector>
#include "temp.h"
#include "glm\glm.hpp"

class LowpassFilter;

class BilateralFilter{
protected:
	int m_width, m_height;

	int m_radius, m_nIterations;
	float m_sigma_s, m_sigma_r;

	LowpassFilter *m_lowpassFilter;
	glm::mat4 m_P;

	BilateralFilter(int width, int height,
		GLuint depthTexID, int bf_radius, int bf_sigma_s, float bf_sigma_r, int bf_nIterations,
		GLuint outlineTexID, int dog_radius, int dog_sigma, float dog_similarity,
		GLuint thicknessTexID);

	// remove later
	int m_restorePos;
	float m_intensityRange;
public:
	static BilateralFilter* create(int width, int height,
		GLuint depthTexID, int bf_radius, int bf_sigma_s, float bf_sigma_r, int bf_nIterations,
		GLuint outlineTexID, int dog_radius, int dog_sigma, float dog_similarity,
		GLuint thicknessTexID)
	{
		return new BilateralFilter(width, height, depthTexID, bf_radius, bf_sigma_s, bf_sigma_r, bf_nIterations, outlineTexID, dog_radius, dog_sigma, dog_similarity, thicknessTexID);
	}

	~BilateralFilter();

	void filter(bool renderOutline);
	void setBFParam(int radius, int sigma_s, float sigma_r, int nIterations);
	void setDOGParam(int radius, int sigma, float similarity);
	void setDepthTexture(GLuint depthTexID);
	void setOutlineTexture(GLuint outlineTexID);
	void setThicknessTexture(GLuint thicknessTexID);
	void setProjectionMatrix(glm::mat4 matrix);
	void setRestorePos(int restorePos) { m_restorePos = restorePos; }
	void setSigmaS(float sigmaS){ m_bf_sigma_s = sigmaS; }
	void setIntensityRange(float zNear, float zFar){
		//if (m_restorePos){
		//}
		m_intensityRange = abs(zFar - zNear);
		m_bf_sigma_r = m_bf_sigma_r * m_intensityRange;


		//float viewFrustumWidth =0;
		//float viewFrustumHeight=0;

		//if (m_restorePos){
		//	//glm::vec4 v(1, -1, m_P[0][0], 1);
		//	//float w_c = m_P[3][2] / (m_P[2][2] + v.z);
		//	//glm::vec4 v_eye = glm::inverse(m_P) * (w_c * v);
		//	//float viewFrustumWidth = v_eye.x - v_eye.y;


		//	float temp = m_P[3][2];
		//	temp = m_P[2][2];
		//	viewFrustumWidth = 2.0f * ((zFar + zNear) * 0.5f) / m_P[0][0];
		//	viewFrustumHeight = 2.0f * ((zFar + zNear) * 0.5f) / m_P[1][1];
		//	//viewFrustumWidth = zNear / m_P[0][0];
		//	//viewFrustumHeight = zNear / m_P[1][1];

		//	m_sigma_s_x = (m_bf_sigma_s / (float)m_width) * viewFrustumWidth;
		//	m_sigma_s_y = (m_bf_sigma_s / (float)m_height) * viewFrustumHeight;

		//	//printf("width : %.4f, sigma_s : %.4f, sigam_s^ : %.4f\n, 1/width : %.4f", viewFrustumWidth, (float)m_bf_sigma_s, (m_bf_sigma_s / 1024.0f) * viewFrustumWidth, 1.0f / viewFrustumWidth);
		//	//m_bf_sigma_s = (m_bf_sigma_s / 1024.0f) * viewFrustumWidth;
		//}
		//else{
		//	m_sigma_s_x = m_bf_sigma_s;
		//	m_sigma_s_y = m_bf_sigma_s;
		//}
		//printf("radius = %.3f, width = %.3f, sigmaS : %.3f, sigmaSX : %.3f, sigmaSY : %.3f, sigmaR : %.3f\n", (m_bf_radius / (float)m_width) * viewFrustumWidth, viewFrustumWidth, m_bf_sigma_s, m_sigma_s_x, m_sigma_s_y, m_bf_sigma_r);

		//cout << v_eye.x << ", " << v_eye.y << ", " << v_eye.z << ", " << v_eye.w << endl;
	}

	void filterThickness();
};