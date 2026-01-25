///////////////////////////////////////////////////////////////////////////////
// BoxMultipleTextures.cpp
// ========
// create box mesh that uses texture atlas for puzzle box
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	EDITED BY Arka Tu
//	Created for CS-330-Computational Graphics and Visualization, Nov. 7th, 2022
///////////////////////////////////////////////////////////////////////////////

#include "BoxPuzzleTextures.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

namespace
{
	const double M_PI = 3.14159265358979323846f;
	const double M_PI_2 = 1.571428571428571;
	const GLuint g_FloatsPerVertex = 3;	// Number of coordinates per vertex
	const GLuint g_FloatsPerNormal = 3;	// Number of values per vertex color
	const GLuint g_FloatsPerUV = 2;		// Number of texture coordinate values
}

BoxPuzzleTextures::BoxPuzzleTextures()
{
	m_bMemoryLayoutDone = false;
}

///////////////////////////////////////////////////
//	LoadCylinderMesh()
//
//	Create a box mesh by specifying the vertices and 
//  store it in a VAO/VBO.  The normals and texture
//  coordinates are also set.
//
//	Correct triangle drawing command:
//
//	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
///////////////////////////////////////////////////
void BoxPuzzleTextures::LoadBoxMesh()
{

	// Position and Color data
	GLfloat verts[] = {
		//Positions				//Normals
		// ------------------------------------------------------

		//Back Face				//Negative Z Normal  Texture Coords. -- edited to use texture atlas
		0.5f, 0.5f, -0.5f,		0.0f,  0.0f, -1.0f,  0.5f, 1.0f,   //0
		0.5f, -0.5f, -0.5f,		0.0f,  0.0f, -1.0f,  0.5f, 0.8145f,   //1
		-0.5f, -0.5f, -0.5f,	0.0f,  0.0f, -1.0f,  1.0f, 0.8145f,   //2
		-0.5f, 0.5f, -0.5f,		0.0f,  0.0f, -1.0f,  1.0f, 1.0f,   //3

		//Bottom Face			//Negative Y Normal
		-0.5f, -0.5f, 0.5f,		0.0f, -1.0f,  0.0f,  0.5f, 1.0f,  //4
		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f,  0.0f,  0.5f, 0.0f,  //5
		0.5f, -0.5f, -0.5f,		0.0f, -1.0f,  0.0f,  1.0f, 0.0f,  //6
		0.5f, -0.5f,  0.5f,		0.0f, -1.0f,  0.0f,  1.0f, 1.0f,  //7

		//Left Face				//Negative X Normal
		-0.5f, 0.5f, -0.5f,		-1.0f,  0.0f,  0.0f,  0.5f, 1.0f,  //8
		-0.5f, -0.5f,  -0.5f,	-1.0f,  0.0f,  0.0f,  0.5f, 0.8145f,  //9
		-0.5f,  -0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,  1.0f, 0.8145f,  //10
		-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  //11

		//Right Face			//Positive X Normal
		0.5f,  0.5f,  0.5f,		1.0f,  0.0f,  0.0f,  0.5f, 1.0f,  //12
		0.5f,  -0.5f, 0.5f,		1.0f,  0.0f,  0.0f,  0.5f, 0.8145f,  //13
		0.5f, -0.5f, -0.5f,		1.0f,  0.0f,  0.0f,  1.0f, 0.8145f,  //14
		0.5f, 0.5f, -0.5f,		1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  //15

		//Top Face				//Positive Y Normal
		-0.5f,  0.5f, -0.5f,	0.0f,  1.0f,  0.0f,  0.0f, 1.0f, //16
		-0.5f,  0.5f, 0.5f,		0.0f,  1.0f,  0.0f,  0.0f, 0.0f, //17
		0.5f,  0.5f,  0.5f,		0.0f,  1.0f,  0.0f,  0.5f, 0.0f, //18
		0.5f,  0.5f,  -0.5f,	0.0f,  1.0f,  0.0f,  0.5f, 1.0f, //19

		//Front Face			//Positive Z Normal
		-0.5f, 0.5f,  0.5f,	    0.0f,  0.0f,  1.0f,  0.5f, 1.0f, //20
		-0.5f, -0.5f,  0.5f,	0.0f,  0.0f,  1.0f,  0.5f, 0.8145f, //21
		0.5f,  -0.5f,  0.5f,	0.0f,  0.0f,  1.0f,  1.0f, 0.8145f, //22
		0.5f,  0.5f,  0.5f,		0.0f,  0.0f,  1.0f,  1.0f, 1.0f, //23
	};

	// Index data
	GLuint indices[] = {
		0,1,2,
		0,3,2,
		4,5,6,
		4,7,6,
		8,9,10,
		8,11,10,
		12,13,14,
		12,15,14,
		16,17,18,
		16,19,18,
		20,21,22,
		20,23,22
	};

	m_BoxMesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (g_FloatsPerVertex + g_FloatsPerNormal + g_FloatsPerUV));
	m_BoxMesh.nIndices = sizeof(indices) / sizeof(indices[0]);

	glGenVertexArrays(1, &m_BoxMesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(m_BoxMesh.vao);

	// Create 2 buffers: first one for the vertex data; second one for the indices
	glGenBuffers(2, m_BoxMesh.vbos);
	glBindBuffer(GL_ARRAY_BUFFER, m_BoxMesh.vbos[0]); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BoxMesh.vbos[1]); // Activates the buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	if (m_bMemoryLayoutDone == false)
	{
		SetShaderMemoryLayout();
	}
}

///////////////////////////////////////////////////
//	DrawBoxMesh()
// 
//	Transform and draw the mesh to the window.
// 
///////////////////////////////////////////////////
void BoxPuzzleTextures::DrawBoxMesh()
{
	glBindVertexArray(m_BoxMesh.vao);

	glDrawElements(GL_TRIANGLES, m_BoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	glBindVertexArray(0);
}

glm::vec3 BoxPuzzleTextures::CalculateTriangleNormal(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
{
	glm::vec3 Normal(0, 0, 0);
	float v1x = p1.x - p0.x;
	float v1y = p1.y - p0.y;
	float v1z = p1.z - p0.z;
	float v2x = p2.x - p1.x;
	float v2y = p2.y - p1.y;
	float v2z = p2.z - p1.z;
	Normal.x = v1y * v2z - v1z * v2y;
	Normal.y = v1z * v2x - v1x * v2z;
	Normal.y = v1x * v2y - v1y * v2x;
	float len = (float)sqrt(Normal.x * Normal.x + Normal.y * Normal.y + Normal.z * Normal.z);
	if (len == 0)
	{
		//throw Exception();
	}
	else
	{
		Normal.x /= len;
		Normal.y /= len;
		Normal.z /= len;
	}
	return Normal;
}



void BoxPuzzleTextures::SetShaderMemoryLayout()
{
	// The following code defines the layout of the mesh data in memory - each mesh needs
	// to have the same memory layout so that the data is retrieved properly by the shaders

	// Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (g_FloatsPerVertex + g_FloatsPerNormal + g_FloatsPerUV);// The number of floats before each

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, g_FloatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, g_FloatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * g_FloatsPerVertex));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, g_FloatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (g_FloatsPerVertex + g_FloatsPerNormal)));
	glEnableVertexAttribArray(2);
}