# Music Materialized
Clinton Kunhardt & Ansel Colby
A final project for COMP 465 - Interactive Computer Graphics

## What is Music Materialized?
Music Materialized is a toolkit for a graphical DJ. 
Events that have a DJ will often times accompany the music with a graphical representation of the music, 
but usually this is just a predetermined set of visuals that does not have the capability of responding 
fluidly to the music. Our goal for this project was to create a set of visuals that are almost completely
tweakable during the performance, allowing for a greater level of control and the ability to have a 
graphical representation of music that feels alive.

## Overall Architecture
To accomplish this task we implemented JUCE.
**Main.cpp** handles window creation.
**GroovPlayer.cpp** contains the user interface aspects of the application. Loading the audio file takes 
place here as well the creation of all of the sliders and buttons that allow the graphics to be tweaked.
GroovPlayer talks to GroovAudioApp and GroovRenderer.
**GroovRenderer.cpp** is an OpenGLRenderer and handles all of the graphical elements. All of the OpenGL 
work is accomplished here.
**GroovAudioApp.cpp** is an AudioAppComponent that handles starting and stopping the audio file. 
**Mesh.h** is an implementation of the Mesh class from Bret Jackson's BasicGraphics repository.
**Shaders.h** contains the shaders we implemented.
**GLMHelpers.h** provides some helper functions for conversions.
**Utilities.h** contains a bunch of miscellaneous utilities that are used by the various JUCE demos.
**WavefrontObjParser.h**  is a parser for the 3D OBJ file format provided by JUCE.

## How to build and run


## Credits

