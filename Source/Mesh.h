/*
  ==============================================================================

    Mesh.h
    Created: 2 Nov 2018 3:08:20pm
    Author:  ClintonK
	Notes: Adapted from JUCE's example OpenGLDemo.h

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Utilities.h"
#include "WavefrontObjParser.h"

class Mesh {
public:
	/** Vertex data to be passed to the shaders.*/
	struct Vertex
	{
		float position[3];
		float normal[3];
		float color[4];
		float texCoord[2];
	};

	struct Attributes
	{
		Attributes(OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
		{
			position.reset(createAttribute(openGLContext, shader, "position"));
			normal.reset(createAttribute(openGLContext, shader, "normal"));
			sourceColor.reset(createAttribute(openGLContext, shader, "sourceColor"));
			textureCoordIn.reset(createAttribute(openGLContext, shader, "textureCoordIn"));
		}

		void enable(OpenGLContext& openGLContext)
		{
			if (position.get() != nullptr)
			{
				openGLContext.extensions.glVertexAttribPointer(position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
				openGLContext.extensions.glEnableVertexAttribArray(position->attributeID);
			}

			if (normal.get() != nullptr)
			{
				openGLContext.extensions.glVertexAttribPointer(normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 3));
				openGLContext.extensions.glEnableVertexAttribArray(normal->attributeID);
			}

			if (sourceColor.get() != nullptr)
			{
				openGLContext.extensions.glVertexAttribPointer(sourceColor->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 6));
				openGLContext.extensions.glEnableVertexAttribArray(sourceColor->attributeID);
			}

			if (textureCoordIn.get() != nullptr)
			{
				openGLContext.extensions.glVertexAttribPointer(textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 10));
				openGLContext.extensions.glEnableVertexAttribArray(textureCoordIn->attributeID);
			}
		}

		void disable(OpenGLContext& openGLContext)
		{
			if (position.get() != nullptr)        openGLContext.extensions.glDisableVertexAttribArray(position->attributeID);
			if (normal.get() != nullptr)          openGLContext.extensions.glDisableVertexAttribArray(normal->attributeID);
			if (sourceColor.get() != nullptr)    openGLContext.extensions.glDisableVertexAttribArray(sourceColor->attributeID);
			if (textureCoordIn.get() != nullptr)  openGLContext.extensions.glDisableVertexAttribArray(textureCoordIn->attributeID);
		}

		std::unique_ptr<OpenGLShaderProgram::Attribute> position, normal, sourceColor, textureCoordIn;

	private:
		static OpenGLShaderProgram::Attribute* createAttribute(OpenGLContext& openGLContext,
			OpenGLShaderProgram& shader,
			const char* attributeName)
		{
			if (openGLContext.extensions.glGetAttribLocation(shader.getProgramID(), attributeName) < 0)
				return nullptr;

			return new OpenGLShaderProgram::Attribute(shader, attributeName);
		}
	};

	//==============================================================================
	// This struct manages all of our uniforms that are passed through to the shader

	struct Uniforms
	{
		Uniforms(OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
		{
			modelMatrix.reset(createUniform(openGLContext, shader, "modelMatrix"));
			projectionMatrix.reset(createUniform(openGLContext, shader, "projectionMatrix"));
			viewMatrix.reset(createUniform(openGLContext, shader, "viewMatrix"));
			normalMatrix.reset(createUniform(openGLContext, shader, "normalMatrix"));
			texture.reset(createUniform(openGLContext, shader, "textureSampler"));
			permTexture.reset(createUniform(openGLContext, shader, "permTexture"));
			simplexTexture.reset(createUniform(openGLContext, shader, "simplexTexture"));
			gradTexture.reset(createUniform(openGLContext, shader, "gradTexture"));
			amounts.reset(createUniform(openGLContext, shader, "amounts"));
			eyePosition.reset(createUniform(openGLContext, shader, "eyePosition"));
			lightPosition.reset(createUniform(openGLContext, shader, "lightPosition"));
			userColor.reset(createUniform(openGLContext, shader, "userColor"));
			looper.reset(createUniform(openGLContext, shader, "looper"));
		}

		std::unique_ptr<OpenGLShaderProgram::Uniform> 
			modelMatrix,
			projectionMatrix,
			viewMatrix,
			normalMatrix,
			texture,
			permTexture,
			simplexTexture,
			gradTexture,
			amounts,
			eyePosition,
			lightPosition,
			userColor,
			looper;

	private:
		static OpenGLShaderProgram::Uniform* createUniform(OpenGLContext& openGLContext,
			OpenGLShaderProgram& shader,
			const char* uniformName)
		{
			if (openGLContext.extensions.glGetUniformLocation(shader.getProgramID(), uniformName) < 0)
				return nullptr;

			return new OpenGLShaderProgram::Uniform(shader, uniformName);
		}
	};

	//==============================================================================
	/** This loads a 3D model from an OBJ file and converts it into some vertex buffers
		that we can draw. Copyright JUCE
	*/
	struct Shape
	{
		Shape(OpenGLContext& openGLContext, String name)
		{
			if (shapeFile.load(loadEntireAssetIntoString(name.toStdString().c_str())).wasOk())
				for (auto* s : shapeFile.shapes)
					vertexBuffers.add(new VertexBuffer(openGLContext, *s));
		}

		void draw(OpenGLContext& openGLContext, Attributes& attributes)
		{
			for (auto* vertexBuffer : vertexBuffers)
			{
				vertexBuffer->bind();

				attributes.enable(openGLContext);
				glDrawElements(GL_TRIANGLES, vertexBuffer->numIndices, GL_UNSIGNED_INT, 0);
				attributes.disable(openGLContext);
			}
		}

	private:
		struct VertexBuffer
		{
			VertexBuffer(OpenGLContext& context, WavefrontObjFile::Shape& shape) : openGLContext(context)
			{
				numIndices = shape.mesh.indices.size();

				openGLContext.extensions.glGenBuffers(1, &vertexBuffer);
				openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

				Array<Vertex> vertices;
				createVertexListFromMesh(shape.mesh, vertices, Colours::green);

				openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, vertices.size() * (int) sizeof(Vertex),
					vertices.getRawDataPointer(), GL_STATIC_DRAW);

				openGLContext.extensions.glGenBuffers(1, &indexBuffer);
				openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
				openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * (int) sizeof(juce::uint32),
					shape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
			}

			~VertexBuffer()
			{
				openGLContext.extensions.glDeleteBuffers(1, &vertexBuffer);
				openGLContext.extensions.glDeleteBuffers(1, &indexBuffer);
			}

			void bind()
			{
				openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
				openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			}

			GLuint vertexBuffer, indexBuffer;
			int numIndices;
			OpenGLContext& openGLContext;

			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VertexBuffer)
		};

		WavefrontObjFile shapeFile;
		OwnedArray<VertexBuffer> vertexBuffers;

		static void createVertexListFromMesh(const WavefrontObjFile::Mesh& mesh, Array<Vertex>& list, Colour colour)
		{
			auto scale = 0.2f;
			WavefrontObjFile::TextureCoord defaultTexCoord = { 0.5f, 0.5f };
			WavefrontObjFile::Vertex defaultNormal = { 0.5f, 0.5f, 0.5f };

			for (int i = 0; i < mesh.vertices.size(); ++i)
			{
				auto& v = mesh.vertices.getReference(i);

				auto& n = (i < mesh.normals.size() ? mesh.normals.getReference(i)
					: defaultNormal);

				auto& tc = (i < mesh.textureCoords.size() ? mesh.textureCoords.getReference(i)
					: defaultTexCoord);

				list.add({ { scale * v.x, scale * v.y, scale * v.z, },
							{ scale * n.x, scale * n.y, scale * n.z, },
							{ colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
							{ tc.x, tc.y } });
			}
		}
	};

	//==============================================================================
		// These classes are used to load textures from the various sources that the program uses..
	struct Texture
	{
		virtual ~Texture() {}
		virtual bool applyTo(OpenGLTexture&) = 0;

		String name;
	};

	struct DynamicTexture : public Texture
	{
		DynamicTexture() { name = "Dynamically-generated texture"; }

		Image image;
		BouncingNumber x, y;

		bool applyTo(OpenGLTexture& texture) override
		{
			int size = 128;

			if (!image.isValid())
				image = Image(Image::ARGB, size, size, true);

			{
				Graphics g(image);
				g.fillAll(Colours::lightcyan);

				g.setColour(Colours::darkred);
				g.drawRect(0, 0, size, size, 2);

				g.setColour(Colours::green);
				g.fillEllipse(x.getValue() * size * 0.9f, y.getValue() * size * 0.9f, size * 0.1f, size * 0.1f);

				g.setColour(Colours::black);
				g.setFont(40);
				g.drawFittedText(String(Time::getCurrentTime().getMilliseconds()), image.getBounds(), Justification::centred, 1);
			}

			texture.loadImage(image);
			return true;
		}
	};

	struct BuiltInTexture : public Texture
	{
		BuiltInTexture(const char* nm, const void* imageData, size_t imageSize)
			: image(resizeImageToPowerOfTwo(ImageFileFormat::loadFrom(imageData, imageSize)))
		{
			name = nm;
		}

		Image image;

		bool applyTo(OpenGLTexture& texture) override
		{
			texture.loadImage(image);
			return false;
		}
	};

	struct TextureFromFile : public Texture
	{
		TextureFromFile(const File& file)
		{
			name = file.getFileName();
			image = resizeImageToPowerOfTwo(ImageFileFormat::loadFrom(file));
		}

		Image image;

		bool applyTo(OpenGLTexture& texture) override
		{
			texture.loadImage(image);
			return false;
		}
	};

	struct TextureFromAsset : public Texture
	{
		TextureFromAsset(const char* assetName)
		{
			name = assetName;
			image = resizeImageToPowerOfTwo(getImageFromAssets(assetName));
		}

		Image image;

		bool applyTo(OpenGLTexture& texture) override
		{
			texture.loadImage(image);
			return false;
		}
	};

	static Image resizeImageToPowerOfTwo(Image image)
	{
		if (!(isPowerOfTwo(image.getWidth()) && isPowerOfTwo(image.getHeight())))
			return image.rescaled(jmin(1024, nextPowerOfTwo(image.getWidth())),
				jmin(1024, nextPowerOfTwo(image.getHeight())));

		return image;
	};
};
