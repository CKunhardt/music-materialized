/*
  ==============================================================================

    GroovPlayer.cpp
    Created: 2 Nov 2018 2:56:27pm
    Author:  ClintonK

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Mesh.h"
#include "Shaders.h"
#include "GroovPlayer.h"
#include "GroovRenderer.h"
#include "GroovAudioApp.h"
#include "Utilities.h"

//==============================================================================
GroovPlayer::GroovPlayer(GroovRenderer& r) : renderer(r)
{
	audioApp.reset(new GroovAudioApp (*this));

	addAndMakeVisible(statusLabel);
	statusLabel.setJustificationType(Justification::topLeft);
	statusLabel.setFont(Font(14.0f));

	// SLIDERS -----------------------
	addAndMakeVisible(sizeSlider);
	sizeSlider.setRange(1.0, 3.0, 0.01);
	sizeSlider.addListener(this);

	addAndMakeVisible(zoomLabel);
	zoomLabel.attachToComponent(&sizeSlider, true);

	addAndMakeVisible(speedSlider);
	speedSlider.setRange(0.0, 0.5, 0.001);
	speedSlider.addListener(this);
	speedSlider.setSkewFactor(0.5f);

	addAndMakeVisible(speedLabel);
	speedLabel.attachToComponent(&speedSlider, true);

	addAndMakeVisible(wiggleSlider);
	wiggleSlider.setRange(0.0, 10.0, 0.001);
	wiggleSlider.addListener(this);

	addAndMakeVisible(wiggleLabel);
	wiggleLabel.attachToComponent(&wiggleSlider, true);

	addAndMakeVisible(colorSatSlider);
	colorSatSlider.setRange(0.0, 1.0, 0.001);
	colorSatSlider.addListener(this);

	addAndMakeVisible(colorSatLabel);
	colorSatLabel.attachToComponent(&colorSatSlider, true);

	addAndMakeVisible(colorValSlider);
	colorValSlider.setRange(0.0, 1.0, 0.001);
	colorValSlider.addListener(this);

	addAndMakeVisible(colorValLabel);
	colorValLabel.attachToComponent(&colorValSlider, true);

	addAndMakeVisible(bgHueSlider);
	bgHueSlider.setRange(0.0, 360.0, 1.0);
	bgHueSlider.addListener(this);

	addAndMakeVisible(bgHueLabel);
	bgHueLabel.attachToComponent(&bgHueSlider, true);

	addAndMakeVisible(bpmSlider);
	bpmSlider.setRange(0, 250, 1);
	bpmSlider.addListener(this);

	addAndMakeVisible(bpmLabel);
	bpmLabel.attachToComponent(&bpmSlider, true);

	// TOGGLE BUTTONS -----------------------

	// this button toggles the feature that bounces the cubes
	addAndMakeVisible(enableScaleBounce);
	enableScaleBounce.onClick = [this] {renderer.doScaleBounce = enableScaleBounce.getToggleState(); };

	// this button toggles the feature that 'freezes' the animation by setting BPM to 0
	addAndMakeVisible(freeze);
	freeze.onClick = [this] { freezeBlocks(); };

	// FILE LOADING BUTTONS -----------------------
	addAndMakeVisible(&openButton);
	openButton.setButtonText("Open File");
	openButton.onClick = [this] { openButtonClicked(); };

	addAndMakeVisible(&playButton);
	playButton.setButtonText("PLAY");
	playButton.onClick = [this] { playButtonClicked(); };
	playButton.setColour(TextButton::buttonColourId, Colours::green);
	playButton.setEnabled(true);

	addAndMakeVisible(&stopButton);
	stopButton.setButtonText("STOP");
	stopButton.onClick = [this] { stopButtonClicked(); };
	stopButton.setColour(TextButton::buttonColourId, Colours::red);
	stopButton.setEnabled(false);

	lookAndFeelChanged();
}

GroovPlayer::~GroovPlayer()
{
	
}

void GroovPlayer::initialize()
{
	speedSlider.setValue(0.01);
	sizeSlider.setValue(2.5);

	bpmSlider.setValue(125);
	lastBPM = 125;
	frozen = false;

	wiggleSlider.setValue(4.0);
	colorSatSlider.setValue(0.5);
	colorValSlider.setValue(1.0);
	bgHueSlider.setValue(180.0);

	loadShaders();
}



void GroovPlayer::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

	auto area = getLocalBounds().reduced(4);

	auto top = area.removeFromTop(225);

	auto musicControls = top.removeFromLeft((area.getWidth() / 2) - 90);
	musicControls = musicControls.removeFromTop(100);
	stopButton.setBounds(musicControls.removeFromBottom(25));
	playButton.setBounds(musicControls.removeFromBottom(25));
	openButton.setBounds(musicControls.removeFromBottom(50));

	auto controls = top.removeFromRight(area.getWidth() / 2);
	freeze.setBounds(controls.removeFromBottom(25));
	enableScaleBounce.setBounds(controls.removeFromBottom(25));
	bgHueSlider.setBounds(controls.removeFromBottom(25));
	colorSatSlider.setBounds(controls.removeFromBottom(25));
	colorValSlider.setBounds(controls.removeFromBottom(25));
	wiggleSlider.setBounds(controls.removeFromBottom(25));
	bpmSlider.setBounds(controls.removeFromBottom(25));
	speedSlider.setBounds(controls.removeFromBottom(25));
	sizeSlider.setBounds(controls.removeFromBottom(25));
	

	top.removeFromRight(70);
	statusLabel.setBounds(top);
}

void GroovPlayer::mouseDown(const MouseEvent& e)
{
}

void GroovPlayer::mouseDrag(const MouseEvent& e)
{
}

void GroovPlayer::mouseWheelMove(const MouseEvent&, const MouseWheelDetails& d)
{
	sizeSlider.setValue(sizeSlider.getValue() + d.deltaY);
}

void GroovPlayer::mouseMagnify(const MouseEvent&, float magnifyAmmount)
{
	sizeSlider.setValue(sizeSlider.getValue() + magnifyAmmount - 1.0f);
}

void GroovPlayer::loadShaders()
{
	const auto& shader = getShader();
	const auto& shaderSky = getSkyShader();

	renderer.setShaderProgram(shader.vertexShader, shader.fragmentShader, false);
	renderer.setShaderProgram(shaderSky.vertexShader, shaderSky.fragmentShader, true);
}

void GroovPlayer::sliderValueChanged(Slider*)
{
	renderer.scale =  (float)sizeSlider.getValue();
	renderer.rotationSpeed = (float)speedSlider.getValue();
	renderer.bpm = (int)bpmSlider.getValue();
	renderer.wiggleSpeed = (float)wiggleSlider.getValue();
	renderer.colorSat = (float)colorSatSlider.getValue();
	renderer.colorVal = (float)colorValSlider.getValue();
	renderer.bgHue = (float)bgHueSlider.getValue();
}

void GroovPlayer::lookAndFeelChanged()
{
	auto editorBackground = getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
		Colours::white);
}

void GroovPlayer::freezeBlocks()
{
	if (!frozen)
	{
		lastBPM = bpmSlider.getValue();
		bpmSlider.setValue(0);
		frozen = true;
	}
	else
	{
		bpmSlider.setValue(lastBPM);
		frozen = false;
	}
	
}

void GroovPlayer::openButtonClicked()
{
	DBG("clicked");
	FileChooser chooser ("Choose an Mp3 or Wav File", getProgramDirectory().getChildFile("Assets"), "*.wav; *.mp3");
	
	if (chooser.browseForFileToOpen())
	{
		audioApp->playFile(chooser.getResult());
	}
}

void GroovPlayer::playButtonClicked()
{
	audioApp->transportStateChanged(GroovAudioApp::TransportState::Starting);
	renderer.startPlaying();
}

void GroovPlayer::stopButtonClicked()
{
	audioApp->transportStateChanged(GroovAudioApp::TransportState::Stopping);
	renderer.stopPlaying();
}

void GroovPlayer::changeButtonEnabled(ButtonName buttonName, bool state)
{
	Button::ButtonState newState = (Button::ButtonState)state;
	switch (buttonName)
	{
	case OpenButton:
		openButton.setEnabled(newState);
		break;
	case PlayButton:
		playButton.setEnabled(newState);
		break;
	case StopButton:
		stopButton.setEnabled(newState);
		break;
	}
}

