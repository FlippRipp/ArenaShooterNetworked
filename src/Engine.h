#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "Key.h"

// Initializes the engine
// call this first
void engInit();

// Shuts down the engine
void engShutdown();

// Starts a frame, used like `while(engBeginFrame())`
bool engBeginFrame();

// *** RENDERING *** //

// Sets color to hex color in the form 0xRRGGBBAA
void engSetColor(unsigned int clr);

// Sets color with each color provided
void engSetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

// Clears the entire screen to the currently set color
void engClear();

// Renders a hollow rectangle at (x, y) with size (width, height)
void engRect(int x, int y, int width, int height);

// Fills a rectangle at (x, y) with size (width, height)
void engFillRect(int x, int y, int width, int height);

// Renders text (str) at location (x, y)
void engText(int x, int y, const char* str);
void engTextf(int x, int y, const char* format, ...);

// *** DEBUGGING *** //
void engPrint(const char* format, ...);

// *** INPUT *** //

// Returns if given key is currently pressed
bool engKeyDown(Key key);

// Returns if given key was pressed this frame
bool engKeyPressed(Key key);

// Returns if given key was released this frame
bool engKeyReleased(Key key);

// *** TIME *** //
float engDeltaTime();
float engElapsedTime();

// *** DEBUGGING *** //
template<typename T>
bool _debugDoOnceHelper(T uniqueLambda)
{
	static bool hasRun = false;
	if (hasRun)
		return false;

	hasRun = true;
	return true;
}

bool _engError(const char* format, ...);
#define DEBUG_BREAK (IsDebuggerPresent() && (__debugbreak(), 1))
#define DO_ONCE (_debugDoOnceHelper([]{}))
#define engError(format, ...) (DO_ONCE && _engError(format, __VA_ARGS__) && DEBUG_BREAK)
#define engErrorAlways(format, ...) (_engError(format, __VA_ARGS__) && DEBUG_BREAK)