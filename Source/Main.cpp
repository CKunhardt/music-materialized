/*
  ==============================================================================

	This file was auto-generated!

	It contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "GroovRenderer.h"
#include "GroovPlayer.h"

//==============================================================================
class GroovApplication : public JUCEApplication
{
public:
	//==============================================================================
	GroovApplication() {}

	const String getApplicationName() override { return ProjectInfo::projectName; }
	const String getApplicationVersion() override { return ProjectInfo::versionString; }
	bool moreThanOneInstanceAllowed() override { return true; }

	//==============================================================================
	void initialise(const String& commandLine) override
	{
		// This method is where you should put your application's initialisation code..
		renderer.reset(new GroovRenderer());
		mainWindow.reset(new MainWindow(getApplicationName(), renderer.get()));
		displayWindow.reset(new DisplayWindow(getApplicationName() + " Display", renderer.get()));
	}

	void shutdown() override
	{
		// Add your application's shutdown code here..
		renderer = nullptr;
		mainWindow = nullptr; // (deletes our window)
		displayWindow = nullptr;
	}

	//==============================================================================
	void systemRequestedQuit() override
	{
		// This is called when the app is being asked to quit: you can ignore this
		// request and let the app carry on running, or call quit() to allow the app to close.
		quit();
	}

	void anotherInstanceStarted(const String& commandLine) override
	{
		// When another instance of the app is launched while this one is running,
		// this method is invoked, and the commandLine parameter tells you what
		// the other instance's command-line arguments were.
	}

	//==============================================================================
	/*
		This class implements the desktop window that contains an instance of
		our MainComponent class.
	*/
	class MainWindow : public DocumentWindow
	{
	public:
		MainWindow(String name, GroovRenderer* r) : DocumentWindow(name,
			Desktop::getInstance().getDefaultLookAndFeel()
			.findColour(ResizableWindow::backgroundColourId),
			DocumentWindow::allButtons)
		{
			setUsingNativeTitleBar(true);
			setContentOwned(r->controlsOverlay.get(), true);
			setResizable(true, true);

			setSize(500, 500);

			centreWithSize(getWidth(), getHeight());
			setVisible(true);
		}

		void closeButtonPressed() override
		{
			// This is called when the user tries to close this window. Here, we'll just
			// ask the app to quit when this happens, but you can change this to do
			// whatever you need.
			JUCEApplication::getInstance()->systemRequestedQuit();
		}

		/* Note: Be careful if you override any DocumentWindow methods - the base
		   class uses a lot of them, so by overriding you might break its functionality.
		   It's best to do all your work in your content component instead, but if
		   you really have to override any DocumentWindow methods, make sure your
		   subclass also calls the superclass's method.
		*/

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
	};

	class DisplayWindow : public TopLevelWindow
	{
	public:
		DisplayWindow(String name, GroovRenderer* r) : TopLevelWindow(name, true)
		{
			addAndMakeVisible(r);
			if (Desktop::getInstance().getDisplays().displays.size() > 1) {
				auto screen = Desktop::getInstance().getDisplays().displays[1].totalArea;
				setBounds(screen);
			}
			else {
				auto screen = Desktop::getInstance().getDisplays().getMainDisplay().totalArea;
				setBounds(screen);
			}
			setOpaque(true);
			setVisible(true);
		}
		~DisplayWindow()
		{
			removeAllChildren();
		}

		void paint(Graphics& g) {};
	};

private:
	std::unique_ptr<GroovRenderer> renderer;
	std::unique_ptr<MainWindow> mainWindow;
	std::unique_ptr<DisplayWindow> displayWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(GroovApplication)
