/*
  ==============================================================================

    Shaders.h
    Created: 2 Nov 2018 6:31:15pm
    Author:  ClintonK
	Notes: Heavily adapted from JUCE's example OpenGLDemo.h

	Simplex noise fragment shader functionality adapted from Stefan Gustavson:
		* Author: Stefan Gustavson ITN-LiTH (stegu@itn.liu.se) 2004-12-05
		* You may use, modify and redistribute this code free of charge,
		* provided that my name and this notice appears intact.

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
		"attribute vec4 sourceColor;\n"
		"attribute vec2 textureCoordIn;\n"
		"\n"
		"uniform mat4 modelMatrix, viewMatrix, projectionMatrix;\n"
		"uniform mat3 normalMatrix;\n"
		"\n"
		"varying vec4 destinationColor;\n"
		"varying vec3 worldPos, worldNormal;\n"
		"varying vec2 textureCoordOut;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    "
		"    destinationColor = sourceColor;\n"
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
		"varying lowp vec4 destinationColor;\n"
		"varying lowp vec2 textureCoordOut;\n"
	   #else
		"varying vec3 worldPos, worldNormal;\n"
		"varying vec4 destinationColor;\n"
		"varying vec2 textureCoordOut;\n"
	   #endif
		"\n"
		"uniform vec3 lightPosition;\n"
		"uniform vec3 eyePosition;\n"
		"uniform vec3 userColor;\n"
		"uniform sampler2D textureSampler;\n"
		"\n"
		"void main()\n"
		"{\n"
	   #if JUCE_OPENGL_ES
		"   highp vec4 color = vec4(userColor, 1.0);\n"
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
		"   vec4 color = vec4(userColor, 1.0);\n"
		"\n"
		"   vec3 ambient = 0.3 * color.rgb;\n"
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
		"   spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);\n"
		"\n"
		"   vec3 specular = vec3(0.7) * spec;\n"
	   #endif
		"    gl_FragColor = vec4(ambient + diffuse + specular, color.a);\n"
		"}\n"
	};

	return shader;
}

static Shader getSkyShader()
{
	Shader skyShader =
	{
		"#version 420\n"
		"\n"
		"attribute vec4 position;\n"
		"attribute vec4 normal;\n"
		"attribute vec4 sourceColor;\n"
		"attribute vec2 textureCoordIn;\n"
		"\n"
		"uniform mat4 modelMatrix, viewMatrix, projectionMatrix;\n"
		"\n"
		"varying vec4 destinationColor;\n"
		"varying vec3 worldPos;\n"
		"varying vec2 textureCoordOut;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    "
		"    destinationColor = sourceColor;\n"
		"    textureCoordOut = textureCoordIn;\n"
		"    worldPos = vec3(modelMatrix * position);\n"
		"\n"
		"    gl_Position = projectionMatrix * viewMatrix * vec4(worldPos, 1.0);\n"
		"}\n",

		"#version 420\n"
		"\n"
	   #if JUCE_OPENGL_ES
		"varying lowp vec3 worldPos;\n"
		"varying lowp vec4 destinationColor;\n"
		"varying lowp vec2 textureCoordOut;\n"
		"\n"
		"uniform lowp float looper;\n"
	   #else
		"varying vec3 worldPos;\n"
		"varying vec4 destinationColor;\n"
		"varying vec2 textureCoordOut;\n"
		"\n"
		"uniform sampler2D permTexture;\n"
		"uniform sampler1D simplexTexture;\n"
		"uniform sampler2D gradTexture;\n"
		"uniform float looper;\n"
	   #endif
		"\n"
		"#define ONE 0.00390625\n"
		"#define ONEHALF 0.001953125\n"
		"float snoise(vec4 P) {\n"
		"// (sqrt(5.0)-1.0)/4.0\n"
		"#define F4 0.309016994375\n"
		"// (5.0-sqrt(5.0))/20.0\n"
		"#define G4 0.138196601125\n"
		"\n"
		"   float s = (P.x + P.y + P.z + P.w) * F4;\n"
		"   vec4 Pi = floor(P + s);\n"
		"   float t = (Pi.x + Pi.y + Pi.z + Pi.w) * G4;\n"
		"   vec4 P0 = Pi - t;\n"
		"   Pi = Pi * ONE + ONEHALF;\n"
		"   vec4 Pf0 = P - P0; // x,y distance from cell origin\n"
		"   \n"
		"   float c1 = (Pf0.x > Pf0.y) ? 0.5078125 : 0.0078125; // 1/2 + 1/128\n"
		"   float c2 = (Pf0.x > Pf0.z) ? 0.25 : 0.0;\n"
		"   float c3 = (Pf0.y > Pf0.z) ? 0.125 : 0.0;\n"
		"   float c4 = (Pf0.x > Pf0.w) ? 0.0625 : 0.0;\n"
		"   float c5 = (Pf0.y > Pf0.w) ? 0.03125 : 0.0;\n"
		"   float c6 = (Pf0.z > Pf0.w) ? 0.015625 : 0.0;\n"
		"   float sindex = c1 + c2 + c3 + c4 + c5 + c6;\n"
		"   vec4 offsets = texture1D(simplexTexture, sindex).rgba;\n"
		"   vec4 o1 = step(0.625, offsets);\n"
		"   vec4 o2 = step(0.375, offsets);\n"
		"   vec4 o3 = step(0.125, offsets);\n"
		"   \n"
		"   float perm0xy = texture2D(permTexture, Pi.xy).a;\n"
		"   float perm0zw = texture2D(permTexture, Pi.zw).a;\n"
		"   vec4  grad0 = texture2D(gradTexture, vec2(perm0xy, perm0zw)).rgba * 4.0 - 1.0;\n"
		"   float t0 = 0.6 - dot(Pf0, Pf0);\n"
		"   float n0;\n"
		"   if (t0 < 0.0) n0 = 0.0;\n"
		"   else {\n"
		"      t0 *= t0;\n"
		"      n0 = t0 * t0 * dot(grad0, Pf0);\n"
		"   }\n"
		"   \n"
		"   vec4 Pf1 = Pf0 - o1 + G4;\n"
		"   o1 = o1 * ONE;\n"
		"   float perm1xy = texture2D(permTexture, Pi.xy + o1.xy).a;\n"
		"   float perm1zw = texture2D(permTexture, Pi.zw + o1.zw).a;\n"
		"   vec4  grad1 = texture2D(gradTexture, vec2(perm1xy, perm1zw)).rgba * 4.0 - 1.0;\n"
		"   float t1 = 0.6 - dot(Pf1, Pf1);\n"
		"   float n1;\n"
		"   if (t1 < 0.0) n1 = 0.0;\n"
		"   else {\n"
		"      t1 *= t1;\n"
		"      n1 = t1 * t1 * dot(grad1, Pf1);\n"
		"   }\n"
		"   \n"
		"   vec4 Pf2 = Pf0 - o2 + 2.0 * G4;\n"
		"   o2 = o2 * ONE;\n"
		"   float perm2xy = texture2D(permTexture, Pi.xy + o2.xy).a;\n"
		"   float perm2zw = texture2D(permTexture, Pi.zw + o2.zw).a;\n"
		"   vec4  grad2 = texture2D(gradTexture, vec2(perm2xy, perm2zw)).rgba * 4.0 - 1.0;\n"
		"   float t2 = 0.6 - dot(Pf2, Pf2);\n"
		"   float n2;\n"
		"   if(t2 < 0.0) n2 = 0.0;\n"
		"   else {\n"
		"      t2 *= t2;\n"
		"      n2 = t2 * t2 * dot(grad2, Pf2);\n"
		"   }\n"
		"   \n"
		"   vec4 Pf3 = Pf0 - o3 + 3.0 * G4;\n"
		"   o3 = o3 * ONE;\n"
		"   float perm3xy = texture2D(permTexture, Pi.xy + o3.xy).a;\n"
		"   float perm3zw = texture2D(permTexture, Pi.zw + o3.zw).a;\n"
		"   vec4  grad3 = texture2D(gradTexture, vec2(perm3xy, perm3zw)).rgba * 4.0 - 1.0;\n"
		"   float t3 = 0.6 - dot(Pf3, Pf3);\n"
		"   float n3;\n"
		"   if(t3 < 0.0) n3 = 0.0;\n"
		"   else {\n"
		"      t3 *= t3;\n"
		"      n3 = t3 * t3 * dot(grad3, Pf3);\n"
		"   }\n"
		"   \n"
		"   vec4 Pf4 = Pf0 - vec4(1.0-4.0*G4);\n"
		"   float perm4xy = texture2D(permTexture, Pi.xy + vec2(ONE, ONE)).a;\n"
		"   float perm4zw = texture2D(permTexture, Pi.zw + vec2(ONE, ONE)).a;\n"
		"   vec4  grad4   = texture2D(gradTexture, vec2(perm4xy, perm4zw)).rgba * 4.0 - 1.0;\n"
		"   float t4 = 0.6 - dot(Pf4, Pf4);\n"
		"   float n4;\n"
		"   if(t4 < 0.0) n4 = 0.0;\n"
		"   else {\n"
		"      t4 *= t4;\n"
		"      n4 = t4 * t4 * dot(grad4, Pf4);\n"
		"   }\n"
		"   \n"
		"   return 27.0 * (n0 + n1 + n2 + n3 + n4);\n"
		"}\n"
		"void main()\n"
		"{\n"
	   #if JUCE_OPENGL_ES
		"   highp float n = snoise(vec4(4.0 * worldPos.xyz, 0.5*looper));\n"
	   #else
		"   float n = snoise(vec4(4.0 * worldPos.xyz, 0.5 * cos(looper)));\n"
	   #endif
		"    gl_FragColor = vec4(0.5 + 0.5 * vec3(n,n,n), 1.0);\n"
		"}\n"
	};

	return skyShader;
}
