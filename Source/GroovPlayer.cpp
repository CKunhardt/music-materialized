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
	sizeSlider.setRange(0.0, 1.0, 0.001);
	sizeSlider.addListener(this);

	addAndMakeVisible(zoomLabel);
	zoomLabel.attachToComponent(&sizeSlider, true);

	addAndMakeVisible(speedSlider);
	speedSlider.setRange(0.0, 0.5, 0.001);
	speedSlider.addListener(this);
	speedSlider.setSkewFactor(0.5f);

	addAndMakeVisible(speedLabel);
	speedLabel.attachToComponent(&speedSlider, true);

	addAndMakeVisible(showBackgroundToggle);
	showBackgroundToggle.onClick = [this] { renderer.doBackgroundDrawing = showBackgroundToggle.getToggleState(); };

	addAndMakeVisible(tabbedComp);
	tabbedComp.setTabBarDepth(25);
	tabbedComp.setColour(TabbedButtonBar::tabTextColourId, Colours::grey);
	tabbedComp.addTab("Vertex", Colours::transparentBlack, &vertexEditorComp, false);
	tabbedComp.addTab("Fragment", Colours::transparentBlack, &fragmentEditorComp, false);

	vertexDocument.addListener(this);
	fragmentDocument.addListener(this);

	textures.add(new Mesh::TextureFromAsset("portmeirion.jpg"));
	textures.add(new Mesh::TextureFromAsset("tile_background.png"));
	textures.add(new Mesh::TextureFromAsset("juce_icon.png"));
	textures.add(new Mesh::DynamicTexture());

	addAndMakeVisible(textureBox);
	textureBox.onChange = [this] { selectTexture(textureBox.getSelectedId()); };
	updateTexturesList();

	addAndMakeVisible(presetBox);
	presetBox.onChange = [this] { selectPreset(presetBox.getSelectedItemIndex()); };

	auto presets = getPresets();

	for (int i = 0; i < presets.size(); ++i)
		presetBox.addItem(presets[i].name, i + 1);

	addAndMakeVisible(presetLabel);
	presetLabel.attachToComponent(&presetBox, true);

	addAndMakeVisible(textureLabel);
	textureLabel.attachToComponent(&textureBox, true);

	lookAndFeelChanged();

}

void GroovPlayer::initialize()
{
	showBackgroundToggle.setToggleState(false, sendNotification);
	textureBox.setSelectedItemIndex(0);
	presetBox.setSelectedItemIndex(0);
	speedSlider.setValue(0.01);
	sizeSlider.setValue(0.5);
}



void GroovPlayer::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

	auto area = getLocalBounds().reduced(4);

	auto top = area.removeFromTop(75);

	auto sliders = top.removeFromRight(area.getWidth() / 2);
	showBackgroundToggle.setBounds(sliders.removeFromBottom(25));
	speedSlider.setBounds(sliders.removeFromBottom(25));
	sizeSlider.setBounds(sliders.removeFromBottom(25));

	top.removeFromRight(70);
	statusLabel.setBounds(top);

	auto shaderArea = area.removeFromBottom(area.getHeight() / 2);

	auto presets = shaderArea.removeFromTop(25);
	presets.removeFromLeft(100);
	presetBox.setBounds(presets.removeFromLeft(150));
	presets.removeFromLeft(100);
	textureBox.setBounds(presets);

	shaderArea.removeFromTop(4);
	tabbedComp.setBounds(shaderArea);
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

void GroovPlayer::selectPreset(int preset)
{
	const auto& p = getPresets()[preset];

	vertexDocument.replaceAllContent(p.vertexShader);
	fragmentDocument.replaceAllContent(p.fragmentShader);

	startTimer(1);
}

void GroovPlayer::selectTexture(int itemID)
{
#if JUCE_MODAL_LOOPS_PERMITTED
	if (itemID == 1000)
	{
		auto lastLocation = File::getSpecialLocation(File::userPicturesDirectory);

		FileChooser fc("Choose an image to open...", lastLocation, "*.jpg;*.jpeg;*.png;*.gif");

		if (fc.browseForFileToOpen())
		{
			lastLocation = fc.getResult();

			textures.add(new Mesh::TextureFromFile(fc.getResult()));
			updateTexturesList();

			textureBox.setSelectedId(textures.size());
		}
	}
	else
#endif
	{
		if (auto* t = textures[itemID - 1])
			renderer.setTexture(t);
	}
}

void GroovPlayer::updateTexturesList()
{
	textureBox.clear();

	for (int i = 0; i < textures.size(); ++i)
		textureBox.addItem(textures.getUnchecked(i)->name, i + 1);

#if JUCE_MODAL_LOOPS_PERMITTED
	textureBox.addSeparator();
	textureBox.addItem("Load from a file...", 1000);
#endif
}

void GroovPlayer::updateShader()
{
	startTimer(10);
}

void GroovPlayer::sliderValueChanged(Slider*)
{
	renderer.scale = (float)sizeSlider.getValue();
	renderer.rotationSpeed = (float)speedSlider.getValue();
}

void GroovPlayer::codeDocumentTextInserted(const String& /*newText*/, int /*insertIndex*/)
{
	startTimer(shaderLinkDelay);
}

void GroovPlayer::codeDocumentTextDeleted(int /*startIndex*/, int /*endIndex*/)
{
	startTimer(shaderLinkDelay);
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

	for (int i = tabbedComp.getNumTabs(); i >= 0; --i)
		tabbedComp.setTabBackgroundColour(i, editorBackground);

	vertexEditorComp.setColour(CodeEditorComponent::backgroundColourId, editorBackground);
	fragmentEditorComp.setColour(CodeEditorComponent::backgroundColourId, editorBackground);
}