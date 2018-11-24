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

//==============================================================================
GroovPlayer::GroovPlayer(GroovRenderer& r)
	: renderer(r)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
	addAndMakeVisible(statusLabel);
	statusLabel.setJustificationType(Justification::topLeft);
	statusLabel.setFont(Font(14.0f));

	addAndMakeVisible(sizeSlider);
	sizeSlider.setRange(1.0, 4.0, 0.01);
	sizeSlider.addListener(this);

	addAndMakeVisible(zoomLabel);
	zoomLabel.attachToComponent(&sizeSlider, true);

	addAndMakeVisible(speedSlider);
	speedSlider.setRange(0.0, 0.5, 0.001);
	speedSlider.addListener(this);
	speedSlider.setSkewFactor(0.5f);

	addAndMakeVisible(speedLabel);
	speedLabel.attachToComponent(&speedSlider, true);

	addAndMakeVisible(enableScaleBounce);
	enableScaleBounce.onClick = [this] {renderer.doScaleBounce = enableScaleBounce.getToggleState(); };

	addAndMakeVisible(bpmSlider);
	bpmSlider.setRange(60, 180, 1);
	bpmSlider.addListener(this);

	addAndMakeVisible(bpmLabel);
	bpmLabel.attachToComponent(&bpmSlider, true);

	textures.add(new Mesh::TextureFromAsset("background.png"));

	lookAndFeelChanged();

}

void GroovPlayer::initialize()
{
	speedSlider.setValue(0.01);
	sizeSlider.setValue(2.5);
	bpmSlider.setValue(120);

	selectTexture(1);
	loadShaders();
}



void GroovPlayer::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

	auto area = getLocalBounds().reduced(4);

	auto top = area.removeFromTop(100);

	auto sliders = top.removeFromRight(area.getWidth() / 2);
	enableScaleBounce.setBounds(sliders.removeFromBottom(25));
	bpmSlider.setBounds(sliders.removeFromBottom(25));
	speedSlider.setBounds(sliders.removeFromBottom(25));
	sizeSlider.setBounds(sliders.removeFromBottom(25));

	top.removeFromRight(70);
	statusLabel.setBounds(top);
}

void GroovPlayer::mouseDown(const MouseEvent& e)
{
	renderer.draggableOrientation.mouseDown(e.getPosition());
}

void GroovPlayer::mouseDrag(const MouseEvent& e)
{
	renderer.draggableOrientation.mouseDrag(e.getPosition());
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
	const auto& p = getShader();

	vertexDocument.replaceAllContent(p.vertexShader);
	fragmentDocument.replaceAllContent(p.fragmentShader);

	startTimer(1);
}

void GroovPlayer::selectTexture(int itemID)
{
	if (auto* t = textures[itemID - 1])
		renderer.setTexture(t);
}

void GroovPlayer::updateShader()
{
	startTimer(10);
}

void GroovPlayer::sliderValueChanged(Slider*)
{
	renderer.scale =  (float)sizeSlider.getValue();
	renderer.rotationSpeed = (float)speedSlider.getValue();
	renderer.bpm = (int)bpmSlider.getValue();
}

void GroovPlayer::timerCallback()
{
	stopTimer();
	renderer.setShaderProgram(vertexDocument.getAllContent(),
		fragmentDocument.getAllContent());
}

void GroovPlayer::lookAndFeelChanged()
{
	auto editorBackground = getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
		Colours::white);
}