///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	EDITED BY Arka Tu
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>
#include "Shader.h"

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager()
{
	m_basicMeshes = new ShapeMeshes();
	// Added for using half cylinder without editing ShapeMeshes
	m_halfCylinder = new HalfCylinder();
	// Added for using box with multiple textures
	m_boxAlbumTextures = new BoxAlbumTextures();
	// Added for using box with puzzle textures
	m_boxPuzzleTextures = new BoxPuzzleTextures();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	delete m_basicMeshes;
	m_basicMeshes = NULL;
	// Added for using half cylinder without editing ShapeMeshes
	delete m_halfCylinder;
	m_halfCylinder = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag, enum Wrapping wrapping = repeat)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Edited to allow other texture wrapping options
		switch (wrapping) {
			case mirrored_repeat:
				// mirrored repeat
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
				break;
			case clamp_to_edge:
				// clamp to edge
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				break;
			case clamp_to_border:
				// clamp to border
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				break;
			default:
				// repeat
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				break;
		}

		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
		//std::cout << m_textureIDs[i].ID << std::endl;
		//std::cout << m_textureIDs[i].tag << std::endl;
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  LoadSceneTextures()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::LoadSceneTextures(unsigned int &depthmap, const ShaderManager &shader)
{

	bool bReturn = false;

	bReturn = CreateGLTexture(
		"Textures/album_back.jpg",
		"album_back");

	bReturn = CreateGLTexture(
		"Textures/album_atlas.jpg",
		"album");

	// Mirrored repeat wrapping
	bReturn = CreateGLTexture(
		"Textures/album_pages.jpg",
		"album_pages", mirrored_repeat);

	bReturn = CreateGLTexture(
		"Textures/marble.png",
		"marble");

	bReturn = CreateGLTexture(
		"Textures/cork.png",
		"cork");

	bReturn = CreateGLTexture(
		"Textures/puzzle_atlas_02.jpg",
		"puzzle");

	// Added to load depthMap alongside other textures
	LoadDepthMapTexture(depthmap);
	SetDepthMapTexture(shader);

	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for configuring the various material
 *  settings for all of the objects within the 3D scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL glassMaterial;
	glassMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	glassMaterial.ambientStrength = 0.3f;
	glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	glassMaterial.specularColor = glm::vec3(0.6f, 0.6f, 0.6f);
	glassMaterial.shininess = 85.0;
	glassMaterial.tag = "glass";

	m_objectMaterials.push_back(glassMaterial);

	OBJECT_MATERIAL corkMaterial;
	corkMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	corkMaterial.ambientStrength = 0.3f;
	corkMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	corkMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
	corkMaterial.shininess = 0.5;
	corkMaterial.tag = "cork";

	m_objectMaterials.push_back(corkMaterial);

	OBJECT_MATERIAL puzzleMaterial;
	puzzleMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	puzzleMaterial.ambientStrength = 0.6f;
	puzzleMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	puzzleMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
	puzzleMaterial.shininess = 0.5;
	puzzleMaterial.tag = "puzzle";

	m_objectMaterials.push_back(puzzleMaterial);

	OBJECT_MATERIAL clothMaterial;
	clothMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	clothMaterial.ambientStrength = 0.6f;
	clothMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	clothMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
	clothMaterial.shininess = 0.5;
	clothMaterial.tag = "cloth";

	m_objectMaterials.push_back(clothMaterial);

	OBJECT_MATERIAL plasticMaterial;
	plasticMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	plasticMaterial.ambientStrength = 0.6f;
	plasticMaterial.diffuseColor = glm::vec3(0.55f, 0.55f, 0.55f);
	plasticMaterial.specularColor = glm::vec3(0.7f, 0.7f, 0.7f);
	plasticMaterial.shininess = 0.25;
	plasticMaterial.tag = "plastic";

	m_objectMaterials.push_back(plasticMaterial);

	// TODO: Adjust marble material properties
	OBJECT_MATERIAL marbleMaterial;
	marbleMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	marbleMaterial.ambientStrength = 0.6f;
	marbleMaterial.diffuseColor = glm::vec3(0.7f, 0.4f, 0.4f);
	marbleMaterial.specularColor = glm::vec3(0.296648f, 0.296648f, 0.296648f);
	marbleMaterial.shininess = 0.2;
	marbleMaterial.tag = "marble";

	m_objectMaterials.push_back(marbleMaterial);
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights(const ShaderManager &shader)
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting - to use the default rendered 
	// lighting then comment out the following line
	shader.setBoolValue(g_UseLightingName, true);

	shader.setVec3Value("lightSources[0].position", -10.0f, 4.0f, 0.0f);
	shader.setVec3Value("lightSources[0].ambientColor", 0.4296875f, 0.55859375f, 0.6484375f);
	shader.setVec3Value("lightSources[0].diffuseColor", 1.0f, 0.83203125f, 0.1484375f);
	shader.setVec3Value("lightSources[0].specularColor", 1.0f, 0.83203125f, 0.1484375f);
	shader.setFloatValue("lightSources[0].focalStrength", 1.0f);
	shader.setFloatValue("lightSources[0].specularIntensity", 0.1f);

	// FIXME: Changed -- Commented out ambient light while debugging shadow mapping
	//m_pShaderManager->setVec3Value("lightSources[1].position", 6.0f, 8.0f, 20.0f);
	//m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.01f, 0.01f, 0.01f);
	//m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.37890625f, 0.41796875f, 1.0f);
	//m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.37890625f, 0.41796875f, 1.0f);
	//m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 32.0f);
	//m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.2f);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ,
	const ShaderManager &shader)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	shader.setMat4Value(g_ModelName, modelView);
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag, const ShaderManager &shader)
{
	shader.setIntValue(g_UseTextureName, true);

	int textureID = -1;
	textureID = FindTextureSlot(textureTag);
	shader.setSampler2DValue(g_TextureValueName, textureID);
}

