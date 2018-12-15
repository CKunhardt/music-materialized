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

	const int PLAYER_WIDTH = 600;
	const int PLAYER_HEIGHT = 325;

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

	Label
		spinSpeedLabel{ {}, "Speed: " },
		wiggleLabel{ {}, "Wiggle: " },
		colorSatLabel{ {}, "Saturation: " },
		colorValLabel{ {}, "Brightness: " },
		bgHueLabel{ {}, "BG Hue: " },
		bgSatLabel{ {}, "BG Saturation" },
		zoomLabel{ {}, "Zoom: " },
		bpmLabel{ {}, "BPM: " },
		bgSpeedLabel{ {}, "BG Speed" },
		bgValLabel{ {}, "BG Val" };

	CodeDocument 
		vertexDocument, 
		fragmentDocument;
	
	Slider
		spinSpeedSlider,
		sizeSlider,
		bpmSlider,
		wiggleSlider,
		colorSatSlider,
		colorValSlider,
		bgHueSlider,
		bgSpeedSlider,
		bgSatSlider,
		bgValSlider;

	ToggleButton 
		enableScaleBounce{ "Enable Bouncing" },
		freeze{ "FREEZE!" };

	TextButton 
		openButton, 
		playButton, 
		stopButton;

	const int PARAM_HEIGHT = 25;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroovPlayer)
};