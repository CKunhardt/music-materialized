/*
  ==============================================================================

    GroovRenderer.h
    Created: 2 Nov 2018 2:56:15pm
    Author:  ClintonK

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
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
	Matrix3D<float> getViewMatrix() const;

	void setTexture(Mesh::Texture* t);
	void setShaderProgram(const String& vertexShader, const String& fragmentShader);

	void paint(Graphics&) override;

    void resized() override;

	Draggable3DOrientation draggableOrientation;
	bool doBackgroundDrawing = false;
	float scale = 0.5f, rotationSpeed = 0.0f;
	BouncingNumber bouncingNumber;

private:
	void handleAsyncUpdate() override;

	void drawBackground2DStuff(float desktopScale);

	OpenGLContext openGLContext;

	std::unique_ptr<GroovPlayer> controlsOverlay;

	float rotation = 0.0f;

	std::unique_ptr<OpenGLShaderProgram> shader;
	std::unique_ptr<Mesh::Shape> shape;
	std::unique_ptr<Mesh::Attributes> attributes;
	std::unique_ptr<Mesh::Uniforms> uniforms;

	OpenGLTexture texture;
	Mesh::Texture* textureToUse = nullptr;
	Mesh::Texture* lastTexture = nullptr;

	String newVertexShader, newFragmentShader, statusText;

	struct BackgroundStar
	{
		SlowerBouncingNumber x, y, hue, angle;
	};

	BackgroundStar stars[3];

	//==============================================================================
	void updateShader();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroovRenderer)
};
