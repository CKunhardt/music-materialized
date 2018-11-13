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

	void loadShaders();

	void selectTexture(int itemID);

	void updateShader();

	Label statusLabel;

private:
	void sliderValueChanged(Slider*) override;
	enum { shaderLinkDelay = 500 };
	void timerCallback() override;
	void lookAndFeelChanged() override;

	GroovRenderer& renderer;

	Label speedLabel{ {}, "Speed: " },
		zoomLabel{ {}, "Zoom: " },
		bpmLabel{ {}, "BPM: " };

	CodeDocument vertexDocument, fragmentDocument;

	Slider speedSlider, sizeSlider, bpmSlider;

	ToggleButton enableScaleBounce{ "Enable Bouncing" };

	OwnedArray<Mesh::Texture> textures;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroovPlayer)
};