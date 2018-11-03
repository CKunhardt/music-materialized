/*
  ==============================================================================

    GLMHelpers.h
    Created: 3 Nov 2018 3:08:05pm
    Author:  ClintonK

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <glm.hpp>


// Converts a Juce 4x4 matrix to glm
static glm::mat4 j2gMat4(Matrix3D<float> mat)
{
	glm::mat4 newMat;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			int index = (i*4)+j;
			newMat[i][j] = mat.mat[index];
		}
	}

	return newMat;
}

// Converts a glm 4x4 matrix to Juce
static Matrix3D<float> g2jMat4(glm::mat4 mat)
{
	Matrix3D<float> newMat;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			int index = (i * 4) + j;
			newMat.mat[index] = mat[i][j];
		}
	}

	return newMat;
}

static Vector3D<float> g2jVec3(glm::vec3 vec)
{
	Vector3D<float> newVec;

	newVec.x = vec.x;
	newVec.y = vec.y;
	newVec.z = vec.z;

	return newVec;
}

static glm::vec3 j2gVec3(Vector3D<float> vec)
{
	glm::vec3 newVec;

	newVec.x = vec.x;
	newVec.y = vec.y;
	newVec.z = vec.z;

	return newVec;
}