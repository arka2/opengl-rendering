///////////////////////////////////////////////////////////////////////////////
// shadermanager.h
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	EDITED BY Arka Tu
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"
// Added to include half cylinder mesh without editing ShapeMeshes.h and ShapeMeshes.cpp
#include "HalfCylinder.h"
// Added to allow use of multiple textures on different faces of a box
#include "BoxAlbumTextures.h"
#include "BoxPuzzleTextures.h"

#include <string>
#include <vector>

#include "Shader.h"

/***********************************************************
 *  SceneManager
 *
 *  This class contains the code for preparing and rendering
 *  3D scenes, including the shader settings.
 ***********************************************************/
class SceneManager
{
public:
	// constructor
	SceneManager();
	// destructor
	~SceneManager();

	struct TEXTURE_INFO
	{
		std::string tag;
		uint32_t ID;
	};

	struct OBJECT_MATERIAL
	{
		float ambientStrength;
		glm::vec3 ambientColor;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;
		std::string tag;
	};

	// enum for different texture wrapping options
	enum Wrapping
	{
		repeat,
		mirrored_repeat,
		clamp_to_edge,
		clamp_to_border
	};

private:
	// pointer to basic shapes object
	ShapeMeshes* m_basicMeshes;
	// Added -- pointer to half cylinder object
	HalfCylinder* m_halfCylinder;
	// Added -- pointer to box object with multiple textures
	BoxAlbumTextures* m_boxAlbumTextures;
	// Added -- pointer to box object with puzzle textures
	BoxPuzzleTextures* m_boxPuzzleTextures;
	// total number of loaded textures
	int m_loadedTextures;
	// loaded textures info
	TEXTURE_INFO m_textureIDs[16];
	// defined object materials
	std::vector<OBJECT_MATERIAL> m_objectMaterials;

	// load texture images and convert to OpenGL texture data
	// edited to take extra parameter for texture wrapping
	bool CreateGLTexture(const char* filename, std::string tag, enum Wrapping wrapping);
	// bind loaded OpenGL textures to slots in memory
	void BindGLTextures();
	// free the loaded OpenGL textures
	void DestroyGLTextures();
	// find a loaded texture by tag
	int FindTextureID(std::string tag);
	
	// find a defined material by tag
	bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);
	// load textures from directory
	
	// Configure material settings
	void DefineObjectMaterials();

	void SetupSceneLights(const ShaderManager &shader);

	// set the transformation values 
	// into the transform buffer
	void SetTransformations(
		glm::vec3 scaleXYZ,
		float XrotationDegrees,
		float YrotationDegrees,
		float ZrotationDegrees,
		glm::vec3 positionXYZ,
		const ShaderManager &shader);
	// set the texture data into the shader
	void SetShaderTexture(
		std::string textureTag, const ShaderManager &shader);

	void SetDepthMapTexture(const ShaderManager &shader);

public:

	// The following methods are for the students to 
	// customize for their own 3D scene
	void PrepareScene(const ShaderManager &shader);
	void RenderScene(const ShaderManager &mainShader, const ShaderManager &depthShader);

	// methods for rendering the various objects in the 3D scene
	void RenderTable(const ShaderManager& mainShader, const ShaderManager &depthShader);
	void RenderPuzzleBox(const ShaderManager& mainShader, const ShaderManager &depthShader);

	// methods for rendering shadows
	void renderQuad();
	void LoadDepthMapTexture(unsigned int &depthmap);
	int FindTextureSlot(std::string tag);
	unsigned int GetDepthMapSlot();

	void LoadSceneTextures(unsigned int &depthmap, const ShaderManager &shader);

};