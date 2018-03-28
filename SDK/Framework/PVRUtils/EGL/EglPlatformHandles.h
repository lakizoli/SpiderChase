/*!
\brief Contains platform objects required for EGL (EGLDisplay, EGLSurface, EGLContext etc).
\file PVRUtils/EGL/EglPlatformHandles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/CoreIncludes.h"
#if defined(WAYLAND)
#include <wayland-egl.h>
#endif
#include <DynamicEgl.h>

namespace pvr {
namespace platform {
/// <summary>EGL display type.</summary>
typedef EGLNativeDisplayType NativeDisplay;

/// <summary>EGL window type.</summary>
typedef EGLNativeWindowType NativeWindow;

/// <summary>Forward-declare and smart pointer friendly handle to all the objects that EGL needs to identify a rendering
/// context.</summary>
struct NativePlatformHandles_
{
	/// <summary>EGL display type.</summary>
	EGLDisplay display;

	/// <summary>EGL draw Surface.</summary>
	EGLSurface drawSurface;

	/// <summary>EGL read surface.</summary>
	EGLSurface readSurface;

	/// <summary>EGL context.</summary>
	EGLContext context;

#if defined(WAYLAND)
	wl_egl_window* eglWindow;
#endif

	/// <summary>Default constructor for the NativePlatformHandles_ structure initialising the display, drawSurface,
	/// readSurface and context to initial values.</summary>
	NativePlatformHandles_() : display(EGL_NO_DISPLAY), drawSurface(EGL_NO_SURFACE), readSurface(EGL_NO_SURFACE),
		context(EGL_NO_CONTEXT) {}
};

/// <summary>A structure containing the native handles defining a Shared Context, i.e. an EGL context that is suitable
/// for the Framework requires to use to upload textures and perform other functions in a different thread. Contains an
/// EGL context and the EGL p-buffer surface it is tied to.</summary>
struct NativeSharedPlatformHandles_
{
	/// <summary>EGL uploading context.</summary>
	EGLContext uploadingContext;

	/// <summary>EGL buffer surface.</summary>
	EGLSurface pBufferSurface;

	/// <summary>Default constructor for the NativeSharedPlatformHandles_ structure initialising the uploadingContext and pBufferSurface to initial values.</summary>
	NativeSharedPlatformHandles_() : pBufferSurface(EGL_NO_SURFACE), uploadingContext(EGL_NO_CONTEXT) {}
};
/// <summary>Forward-declare and smart pointer friendly handle to an EGL display</summary>
struct NativeDisplayHandle_
{
	/// <summary>A native display handle or EGL display type.</summary>
	NativeDisplay nativeDisplay;

	/// <summary>Operator() for the NativeDisplayHandle_ structure which retrieves the native display handle.</summary>
	/// <returns>Returns a reference to the native display handle.</returns>
	operator NativeDisplay& () { return nativeDisplay; }

	/// <summary>const Operator() for the NativeDisplayHandle_ structure which retrieves the native display handle.</summary>
	/// <returns>Returns a const reference to the native display handle.</returns>
	operator const NativeDisplay& () const { return nativeDisplay; }
};

/// <summary>Forward-declare and smart pointer friendly handle to an EGL window</summary>
struct NativeWindowHandle_
{
	/// <summary>A Native window or EGL window type.</summary>
	NativeWindow nativeWindow;

	/// <summary>Operator() for the NativeWindowHandle_ structure which retrieves the native window.</summary>
	/// <returns>Returns a reference to the native window.</returns>
	operator NativeWindow& () { return nativeWindow; }

	/// <summary>const Operator() for the NativeWindowHandle_ structure which retrieves the native window.</summary>
	/// <returns>Returns a const reference to the native window.</returns>
	operator const NativeWindow& () const { return nativeWindow; }
};

/// <summary>Pointer to a struct of platform handles. Used to pass around the undefined NativePlatformHandles_ struct.
/// </summary>
typedef RefCountedResource<NativePlatformHandles_> NativePlatformHandles;

/// <summary>Pointer to a struct of platform handles. Used to pass around the undefined NativePlatformHandles_ struct.
/// </summary>
typedef RefCountedResource<NativeDisplayHandle_> NativeDisplayHandle;

/// <summary>Pointer to a struct of Shared context handles. Used to pass around the undefined NativeSharedPlatformHandles_
/// struct</summary>
typedef RefCountedResource<NativeSharedPlatformHandles_> NativeSharedPlatformHandles;
}
}