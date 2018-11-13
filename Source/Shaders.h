/*
  ==============================================================================

    Shaders.h
    Created: 2 Nov 2018 6:31:15pm
    Author:  ClintonK

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
struct ShaderPreset
{
	const char* name;
	const char* vertexShader;
	const char* fragmentShader;
};

struct Shader
{
	const char* vertexShader;
	const char* fragmentShader;
};

static Shader getShader()
{
	Shader shader =
	{
		"#version 420\n"
		"\n"
		"attribute vec4 position;\n"
		"attribute vec4 normal;\n"
		"attribute vec4 sourceColour;\n"
		"attribute vec2 textureCoordIn;\n"
		"\n"
		"uniform mat4 modelMatrix, viewMatrix, projectionMatrix;\n"
		"uniform mat3 normalMatrix;\n"
		"\n"
		"varying vec4 destinationColour;\n"
		"varying vec3 worldPos, worldNormal;\n"
		"varying vec2 textureCoordOut;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    "
		"    destinationColour = sourceColour;\n"
		"    textureCoordOut = textureCoordIn;\n"
		"    worldPos = vec3(modelMatrix * position);\n"
		"    worldNormal = normalize(normalMatrix * normal.xyz);\n"
		"\n"
		"    gl_Position = projectionMatrix * viewMatrix * vec4(worldPos, 1.0);\n"
		"}\n",

		"#version 420\n"
		"\n"
	   #if JUCE_OPENGL_ES
		"varying lowp vec3 worldPos, worldNormal;\n"
		"varying lowp vec4 destinationColour;\n"
		"varying lowp vec2 textureCoordOut;\n"
	   #else
		"varying vec3 worldPos, worldNormal;\n"
		"varying vec4 destinationColour;\n"
		"varying vec2 textureCoordOut;\n"
	   #endif
		"\n"
		"uniform vec3 lightPosition;\n"
		"uniform vec3 eyePosition;\n"
		"uniform sampler2D textureSampler;\n"
		"\n"
		"void main()\n"
		"{\n"
	   #if JUCE_OPENGL_ES
		"   highp vec4 color = texture2D (textureSampler, textureCoordOut);\n"
		"\n"
		"   highp vec3 ambient = 0.2 * color.rgb;\n"
		"\n"
		"   highp vec3 lightDir = normalize(lightPosition - worldPos);\n"
		"   highp vec3 normal = normalize(worldNormal);\n"
		"   highp float diff = max(dot(lightDir, normal), 0.0);\n"
		"   highp vec3 diffuse = diff * color.rgb;\n"
		"\n"
		"   highp vec3 viewDir = normalize(eyePosition - worldPos);\n"
		"   highp vec3 reflectDir = reflect(-lightDir, normal);\n"
		"   highp float spec = 0.0;\n"
		"\n"
		"   highp vec3 halfwayDir = normalize(lightDir + viewDir);\n"
		"   spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);\n"
		"\n"
		"   highp vec3 specular = vec3(0.8) * spec;\n"
	   #else
		"   vec4 color = texture2D (textureSampler, textureCoordOut);\n"
		"\n"
		"   vec3 ambient = 0.2 * color.rgb;\n"
		"\n"
		"   vec3 lightDir = normalize(lightPosition - worldPos);\n"
		"   vec3 normal = normalize(worldNormal);\n"
		"   float diff = max(dot(lightDir, normal), 0.0);\n"
		"   vec3 diffuse = diff * color.rgb;\n"
		"\n"
		"   vec3 viewDir = normalize(eyePosition - worldPos);\n"
		"   vec3 reflectDir = reflect(-lightDir, normal);\n"
		"   float spec = 0.0;\n"
		"\n"
		"   vec3 halfwayDir = normalize(lightDir + viewDir);\n"
		"   spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);\n"
		"\n"
		"   vec3 specular = vec3(0.8) * spec;\n"
	   #endif
		"    gl_FragColor = vec4(ambient + diffuse + specular, color.a);\n"
		"}\n"
	};

	return shader;
}
