/*
  ==============================================================================

    GroovRenderer.cpp
    Created: 2 Nov 2018 2:56:15pm
    Author:  ClintonK

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "GroovRenderer.h"
#include "GroovPlayer.h"

#include <gtc/matrix_transform.hpp>

//==============================================================================
GroovRenderer::GroovRenderer()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
	if (auto* peer = getPeer())
		peer->setCurrentRenderingEngine(0);

	setOpaque(true);
	controlsOverlay.reset(new GroovPlayer(*this));
	addAndMakeVisible(controlsOverlay.get());

	openGLContext.setRenderer(this);
	openGLContext.attachTo(*this);
	openGLContext.setContinuousRepainting(true);

	controlsOverlay->initialize();

	setSize(500, 500);
}

GroovRenderer::~GroovRenderer()
{
	openGLContext.detach();
}

void GroovRenderer::newOpenGLContextCreated()
{
	// nothing to do in this case - we'll initialise our shaders + textures
	// on demand, during the render callback.
	freeAllContextObjects();

	if (controlsOverlay.get() != nullptr)
		controlsOverlay->updateShader();
}

void GroovRenderer::openGLContextClosing()
{
	// When the context is about to close, you must use this callback to delete
	// any GPU resources while the context is still current.
	freeAllContextObjects();

	if (lastTexture != nullptr)
		setTexture(lastTexture);
}

void GroovRenderer::freeAllContextObjects()
{
	shape.reset();
	shader.reset();
	attributes.reset();
	uniforms.reset();
	texture.release();
}

void GroovRenderer::renderOpenGL()
{
	jassert(OpenGLHelpers::isContextActive());

	auto desktopScale = (float)openGLContext.getRenderingScale();

	OpenGLHelpers::clear(getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
		Colours::lightblue));

	if (textureToUse != nullptr)
		if (!textureToUse->applyTo(texture))
			textureToUse = nullptr;

	// First draw our background graphics to demonstrate the OpenGLGraphicsContext class
	if (doBackgroundDrawing)
		drawBackground2DStuff(desktopScale);

	updateShader();   // Check whether we need to compile a new shader

	if (shader.get() == nullptr)
		return;

	// Having used the juce 2D renderer, it will have messed-up a whole load of GL state, so
	// we need to initialise some important settings before doing our normal GL 3D drawing..
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	openGLContext.extensions.glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	glViewport(0, 0, roundToInt(desktopScale * getWidth()), roundToInt(desktopScale * getHeight()));

	texture.bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set up view matrix + eye position
	glm::vec3 eye_world = glm::vec3(0.0, 5.0, 10.0);
	glm::mat4 view = glm::lookAt(eye_world, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	// Set up projection matrix
	// glm::mat4 projection = glm::perspective(glm::radians(45.0f), getLocalBounds().toFloat().getAspectRatio(true), 0.01f, 100.0f);

	// Set up model matrix from draggableOrientation + rotate it
	Matrix3D<float> modelMatrix = draggableOrientation.getRotationMatrix();
	//Matrix3D<float> modelMatrix = Matrix3D<float>();
	auto rotationMatrix = Matrix3D<float>::rotation({ rotation, rotation, -0.3f });
	modelMatrix = rotationMatrix * modelMatrix;
	glm::mat4 model = j2gMat4(modelMatrix);

	// Set up normal matrix
	glm::mat3 normal_mat = glm::transpose(glm::inverse(model));

	// Set up light position
	glm::vec3 light_position = glm::vec3(-15.0f, 10.0f, 15.0f);

	// Convert from GLM to Juce data types. We've already converted "model" by this point
	Matrix3D<float> viewMatrix = g2jMat4(view);
	Matrix3D<float> projectionMatrix = getProjectionMatrix();

	// No native type for normal matrices, so just put into a float[9]. Can't really put this anywhere else or access violations :(
	float normalMatrix[9];
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			int index = (i * 3) + j;
			normalMatrix[index] = normal_mat[i][j];
		}
	}

	shader->use();

	if (uniforms->modelMatrix.get() != nullptr)
		uniforms->modelMatrix->setMatrix4(modelMatrix.mat, 1, false);

	if (uniforms->projectionMatrix.get() != nullptr)
		uniforms->projectionMatrix->setMatrix4(projectionMatrix.mat, 1, false);

	if (uniforms->viewMatrix.get() != nullptr)
		uniforms->viewMatrix->setMatrix4(viewMatrix.mat, 1, false);

	if (uniforms->normalMatrix.get() != nullptr)
		uniforms->normalMatrix->setMatrix3(normalMatrix, 1, false);

	if (uniforms->texture.get() != nullptr)
		uniforms->texture->set((GLint)0);

	if (uniforms->eyePosition.get() != nullptr)
		uniforms->eyePosition->set(eye_world.x, eye_world.y, eye_world.z);

	if (uniforms->lightPosition.get() != nullptr)
		uniforms->lightPosition->set(light_position.x, light_position.y, light_position.z);

	if (uniforms->bouncingNumber.get() != nullptr)
		uniforms->bouncingNumber->set(bouncingNumber.getValue());

	shape->draw(openGLContext, *attributes);

	// Reset the element buffers so child Components draw correctly
	openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
	openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (!controlsOverlay->isMouseButtonDown())
		rotation += (float)rotationSpeed;
}

Matrix3D<float> GroovRenderer::getProjectionMatrix() const
{
	auto w = 1.0f / (scale + 0.1f);
	auto h = w * getLocalBounds().toFloat().getAspectRatio(false);

	return Matrix3D<float>::fromFrustum(-w, w, -h, h, 4.0f, 30.0f);
}

Matrix3D<float> GroovRenderer::getViewMatrix() const
{
	auto viewMatrix = draggableOrientation.getRotationMatrix()
		* Vector3D<float>(0.0f, 1.0f, -10.0f);

	auto rotationMatrix = Matrix3D<float>::rotation({ rotation, rotation, -0.3f });

	return rotationMatrix * viewMatrix;
}

void GroovRenderer::setTexture(Mesh::Texture* t)
{
	lastTexture = textureToUse = t;
}

void GroovRenderer::setShaderProgram(const String& vertexShader, const String& fragmentShader)
{
	newVertexShader = vertexShader;
	newFragmentShader = fragmentShader;
}

void GroovRenderer::paint(Graphics&) {}

void GroovRenderer::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

	controlsOverlay->setBounds(getLocalBounds());
	draggableOrientation.setViewport(getLocalBounds());
}

void GroovRenderer::handleAsyncUpdate()
{
	controlsOverlay->statusLabel.setText(statusText, dontSendNotification);
}

void GroovRenderer::drawBackground2DStuff(float desktopScale)
{
	// Create an OpenGLGraphicsContext that will draw into this GL window..
	std::unique_ptr<LowLevelGraphicsContext> glRenderer(createOpenGLGraphicsContext(openGLContext,
		roundToInt(desktopScale * getWidth()),
		roundToInt(desktopScale * getHeight())));

	if (glRenderer.get() != nullptr)
	{
		Graphics g(*glRenderer);
		g.addTransform(AffineTransform::scale(desktopScale));

		for (auto s : stars)
		{
			auto size = 0.25f;

			// This stuff just creates a spinning star shape and fills it..
			Path p;
			p.addStar({ getWidth()  * s.x.getValue(),
						 getHeight() * s.y.getValue() },
				7,
				getHeight() * size * 0.5f,
				getHeight() * size,
				s.angle.getValue());

			auto hue = s.hue.getValue();

			g.setGradientFill(ColourGradient(Colours::green.withRotatedHue(hue).withAlpha(0.8f),
				0, 0,
				Colours::red.withRotatedHue(hue).withAlpha(0.5f),
				0, (float)getHeight(), false));
			g.fillPath(p);
		}
	}
}

void GroovRenderer::updateShader()
{
	if (newVertexShader.isNotEmpty() || newFragmentShader.isNotEmpty())
	{
		std::unique_ptr<OpenGLShaderProgram> newShader(new OpenGLShaderProgram(openGLContext));

		if (newShader->addVertexShader(OpenGLHelpers::translateVertexShaderToV3(newVertexShader))
			&& newShader->addFragmentShader(OpenGLHelpers::translateFragmentShaderToV3(newFragmentShader))
			&& newShader->link())
		{
			shape.reset();
			attributes.reset();
			uniforms.reset();

			shader.reset(newShader.release());
			shader->use();

			shape.reset(new Mesh::Shape(openGLContext));
			attributes.reset(new Mesh::Attributes(openGLContext, *shader));
			uniforms.reset(new Mesh::Uniforms(openGLContext, *shader));

			statusText = "GLSL: v" + String(OpenGLShaderProgram::getLanguageVersion(), 2);
		}
		else
		{
			statusText = newShader->getLastError();
		}

		triggerAsyncUpdate();

		newVertexShader = {};
		newFragmentShader = {};
	}
}
