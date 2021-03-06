#include "stdafx.h"

/*!*********************************************************************************************************************
\File         GameApplication.cpp
\Title        Introduces PVRShell
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief        Shows how to use the PowerVR framework for initialization. This framework allows platform abstraction so applications
using it will work on any PowerVR enabled device.
***********************************************************************************************************************/

/*
* The PowerVR Shell
* ===========================

The PowerVR shell handles all OS specific initialisation code, and is
extremely convenient for writing portable applications. It also has
several built in command line features, which allow you to specify
attributes like the backbuffer size, vsync and antialiasing modes.

The code is constructed around a "PVRShell" superclass. You must define
your app using a class which inherits from this, which should implement
the following five methods, (which at execution time are essentially
called in the order in which they are listed):

initApplication:  This is called before any API initialisation has
taken place, and can be used to set up any application data which does
not require API calls, for example object positions, or arrays containing
vertex data, before they are uploaded.

initView: This is called after the API has initialized, and can be
used to do any remaining initialisation which requires API functionality.
In this app, it is used to upload the vertex data.

renderFrame:  This is called repeatedly to draw the geometry. Returning
false from this function instructs the app to enter the quit sequence:

releaseView:  This function is called before the API is released, and
is used to release any API resources. In this app, it releases the
vertex buffer.

quitApplication:  This is called last of all, after the API has been
released, and can be used to free any leftover user allocated memory.

The shell framework starts the application by calling a "NewDemo" function,
which must return an instance of the PVRShell class you defined. We will
now use the shell to create a "Hello triangle" app, similar to the previous
one.

*/
#include "Base/EglContext.h"
#include "Game.hpp"
#include <PVRAssets/PVRAssets.h>

/*!*********************************************************************************************************************
To use the shell, you have to inherit a class from PVRShell
and implement the five virtual functions which describe how your application initializes, runs and releases the resources.
***********************************************************************************************************************/
class GameApplication : public pvr::Shell {
	EglContext _context;

public:
	// following function must be override
	virtual pvr::Result initApplication ();
	virtual pvr::Result initView ();
	virtual pvr::Result releaseView ();
	virtual pvr::Result quitApplication ();
	virtual pvr::Result renderFrame ();

	virtual void eventKeyDown (pvr::Keys key);
	virtual void eventKeyUp (pvr::Keys key);
	virtual void eventDragStart (int buttonIdx, pvr::PointerLocation location);
	virtual void eventDragFinished (pvr::PointerLocation location);
};

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result GameApplication::initApplication () { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
If the rendering context is lost, QuitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result GameApplication::quitApplication () { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return Return Result::Success if no error occured
\brief  Code in initView() will be called by pvr::Shell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependant on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result GameApplication::initView () {
	//TODO: implement support for resizable window!

	//Initialize the PowerVR OpenGL bindings. Must be called before using any of the gl:: commands.
	_context.init (getWindow (), getDisplay (), getDisplayAttributes ());

	std::function <pvr::Shell::PointerNormalisedLocation(void)> setPointerLocationGetter (
		[this] () {
			return getPointerNormalisedPosition ();
		}
	);

	//Game::Get ().GetInputHandler ().SetPointerLocationGetter (setPointerLocationGetter);

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in releaseView() will be called by pvr::Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result GameApplication::releaseView () {
	_context.release ();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result GameApplication::renderFrame () {
	//Run the game
	Game& game = Game::Get ();

	double timeInSec = getTime () / 1000.0;
	if (!game.Step (timeInSec)) {
		return pvr::Result::ExitRenderFrame;
	}

	_context.swapBuffers ();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell.
The user should return its PVRShell object defining the behaviour of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo () { return std::unique_ptr<pvr::Shell> (new GameApplication ()); }

void GameApplication::eventKeyDown (pvr::Keys key) {
	InputHandler& inputHandler = Game::Get ().GetInputHandler ();
	inputHandler.OnKeyDown (key);
}

void GameApplication::eventKeyUp (pvr::Keys key) {
	InputHandler& inputHandler = Game::Get ().GetInputHandler ();
	inputHandler.OnKeyUp (key);
}

void GameApplication::eventDragStart (int buttonIdx, pvr::PointerLocation location) {
	InputHandler& inputHandler = Game::Get().GetInputHandler();
	//inputHandler.DragStart ();

	//std::cout << "Start " << location.x << ":" << location.y << std::endl;
	//Log(LogLevel::Information, "Start %lf : %lf /n", location.x, location.y);
}

void GameApplication::eventDragFinished (pvr::PointerLocation location) {
	InputHandler& inputHandler = Game::Get().GetInputHandler();
	std::cout << location.x << ":" << location.y << std::endl;
	//inputHandler.DragFinished ();

	//std::cout << "End " << location.x << ":" << location.y << std::endl;
	//Log(LogLevel::Information, "EGL context creation: EGL_KHR_create_context supported");
}

