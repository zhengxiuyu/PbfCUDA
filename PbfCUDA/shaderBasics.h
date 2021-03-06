
#ifndef _SHADER_BASICS_H_
#define	_SHADER_BASICS_H_

#ifdef  __APPLE__
	#include <GL/glew.h>
#else
	#include <Windows.h>
	#include "gl\glew.h"

	#pragma comment(lib, "cufft")
	#pragma comment(lib, "gl\\glew32.lib") 
	#pragma comment(lib, "opengl32.lib")
	#pragma comment(lib, "gl\\glfw3dll.lib")
	#pragma comment(lib, "FreeImage\\FreeImage.lib")
#endif

//#include <GLUT/glut.h> 
#include "gl\glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/constants.hpp"

// Functions for setting up OpenGL window on top of GLFW
GLFWwindow* initOpenGL(int argc, char* argv[],
	float screenScale, int& screenW, int& screenH, int& windowW, int& windowH);

void	printMat4(glm::mat4 &pMat);
void	printVec4(glm::vec4 &pVec);
char*	textFileRead(char *fn);
void	printShaderInfoLog(GLuint obj, char* shaderName);
void	printProgramInfoLog(GLuint obj);
GLuint	setShaders(const char* vShaderFileName, const char* fShaderFileName);
bool	checkOpenGL(const char* message, const char* file, int line, bool exitOnError, bool report);

#define PER255F 0.003921568627f	//@ /255.0f

//@ Base class for OpenGL wrapper classes.
class GLObject {
protected:
	GLuint m_id;
public:
	virtual		~GLObject() { }
	GLuint		getID() const		{ return m_id; }
};

#endif	// _SHADER_BASICS_H_
