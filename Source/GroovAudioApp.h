/*
  ==============================================================================

    GroovAudioApp.h
    Created: 5 Dec 2018 9:09:29am
    Author:  ClintonK

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class GroovPlayer;

class GroovAudioApp : public AudioAppComponent,
					  public ChangeListener
{
public:
	GroovAudioApp(GroovPlayer& p);
	~GroovAudioApp();

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;

	void changeListenerCallback(ChangeBroadcaster *source) override;

	enum TransportState
	{
		Stopped,
		Playing,
		Starting,
		Stopping
	};

	void transportStateChanged(TransportState newState);

	void playFile(File audioFile);
private:
	TransportState state;

	GroovPlayer& player;

	AudioFormatManager formatManager;
	std::unique_ptr<AudioFormatReaderSource> playSource;
	AudioTransportSource transport;
};
