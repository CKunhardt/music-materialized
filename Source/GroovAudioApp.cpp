/*
  ==============================================================================

    GroovAudioApp.cpp
    Created: 5 Dec 2018 9:09:29am
    Author:  ClintonK

  ==============================================================================
*/

#include "GroovAudioApp.h"
#include "GroovPlayer.h"

GroovAudioApp::GroovAudioApp(GroovPlayer& p) : player(p), state(Stopped)
{
	setAudioChannels(0, 2);
	formatManager.registerBasicFormats();
	transport.addChangeListener(this);
}

GroovAudioApp::~GroovAudioApp()
{
	shutdownAudio();
}

void GroovAudioApp::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	transport.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void GroovAudioApp::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
	bufferToFill.clearActiveBufferRegion();
	transport.getNextAudioBlock(bufferToFill);
}

void GroovAudioApp::releaseResources()
{

}

void GroovAudioApp::changeListenerCallback(ChangeBroadcaster *source)
{
	if (source == &transport)
	{
		if (transport.isPlaying())
		{
			transportStateChanged(Playing);
		}
		else
		{
			transportStateChanged(Stopped);
		}
	}
}

void GroovAudioApp::transportStateChanged(TransportState newState)
{
	if (newState != state)
	{
		state = newState;
		switch (state)
		{
		case Stopped:
			transport.setPosition(0.0);
			break;
		case Playing:
			player.changeButtonEnabled(GroovPlayer::ButtonName::PlayButton, false);
			break;
		case Starting:
			player.changeButtonEnabled(GroovPlayer::ButtonName::StopButton, true);
			player.changeButtonEnabled(GroovPlayer::ButtonName::PlayButton, false);
			transport.start();
			break;
		case Stopping:
			player.changeButtonEnabled(GroovPlayer::ButtonName::StopButton, false);
			player.changeButtonEnabled(GroovPlayer::ButtonName::PlayButton, true);
			transport.stop();
			break;
		default:
			break;
		}
	}
}

void GroovAudioApp::playFile(File audioFile)
{
	AudioFormatReader* reader = formatManager.createReaderFor(audioFile);

	if (reader != nullptr)
	{
		std::unique_ptr<AudioFormatReaderSource> tempSource(new AudioFormatReaderSource(reader, true));

		transport.setSource(tempSource.get(),0,nullptr,reader->sampleRate);
		transportStateChanged(Stopping);

		playSource.reset(tempSource.release());
	}
}