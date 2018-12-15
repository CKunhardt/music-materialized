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

	initPermTexture(&permTextureID);
	initSimplexTexture(&simplexTextureID);
	initGradTexture(&gradTextureID);

	//textures.add(new Mesh::TextureFromAsset("background.png"));
	//setTexture(textures[0]);

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
	delete(permPixels);
	delete(gradPixels);
	openGLContext.detach();
}

void GroovRenderer::newOpenGLContextCreated()
{
	// nothing to do in this case - we'll initialise our shaders + textures
	// on demand, during the render callback.
	freeAllContextObjects();

	if (controlsOverlay.get() != nullptr)
		controlsOverlay->loadShaders();
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

	// Check whether we need to compile a new shader
	updateShader();
	updateSkyShader(); 

	if ((shader.get() == nullptr) || (skyShader.get() == nullptr))
		return;

	// Enable depth tests
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	openGLContext.extensions.glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	glViewport(0, 0, roundToInt(desktopScale * getWidth()), roundToInt(desktopScale * getHeight()));

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, permPixels);
	glBindTexture(GL_TEXTURE_2D, permTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	openGLContext.extensions.glActiveTexture(GL_TEXTURE1);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, simplex4);
	glBindTexture(GL_TEXTURE_1D, simplexTextureID);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	openGLContext.extensions.glActiveTexture(GL_TEXTURE2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, gradPixels);
	glBindTexture(GL_TEXTURE_2D, gradTextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	openGLContext.extensions.glActiveTexture(GL_TEXTURE0);

	// texture.bind();

	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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

	// Add our spastic rotation :)
	auto rotationMatrix = Matrix3D<float>::rotation({ rotation, rotation, -0.3f });

	glm::mat4 rotMat = j2gMat4(rotationMatrix);

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

	glm::vec3 userColor = angleToRGB(glm::degrees(looper), colorSat, colorVal);
	glm::vec3 bgColor = angleToRGB(bgHue, 0.75f, 1.0f); // TODO: add parameterized sat and val if desired

	// Hack to get the background cube to not scale with our scale parameter.
	float tempScale = scale;
	scale = 1.75f;
	Matrix3D<float> bgProjMatrix = getProjectionMatrix();
	scale = tempScale;

	// TODO: environment mapping onto the cubes?
	skyShader->use();

	if (skyUniforms->modelMatrix.get() != nullptr)
		skyUniforms->modelMatrix->setMatrix4(Matrix3D<float>().mat, 1, false);

	if (skyUniforms->projectionMatrix.get() != nullptr)
		skyUniforms->projectionMatrix->setMatrix4(bgProjMatrix.mat, 1, false);

	if (skyUniforms->viewMatrix.get() != nullptr)
		skyUniforms->viewMatrix->setMatrix4(viewMatrix.mat, 1, false);

	if (skyUniforms->permTexture.get() != nullptr)
		skyUniforms->permTexture->set((GLint)0);

	if (skyUniforms->simplexTexture.get() != nullptr)
		skyUniforms->simplexTexture->set((GLint)1);

	if (skyUniforms->gradTexture.get() != nullptr)
		skyUniforms->gradTexture->set((GLint)2);

	if (skyUniforms->amounts.get() != nullptr)
		skyUniforms->amounts->set(bgColor.r, bgColor.g, bgColor.b);

	if (skyUniforms->looper.get() != nullptr)
		skyUniforms->looper->set((float)beatTime);

	glDepthMask(GL_FALSE);
	skyCube->draw(openGLContext, *skyAttributes);
	glDepthMask(GL_TRUE);

	shader->use();

	if (uniforms->modelMatrix.get() != nullptr)
		uniforms->modelMatrix->setMatrix4(modelMatrix.mat, 1, false);

	if (uniforms->projectionMatrix.get() != nullptr)
		uniforms->projectionMatrix->setMatrix4(projectionMatrix.mat, 1, false);

	if (uniforms->viewMatrix.get() != nullptr)
		uniforms->viewMatrix->setMatrix4(viewMatrix.mat, 1, false);

	if (uniforms->normalMatrix.get() != nullptr)
		uniforms->normalMatrix->setMatrix3(normalMatrix, 1, false);

	//if (uniforms->texture.get() != nullptr)
	//	uniforms->texture->set((GLint)0);

	if (uniforms->eyePosition.get() != nullptr)
		uniforms->eyePosition->set(eye_world.x, eye_world.y, eye_world.z);

	if (uniforms->lightPosition.get() != nullptr)
		uniforms->lightPosition->set(light_position.x, light_position.y, light_position.z);

	if (uniforms->userColor != nullptr)
		uniforms->userColor->set(userColor.r, userColor.g, userColor.b);

	papaShape->draw(openGLContext, *attributes);

	// Orbitals

	// Set up an identity model matrix
	glm::mat4 oModelMatrix = glm::mat4(1.0);

	double toAdd = (glm::pi<double>() * (bpm / 60.0) * rdt);

	if (looper > 2 * glm::pi<double>()) {
		looper += (toAdd - 2 * glm::pi<double>());
		beatTime += toAdd / 4.0;
	}
	else if (resetPeriod) {
		looper = toAdd;
		beatTime = toAdd;
		resetPeriod = false;
	}
	else {
		looper += toAdd;
		beatTime += toAdd / 4.0;
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

	// X-Orbitals
	drawXOrbitals(oModelMatrix);

	// Y-Orbitals
	drawYOrbitals(oModelMatrix);

	// Reset the element buffers so child Components draw correctly
	openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
	openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	rotation += (float)rotationSpeed;
}

void GroovRenderer::drawXOrbitals(glm::mat4 model)
{
	glm::mat4 transMat;
	glm::mat3 normal_mat;
	Matrix3D<float> modelMatrix;
	float normalMatrix[9];

	for (int i = 0; i < GV_NUM_ORBITALS; i++) {
		glm::mat4 baseModel = model;
		float wiggleDistance = (audioStopped) ? 1000.0f : GV_INV_WIGGLE_DISTANCE;
		transMat = glm::translate(glm::mat4(1.0), glm::vec3(GV_ORBITAL_DISTANCE * cos(curveLooper + (i*glm::pi<float>() / 2.0)), cos(looper*wiggleSpeed) / wiggleDistance, GV_ORBITAL_DISTANCE * sin(curveLooper + (i*glm::pi<float>() / 2.0))));
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
}

void GroovRenderer::drawYOrbitals(glm::mat4 model)
{
	glm::mat4 transMat;
	glm::mat3 normal_mat;
	Matrix3D<float> modelMatrix;
	float normalMatrix[9];

	for (int i = 0; i < GV_NUM_ORBITALS; i++) {
		glm::mat4 baseModel = model;
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

void GroovRenderer::setShaderProgram(const String& vertexShader, const String& fragmentShader, bool isSkyShader)
{
	if (isSkyShader) {
		newVertexSkyShader = vertexShader;
		newFragmentSkyShader = fragmentShader;
	}
	else {
		newVertexShader = vertexShader;
		newFragmentShader = fragmentShader;
	}
}

void GroovRenderer::paint(Graphics&) {}

void GroovRenderer::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
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

		newVertexShader = {};
		newFragmentShader = {};
	}
}

void GroovRenderer::updateSkyShader()
{
	if (newVertexSkyShader.isNotEmpty() || newFragmentSkyShader.isNotEmpty()) 
	{
		std::unique_ptr<OpenGLShaderProgram> newSkyShader(new OpenGLShaderProgram(openGLContext));

		if (newSkyShader->addVertexShader(OpenGLHelpers::translateVertexShaderToV3(newVertexSkyShader))
			&& newSkyShader->addFragmentShader(OpenGLHelpers::translateFragmentShaderToV3(newFragmentSkyShader))
			&& newSkyShader->link()) 
		{
			skyCube.reset();
			skyAttributes.reset();
			skyUniforms.reset();

			skyShader.reset(newSkyShader.release());
			skyShader->use();

			skyCube.reset(new Mesh::Shape(openGLContext, "skyCube.obj"));
			skyAttributes.reset(new Mesh::Attributes(openGLContext, *skyShader));
			skyUniforms.reset(new Mesh::Uniforms(openGLContext, *skyShader));
		}
	
		newVertexSkyShader = {};
		newFragmentSkyShader = {};
	}
}

// From Stefan Gustavson's code
void GroovRenderer::initPermTexture(GLuint *texID)
{
	int i, j;

	glGenTextures(1, texID);

	permPixels = (char*)malloc(256 * 256 * 4); // we're creating a file manually!
	for (i = 0; i < 256; i++) {
		for (j = 0; j < 256; j++) {
			int offset = (i * 256 + j) * 4;
			char value = perm[(j + perm[i]) & 0xFF];
			permPixels[offset] = grad3[value & 0x0F][0] * 64 + 64;			// Gradient x
			permPixels[offset + 1] = grad3[value & 0x0F][1] * 64 + 64;		// Gradient y
			permPixels[offset + 2] = grad3[value & 0x0F][2] * 64 + 64;		// Gradient z
			permPixels[offset + 3] = value;									// Permuted index
		}
	}
}

// From Stefan Gustavson's code
void GroovRenderer::initSimplexTexture(GLuint *texID)
{
	glGenTextures(1, texID);
}

// From Stefan Gustavson's code
void GroovRenderer::initGradTexture(GLuint *texID)
{
	int i, j;

	glGenTextures(1, texID);
	
	gradPixels = (char*)malloc(256 * 256 * 4);
	for (i = 0; i < 256; i++) {
		for (j = 0; j < 256; j++) {
			int offset = (i * 256 + j) * 4;
			char value = perm[(j + perm[i]) & 0xFF];
			gradPixels[offset] = grad4[value & 0x1F][0] * 64 + 64;     // Gradient x
			gradPixels[offset + 1] = grad4[value & 0x1F][1] * 64 + 64; // Gradient y
			gradPixels[offset + 2] = grad4[value & 0x1F][2] * 64 + 64; // Gradient z
			gradPixels[offset + 3] = grad4[value & 0x1F][3] * 64 + 64; // Gradient z
		}
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

// Adapted from https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
// Takes h in degrees, mind you! Make sure to convert with glm::radians();
// colorVal and colorSat are parameterized controls. Maybe hue's range will be too, at some point? Not sure.
glm::vec3 GroovRenderer::angleToRGB(double h, float sat, float val) {

	// hh is a temporary value used for calculating hue in region. Final color is some combination of p, q, t, and colorVal.
	double hh, p, q, t, ff;

	// I 
	int i;
	glm::vec3 color = glm::vec3();

	if (sat <= 0.0f) {
		color = glm::vec3(val);
		return color;
	}

	hh = h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (int)hh;
	ff = hh - i;
	p = val * (1.0 - sat);
	q = val * (1.0 - (sat * ff));
	t = val * (1.0 - (sat * (1.0 - ff)));

	switch (i) {
	case 0:
		color.r = val;
		color.g = t;
		color.b = p;
		break;
	case 1:
		color.r = q;
		color.g = val;
		color.b = p;
		break;
	case 2:
		color.r = p;
		color.g = val;
		color.b = t;
		break;
	case 3:
		color.r = p;
		color.g = q;
		color.b = val;
		break;
	case 4:
		color.r = t;
		color.g = p;
		color.b = val;
		break;
	case 5:
	default:
		color.r = val;
		color.g = p;
		color.b = q;
		break;
	}
	return color;
}