void SceneManager::SetDepthMapTexture(const ShaderManager &shader)
{
	int textureID = -1;
	textureID = FindTextureSlot("depthMap");
	shader.setSampler2DValue("depthMap", textureID);
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/


/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene(const ShaderManager &shader)
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene
	SetupSceneLights(shader);
	DefineObjectMaterials();

	// Changed -- Removed LoadSceneTextures() to ensure depth map is rendered after meshes are loaded

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	// Added for using half cylinder without editing ShapeMeshes
	m_halfCylinder->LoadCylinderMesh();
	// Added for using box mesh with multiple textures
	m_boxAlbumTextures->LoadBoxMesh();
	// Added for using box mesh with multiple textures
	m_boxPuzzleTextures->LoadBoxMesh();

}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene(const ShaderManager &mainShader, const ShaderManager &depthShader)
{
	// Call functions to render each object
	RenderTable(mainShader, depthShader);
	RenderPuzzleBox(mainShader, depthShader);
}

/***********************************************************
 *  RenderTable()
 *
 *  This method is called to render the shapes for the table
 *  object.
 ***********************************************************/
void SceneManager::RenderTable(const ShaderManager &mainShader, const ShaderManager &depthShader)
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 3.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ,
		depthShader);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ,
		mainShader);

	SetShaderTexture("marble", mainShader);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
}

/***********************************************************
 *  RenderPuzzleBox()
 *
 *  This method is called to render the shapes for the puzzle
 *  box object.
 ***********************************************************/
void SceneManager::RenderPuzzleBox(const ShaderManager& mainShader, const ShaderManager& depthShader)
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// Add vector to position of each mesh to move entire object same amount
	glm::vec3 puzzleBoxPositionXYZ = glm::vec3(1.5f, 0.0f, 6.0f);
	/****************************************************************/
	// Box -- lower section
	/****************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(5.0f, 0.3f, 5.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 24.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.15f, 0.0f);
	// add object position to mesh position
	positionXYZ += puzzleBoxPositionXYZ;

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ,
		depthShader);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ,
		mainShader);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	// Box -- upper section
	/****************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(5.1f, 1.3f, 5.1f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 24.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.8f, 0.0f);
	// add object position to mesh position
	positionXYZ += puzzleBoxPositionXYZ;

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ,
		depthShader);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ,
		mainShader);

	SetShaderTexture("puzzle", mainShader);

	// draw the mesh with transformation values
	m_boxPuzzleTextures->DrawBoxMesh();
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void SceneManager::renderQuad() {

	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void SceneManager::LoadDepthMapTexture(unsigned int &depthmap) {

	// register the loaded texture and associate it with the special tag string
	m_textureIDs[m_loadedTextures].ID = depthmap;
	m_textureIDs[m_loadedTextures].tag = "depthMap";
	m_loadedTextures++;
}

unsigned int SceneManager::GetDepthMapSlot() {
	return FindTextureSlot("depthMap");
}