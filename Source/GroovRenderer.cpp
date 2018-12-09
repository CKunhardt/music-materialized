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
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

//==============================================================================
GroovRenderer::GroovRenderer()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
	if (auto* peer = getPeer())
		peer->setCurrentRenderingEngine(0);

	setOpaque(true);
	controlsOverlay.reset(new GroovPlayer(*this));
	// addAndMakeVisible(controlsOverlay.get());

	initOrbitals();

	openGLContext.setRenderer(this);
	openGLContext.attachTo(*this);
	openGLContext.setContinuousRepainting(true);

	controlsOverlay->initialize();

	_lastTime = std::chrono::high_resolution_clock::now();
	_curTime = std::chrono::high_resolution_clock::now();
	
	if (Desktop::getInstance().getDisplays().displays.size() > 1) {
		auto screen = Desktop::getInstance().getDisplays().displays[1].totalArea;
		setSize(screen.getWidth(), screen.getHeight());
	}
	else {
		auto screen = Desktop::getInstance().getDisplays().getMainDisplay().totalArea;
		setSize(screen.getWidth(), screen.getHeight());
	}
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
	papaShape.reset();
	for (auto xShape : xOrbitals)
		xShape.reset();
	for (auto yShape : yOrbitals)
		yShape.reset();
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

	updateShader();   // Check whether we need to compile a new shader

	if (shader.get() == nullptr)
		return;

	// Having used the juce 2D renderer, it will have messed-up a whole load of GL state, so
	// we need to initialise some important settings before doing our normal GL 3D drawing..
	// C&A note: not sure if this is still necessary, since we're not in the demo runner.
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

	// Frame time calculations for BPM, etc.
	_lastTime = _curTime;
	_curTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = _curTime - _lastTime;
	double rdt = diff.count();

	// Set up view matrix + eye position
	glm::vec3 eye_world = glm::vec3(0.0, 3.0, 25.0);
	glm::mat4 view = glm::lookAt(eye_world, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	// Set up projection matrix
	Matrix3D<float> projectionMatrix = getProjectionMatrix();

	// Set up light position
	glm::vec3 light_position = glm::vec3(-3.0f, 3.0f, 3.0f);

	// Convert from GLM to Juce data types.
	Matrix3D<float> viewMatrix = g2jMat4(view);

	// Get rotation values from draggableOrientation + rotate it more every frame
	Matrix3D<float> rotationMatrix = draggableOrientation.getRotationMatrix();
	auto rotationFrameMatrix = Matrix3D<float>::rotation({ rotation, rotation, -0.3f });

	// Combine rotations
	Matrix3D<float> jRotMat = rotationFrameMatrix * rotationMatrix;

	glm::mat4 rotMat = j2gMat4(jRotMat);

	glm::mat4 beatRot = glm::toMat4(glm::angleAxis((float)looper, glm::vec3(0.0, 1.0, 0.0)));

	rotMat = beatRot * rotMat;

	// Set up model matrix
	glm::mat4 model = glm::mat4(1.0);

	model = rotMat * model;

	Matrix3D<float> modelMatrix = g2jMat4(model);

	// Set up normal matrix
	glm::mat3 normal_mat = glm::transpose(glm::inverse(model));

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

	papaShape->draw(openGLContext, *attributes);

	// Orbitals

	// Set up an identity model matrix
	glm::mat4 oModelMatrix = glm::mat4(1.0);

	if (looper > 2 * glm::pi<double>()) {
		looper += (glm::pi<double>() * (bpm / 60.0) * rdt) - 2 * glm::pi<double>();
	}
	else if (resetPeriod) {
		looper = (glm::pi<double>() * (bpm / 60.0) * rdt);
		resetPeriod = false;
	}
	else {
		looper += (glm::pi<double>() * (bpm / 60.0) * rdt);
		
	}

	// Sinusoidal interpolation between 0 and 1 based on looper.

	if (looper < (glm::pi<double>() ))/// 2.0))
		curveLooper = glm::pi<double>() * sin(looper / 2.0) / 2.0;
	else
		curveLooper = glm::pi<double>() * sin((looper - glm::pi<double>()) / 2.0) / 2.0;


	// Scale it so it bounces every frame
	if (doScaleBounce) {
		loopingScale = (float)abs(cos(looper));
		oModelMatrix = glm::scale(oModelMatrix, glm::vec3(loopingScale / 2.0)); // Divided by 2 to make the orbital cubes smaller than the middle cube
	}
	else {
		oModelMatrix = glm::scale(oModelMatrix, glm::vec3(0.5));
	}

	oModelMatrix = -rotMat * oModelMatrix;

	glm::mat4 transMat;
	// X-Orbitals
	for (int i = 0; i < GV_NUM_ORBITALS; i++) {
		glm::mat4 baseModel = oModelMatrix;
		float wiggleDistance = (audioStopped) ? 1000.0f : GV_INV_WIGGLE_DISTANCE;
		transMat = glm::translate(glm::mat4(1.0), glm::vec3(GV_ORBITAL_DISTANCE * cos(curveLooper + (i*glm::pi<float>()/2.0)), cos(looper*wiggleSpeed)/ wiggleDistance, GV_ORBITAL_DISTANCE * sin(curveLooper + (i*glm::pi<float>() / 2.0))));
		baseModel = transMat * baseModel;

		modelMatrix = g2jMat4(baseModel);

		// Set up normal matrix
		normal_mat = glm::transpose(glm::inverse(baseModel));

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				int index = (i * 3) + j;
				normalMatrix[index] = normal_mat[i][j];
			}
		}

		if (uniforms->modelMatrix.get() != nullptr)
			uniforms->modelMatrix->setMatrix4(modelMatrix.mat, 1, false);

		if (uniforms->normalMatrix.get() != nullptr)
			uniforms->normalMatrix->setMatrix3(normalMatrix, 1, false);

		xOrbitals[i]->draw(openGLContext, *attributes);
	}

	// Y-Orbitals
	for (int i = 0; i < GV_NUM_ORBITALS; i++) {
		glm::mat4 baseModel = oModelMatrix;
		float wiggleDistance = (audioStopped) ? 1000.0f : GV_INV_WIGGLE_DISTANCE;
		transMat = glm::translate(glm::mat4(1.0), glm::vec3(cos(looper*wiggleSpeed) / wiggleDistance, GV_ORBITAL_DISTANCE * cos(curveLooper + (i*glm::pi<float>() / 2.0) + (glm::pi<float>() / 4.0)), GV_ORBITAL_DISTANCE * sin(curveLooper + (i*glm::pi<float>() / 2.0) + (glm::pi<float>() / 4.0))));
		baseModel = transMat * baseModel;

		modelMatrix = g2jMat4(baseModel);

		// Set up normal matrix
		normal_mat = glm::transpose(glm::inverse(baseModel));

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				int index = (i * 3) + j;
				normalMatrix[index] = normal_mat[i][j];
			}
		}

		if (uniforms->modelMatrix.get() != nullptr)
			uniforms->modelMatrix->setMatrix4(modelMatrix.mat, 1, false);

		if (uniforms->normalMatrix.get() != nullptr)
			uniforms->normalMatrix->setMatrix3(normalMatrix, 1, false);

		xOrbitals[i]->draw(openGLContext, *attributes);
	}

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

	return Matrix3D<float>::fromFrustum(-w, w, -h, h, 3.0f, 30.0f);
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

