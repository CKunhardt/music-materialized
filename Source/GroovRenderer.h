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
	void setShaderProgram(const String& vertexShader, const String& fragmentShader);

	void paint(Graphics&) override;

    void resized() override;

	Draggable3DOrientation draggableOrientation;
	bool doScaleBounce = false;
	float scale = 0.5f, rotationSpeed = 0.0f;

	// If we change this, we have to change the initial value of initialBPM in private.
	int bpm = 120;
	BouncingNumber bouncingNumber;

private:
	void handleAsyncUpdate() override;

	OpenGLContext openGLContext;

	std::unique_ptr<GroovPlayer> controlsOverlay;

	float rotation = 0.0f;
	float loopingScale = 1.0f;
	double scaleLooper = 0.0;
	double rotLooper = 0.0;
	double bounceDistance = 1.0;

	// Beat delay variables to account for frame sync
	bool shouldDelay = false;
	double beatDelay = 0.0;
	double delayAccumulator = 0.0;

	// If we change this, we have to change the initial value of bpm in public.
	int initialBPM = 120;

	std::unique_ptr<OpenGLShaderProgram> shader;
	std::unique_ptr<Mesh::Shape> shape;
	std::unique_ptr<Mesh::Attributes> attributes;
	std::unique_ptr<Mesh::Uniforms> uniforms;

	std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> _curTime;

	std::chrono::time_point<std::chrono::high_resolution_clock> _lastBeatTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> _curBeatTime;


	OpenGLTexture texture;
	Mesh::Texture* textureToUse = nullptr;
	Mesh::Texture* lastTexture = nullptr;

	String newVertexShader, newFragmentShader, statusText;

	struct BackgroundStar
	{
		SlowerBouncingNumber x, y, hue, angle;
	};

	//==============================================================================
	void updateShader();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroovRenderer)
};
