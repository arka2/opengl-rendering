///////////////////////////////////////////////////////////////////////////////
// BoxMultipleTextures.h
// ============
// create mesh for box that uses texture atlas for puzzle box
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	EDITED BY Arka Tu
//	Created for CS-330-Computational Graphics and Visualization, Nov. 7th, 2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>

/***********************************************************
 *  ShapeMeshes
 *
 *  This class contains the code for defining the various
 *  basic 3D shapes, loading into memory, and drawing
 ***********************************************************/
class BoxPuzzleTextures
{
public:
	// constructor
	BoxPuzzleTextures();

private:

	// stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nVertices;	// Number of vertices for the mesh
		GLuint nIndices;    // Number of indices for the mesh
	};

	// the available 3D shapes
	GLMesh m_BoxMesh;

	bool m_bMemoryLayoutDone;

public:
	// methods for loading the shape mesh data 
	// into memory
	void LoadBoxMesh();

	// methods for drawing the shape mesh in the
	// display window
	void DrawBoxMesh();


private:

	// called to calculate the normal for 
	// the passed in coordinates
	glm::vec3 CalculateTriangleNormal(
		glm::vec3 px, glm::vec3 py, glm::vec3 pz);

	// called to set the memory layout 
	// template for shader data
	void SetShaderMemoryLayout();
};