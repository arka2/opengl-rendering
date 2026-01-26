#include <iostream>         // error handling and output
#include <cstdlib>          // EXIT_FAILURE

#include <GL/glew.h>        // GLEW library
#include "GLFW/glfw3.h"     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

// Namespace for declaring global variables
namespace
{
	// Macro for window title
	const char* const WINDOW_TITLE = "7-1 FinalProject and Milestones"; 

	// Main GLFW window
	GLFWwindow* g_Window = nullptr;

	// scene manager object for managing the 3D scene prepare and render
	SceneManager* g_SceneManager = nullptr;
	// shader manager object for dynamic interaction with the shader code
	// view manager object for managing the 3D view setup and projection to 2D
	ViewManager* g_ViewManager = nullptr;
}

// Function declarations - all functions that are called manually
// need to be pre-declared at the beginning of the source code.
bool InitializeGLFW();
bool InitializeGLEW();


/***********************************************************
 *  main(int, char*)
 *
 *  This function gets called after the application has been
 *  launched.
 ***********************************************************/
int main(int argc, char* argv[])
{
	// if GLFW fails initialization, then terminate the application
	if (InitializeGLFW() == false)
	{
		return(EXIT_FAILURE);
	}
	
	// try to create a new shader manager object
	//g_ShaderManager = new ShaderManager();
	// try to create a new view manager object
	g_ViewManager = new ViewManager();

	// try to create the main display window
	g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);

	// if GLEW fails initialization, then terminate the application
	if (InitializeGLEW() == false)
	{
		return(EXIT_FAILURE);
	}

	// load the shader code from the external GLSL files
	//g_ShaderManager->LoadShaders(
	//	"../../Utilities/shaders/vertexShader.glsl",
	//	// "../../Utilities/shaders/fragmentShader.glsl");
	//	// Comment out above line and uncomment below line (and vice versa) to use other frag shader
	//	"Source/fragShader.glsl");
	//ShaderManager shader("../../Utilities/shaders/vertexShader.glsl", "Source/fragShader.glsl");

	// Attempt at trying to use the working shaders from OpenGL tutorial
	// needs to have shadowMap and diffuseTexture set in while loop
	//g_ShaderManager->LoadShaders(
	//	"Source/shaders/3.1.3.shadow_mapping.vs",
	//	// "../../Utilities/shaders/fragmentShader.glsl");
	//	// Comment out above line and uncomment below line (and vice versa) to use other frag shader
	//	"Source/shaders/3.1.3.shadow_mapping.fs");
	ShaderManager shader("../../Utilities/shaders/vertexShader.glsl", "Source/fragShader.glsl");
	shader.use();

	// Moved to outside 
	glEnable(GL_DEPTH_TEST);

	// configure depth map FBO
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// attach depth texture as FBO framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Load shaders for depth map and debugging
	ShaderManager simpleDepthShader("Source/shaders/depthVertexShader.glsl", "Source/shaders/depthFragShader.glsl");
	ShaderManager debugDepthQuad("Source/shaders/debugQuadVertexShader.glsl", "Source/shaders/debugQuadFragShader.glsl");

	// try to create a new scene manager object and prepare the 3D scene
	g_SceneManager = new SceneManager();
	g_SceneManager->PrepareScene(shader);

	// Calculate lightspace matrix for shaders
	float near_plane = 0.0f, far_plane = 9.0f;

	glm::mat4 lightProjection = glm::ortho(
		-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

	glm::mat4 lightView = glm::lookAt(
		glm::vec3(-10.0f, 4.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	// Pass in matrix to main shader and depth shader
	shader.use();
	shader.setMat4Value("lightSpaceMatrix", lightSpaceMatrix);

	simpleDepthShader.use();
	simpleDepthShader.setMat4Value("lightSpaceMatrix", lightSpaceMatrix);

	// Render to depth map
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Load scene textures, including depth map
	shader.use();
	g_SceneManager->LoadSceneTextures(depthMap, shader);
	// FIXME: Try to assign depth map to main shader -- DOESN'T DO ANYTHING
	unsigned int depthMapID = g_SceneManager->GetDepthMapSlot();
	shader.setSampler2DValue("depthMap", depthMapID);

	// Uncomment this when using tutorial shader
	//g_ShaderManager->setSampler2DValue("shadowMap", depthMapID);
	//int textureID = g_SceneManager->FindTextureSlot("puzzle");
	//g_ShaderManager->setSampler2DValue("diffuseTexture", textureID);

	// loop will keep running until the application is closed 
	// or until an error has occurred
	while (!glfwWindowShouldClose(g_Window))
	{

		// Clear the frame and z buffers
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// convert from 3D object space to 2D view
		g_ViewManager->PrepareSceneView(shader);

		// refresh the 3D scene
		shader.use();
		g_SceneManager->RenderScene(shader, simpleDepthShader);

		// render Depth map to quad for visual debugging
		// ---------------------------------------------
		//debugDepthQuad.use();
		//debugDepthQuad.setFloatValue("near_plane", near_plane);
		//debugDepthQuad.setFloatValue("far_plane", far_plane);
		//debugDepthQuad.setSampler2DValue("depthMap", depthMapID);
		//g_SceneManager->renderQuad();

		// Flips the the back buffer with the front buffer every frame.
		glfwSwapBuffers(g_Window);

		// query the latest GLFW events
		glfwPollEvents();
	}

	// clear the allocated manager objects from memory
	if (NULL != g_SceneManager)
	{
		delete g_SceneManager;
		g_SceneManager = NULL;
	}
	if (NULL != g_ViewManager)
	{
		delete g_ViewManager;
		g_ViewManager = NULL;
	}
	//if (NULL != g_ShaderManager)
	//{
	//	delete g_ShaderManager;
	//	g_ShaderManager = NULL;
	//}

	// Terminates the program successfully
	exit(EXIT_SUCCESS); 
}

/***********************************************************
 *	InitializeGLFW()
 * 
 *  This function is used to initialize the GLFW library.   
 ***********************************************************/
bool InitializeGLFW()
{
	// GLFW: initialize and configure library
	// --------------------------------------
	glfwInit();

#ifdef __APPLE__
	// set the version of OpenGL and profile to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	// set the version of OpenGL and profile to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	// GLFW: end -------------------------------

	return(true);
}

/***********************************************************
 *	InitializeGLEW()
 *
 *  This function is used to initialize the GLEW library.
 ***********************************************************/
bool InitializeGLEW()
{
	// GLEW: initialize
	// -----------------------------------------
	GLenum GLEWInitResult = GLEW_OK;

	// try to initialize the GLEW library
	GLEWInitResult = glewInit();
	if (GLEW_OK != GLEWInitResult)
	{
		std::cerr << glewGetErrorString(GLEWInitResult) << std::endl;
		return false;
	}
	// GLEW: end -------------------------------

	// Displays a successful OpenGL initialization message
	std::cout << "INFO: OpenGL Successfully Initialized\n";
	std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;

	return(true);
}