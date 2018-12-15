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
class GroovAudioApp;

class GroovPlayer    :  public Component,
						private Slider::Listener
{
public:
    GroovPlayer(GroovRenderer& r);
	~GroovPlayer();

	//==============================================================================
	//void prepareToPlay(double, int) override {}
	//void releaseResources() override {}
	//void processBlock(AudioSampleBuffer&, MidiBuffer&) override {}

	////==============================================================================
	//AudioProcessorEditor* createEditor() override { return nullptr; }
	//bool hasEditor() const override { return false; }

	////==============================================================================
	//const String getName() const override { return {}; }
	//bool acceptsMidi() const override { return false; }
	//bool producesMidi() const override { return false; }
	//double getTailLengthSeconds() const override { return 0; }

	////==============================================================================
	//int getNumPrograms() override { return 0; }
	//int getCurrentProgram() override { return 0; }
	//void setCurrentProgram(int) override {}
	//const String getProgramName(int) override { return {}; }
	//void changeProgramName(int, const String&) override {}

	////==============================================================================
	//void getStateInformation(MemoryBlock&) override {}
	//void setStateInformation(const void*, int) override {}
	////==============================================================================


	void initialize();
	void resized() override;

	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseWheelMove(const MouseEvent&, const MouseWheelDetails& d) override;
	void mouseMagnify(const MouseEvent&, float magnifyAmount) override;

	enum ButtonName {
		OpenButton,
		PlayButton,
		StopButton
	};
	
	void changeButtonEnabled(ButtonName buttonName, bool state);

	void loadShaders();

	Label statusLabel;

private:
	void sliderValueChanged(Slider*) override;

	enum { shaderLinkDelay = 500 };

	void lookAndFeelChanged() override;

	void openButtonClicked();
	void playButtonClicked();
	void stopButtonClicked();

	void freezeBlocks();
	int lastBPM;
	bool frozen;

	GroovRenderer& renderer;
	std::unique_ptr<GroovAudioApp> audioApp;

	Label speedLabel{ {}, "Speed: " },
		wiggleLabel{ {}, "Wiggle: "},
		colorSatLabel{ {}, "Saturation: "},
		colorValLabel{ {}, "Brightness: "},
		zoomLabel{ {}, "Zoom: " },
		bpmLabel{ {}, "BPM: " };

	CodeDocument vertexDocument, fragmentDocument;
	
	Slider speedSlider, sizeSlider, bpmSlider, 
		wiggleSlider, colorSatSlider, colorValSlider;

	ToggleButton enableScaleBounce{ "Enable Bouncing" },
		freeze{ "FREEZE!" };

	TextButton openButton, playButton, stopButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroovPlayer)
};