void GroovRenderer::updateShader()
{
	if (newVertexShader.isNotEmpty() || newFragmentShader.isNotEmpty())
	{
		std::unique_ptr<OpenGLShaderProgram> newShader(new OpenGLShaderProgram(openGLContext));

		if (newShader->addVertexShader(OpenGLHelpers::translateVertexShaderToV3(newVertexShader))
			&& newShader->addFragmentShader(OpenGLHelpers::translateFragmentShaderToV3(newFragmentShader))
			&& newShader->link())
		{
			papaShape.reset();
			for (auto xShape : xOrbitals) xShape.reset();
			for (auto yShape : yOrbitals) yShape.reset();
			attributes.reset();
			uniforms.reset();

			shader.reset(newShader.release());
			shader->use();

			papaShape.reset(new Mesh::Shape(openGLContext, "cube.obj"));
			for (auto &xShape : xOrbitals) xShape.reset(new Mesh::Shape(openGLContext, "cube.obj"));
			for (auto &yShape : yOrbitals) yShape.reset(new Mesh::Shape(openGLContext, "cube.obj"));
			attributes.reset(new Mesh::Attributes(openGLContext, *shader));
			uniforms.reset(new Mesh::Uniforms(openGLContext, *shader));
		}

		triggerAsyncUpdate();

		newVertexShader = {};
		newFragmentShader = {};
	}
}

void GroovRenderer::initOrbitals() {
	for (int i = 0; i < GV_NUM_ORBITALS; i++) {
		std::shared_ptr<Mesh::Shape> xShape;
		std::shared_ptr<Mesh::Shape> yShape;

		xOrbitals.push_back(xShape);
		yOrbitals.push_back(yShape);
	}
}

void GroovRenderer::startPlaying() {
	resetPeriod = true;
	audioStopped = false;
}

void GroovRenderer::stopPlaying() {
	audioStopped = true;
}


