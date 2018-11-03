/*
  ==============================================================================

    GroovPlayer.h
    Created: 2 Nov 2018 2:56:27pm
    Author:  ClintonK

  ==============================================================================
*/

#pragma once

#include "Mesh.h"
#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/

class GroovRenderer;

class GroovPlayer    :  public Component,
						private CodeDocument::Listener,
						private Slider::Listener,
						private Timer
{
public:
    GroovPlayer(GroovRenderer& r);

	void initialize();
	void resized() override;

	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseWheelMove(const MouseEvent&, const MouseWheelDetails& d) override;
	void mouseMagnify(const MouseEvent&, float magnifyAmount) override;

	void selectPreset(int preset);

	void selectTexture(int itemID);

	void updateTexturesList();

	void updateShader();

	Label statusLabel;

private:
	void sliderValueChanged(Slider*) override;
	enum { shaderLinkDelay = 500 };
	void codeDocumentTextInserted(const String& /*newText*/, int /*insertIndex*/) override;
	void codeDocumentTextDeleted(int /*startIndex*/, int /*endIndex*/) override;
	void timerCallback() override;
	void lookAndFeelChanged() override;

	GroovRenderer& renderer;

	Label speedLabel{ {}, "Speed: " },
		zoomLabel{ {}, "Zoom: " };

	CodeDocument vertexDocument, fragmentDocument;
	CodeEditorComponent vertexEditorComp{ vertexDocument, nullptr },
		fragmentEditorComp{ fragmentDocument, nullptr };

	TabbedComponent tabbedComp{ TabbedButtonBar::TabsAtLeft };
	ComboBox presetBox, textureBox;

	Label presetLabel{ {}, "Shader Preset: " },
		textureLabel{ {}, "Texture: " };

	Slider speedSlider, sizeSlider;

	ToggleButton showBackgroundToggle{ "Draw 2D graphics in background" };

	OwnedArray<Mesh::Texture> textures;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroovPlayer)
};