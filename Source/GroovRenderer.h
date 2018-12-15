/*
  ==============================================================================

    GroovRenderer.h
    Created: 2 Nov 2018 2:56:15pm
    Author:  ClintonK

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <glm.hpp>
#include <chrono>
#include "GLMHelpers.h"
#include "Mesh.h"

//==============================================================================
/*
*/

class GroovPlayer;

class GroovRenderer    : public Component, 
						 private OpenGLRenderer,
						 private AsyncUpdater
{
public:
    GroovRenderer();
    ~GroovRenderer();

	void newOpenGLContextCreated() override;
	void openGLContextClosing() override;
	void freeAllContextObjects();

	// This is a virtual method in OpenGLRenderer, and is called when it's time
	// to do your GL rendering.
	void renderOpenGL() override;

	Matrix3D<float> getProjectionMatrix() const;

	void setTexture(Mesh::Texture* t);
	void setShaderProgram(const String& vertexShader, const String& fragmentShader, bool isSkyShader);

	void paint(Graphics&) override;

    void resized() override;

	std::unique_ptr<GroovPlayer> controlsOverlay;

	bool doScaleBounce = false;
	float scale = 0.5f, rotationSpeed = 0.0f, wiggleSpeed = 4.0f, colorSat = 0.5f, colorVal = 1.0f, bgHue = 0.0f;

	// If we change this, we have to change the initial value of initialBPM in private.
	int bpm = 120;
	void startPlaying();
	void stopPlaying();

private:
	void handleAsyncUpdate() override;
	
	void initOrbitals();

	OpenGLContext openGLContext;

	float rotation = 0.0f;
	float loopingScale = 1.0f;
	double looper = 0.0;
	double beatTime = 0.0;
	double curveLooper = 0.0;
	double bounceDistance = 1.0;
	bool resetPeriod = false;
	bool audioStopped = true;

	// Beat delay variables to account for frame sync
	bool shouldDelay = false;
	double beatDelay = 0.0;
	double delayAccumulator = 0.0;

	// If we change this, we have to change the initial value of bpm in public.
	int initialBPM = 120;

	std::unique_ptr<OpenGLShaderProgram> shader;
	std::unique_ptr<OpenGLShaderProgram> skyShader;
	std::unique_ptr<Mesh::Shape> skyCube;
	std::unique_ptr<Mesh::Shape> papaShape;
	std::vector<std::shared_ptr<Mesh::Shape>> xOrbitals;
	std::vector<std::shared_ptr<Mesh::Shape>> yOrbitals;

	std::unique_ptr<Mesh::Attributes> attributes;
	std::unique_ptr<Mesh::Uniforms> uniforms;

	std::unique_ptr<Mesh::Uniforms> skyUniforms;
	std::unique_ptr<Mesh::Attributes> skyAttributes;

	std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> _curTime;

	OpenGLTexture texture;
	Mesh::Texture* textureToUse = nullptr;
	Mesh::Texture* lastTexture = nullptr;

	String newVertexShader, newFragmentShader, newVertexSkyShader, newFragmentSkyShader, statusText;

	OwnedArray<Mesh::Texture> textures;

	void updateShader();
	void updateSkyShader();

	void drawXOrbitals(glm::mat4 model);
	void drawYOrbitals(glm::mat4 model);

	glm::vec3 angleToRGB(double h, float sat, float val);

	const int GV_NUM_ORBITALS = 4;
	const float GV_ORBITAL_DISTANCE = 1.35;
	const float GV_INV_WIGGLE_DISTANCE = 10.0f;


	// Author: Stefan Gustavson (stegu@itn.liu.se) 2004

	GLuint permTextureID, simplexTextureID, gradTextureID;

	char *permPixels, *gradPixels;

	void initPermTexture(GLuint *texID);
	void initSimplexTexture(GLuint *texID);
	void initGradTexture(GLuint *texID);

	int perm[256] = {151,160,137,91,90,15,
		  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 
	};

	int grad3[16][3] = { {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1},
				   {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
				   {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0}, // 12 cube edges
				   {1,0,-1},{-1,0,-1},{0,-1,1},{0,1,1} };

	int grad4[32][4] = { {0,1,1,1}, {0,1,1,-1}, {0,1,-1,1}, {0,1,-1,-1}, // 32 tesseract edges
				   {0,-1,1,1}, {0,-1,1,-1}, {0,-1,-1,1}, {0,-1,-1,-1},
				   {1,0,1,1}, {1,0,1,-1}, {1,0,-1,1}, {1,0,-1,-1},
				   {-1,0,1,1}, {-1,0,1,-1}, {-1,0,-1,1}, {-1,0,-1,-1},
				   {1,1,0,1}, {1,1,0,-1}, {1,-1,0,1}, {1,-1,0,-1},
				   {-1,1,0,1}, {-1,1,0,-1}, {-1,-1,0,1}, {-1,-1,0,-1},
				   {1,1,1,0}, {1,1,-1,0}, {1,-1,1,0}, {1,-1,-1,0},
				   {-1,1,1,0}, {-1,1,-1,0}, {-1,-1,1,0}, {-1,-1,-1,0} };

	unsigned char simplex4[64][4] = { {0,64,128,192},{0,64,192,128},{0,0,0,0},
		  {0,128,192,64},{0,0,0,0},{0,0,0,0},{0,0,0,0},{64,128,192,0},
		  {0,128,64,192},{0,0,0,0},{0,192,64,128},{0,192,128,64},
		  {0,0,0,0},{0,0,0,0},{0,0,0,0},{64,192,128,0},
		  {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
		  {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
		  {64,128,0,192},{0,0,0,0},{64,192,0,128},{0,0,0,0},
		  {0,0,0,0},{0,0,0,0},{128,192,0,64},{128,192,64,0},
		  {64,0,128,192},{64,0,192,128},{0,0,0,0},{0,0,0,0},
		  {0,0,0,0},{128,0,192,64},{0,0,0,0},{128,64,192,0},
		  {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
		  {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
		  {128,0,64,192},{0,0,0,0},{0,0,0,0},{0,0,0,0},
		  {192,0,64,128},{192,0,128,64},{0,0,0,0},{192,64,128,0},
		  {128,64,0,192},{0,0,0,0},{0,0,0,0},{0,0,0,0},
		  {192,64,0,128},{0,0,0,0},{192,128,0,64},{192,128,64,0} };

	// end Stefan's code

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroovRenderer)
};
