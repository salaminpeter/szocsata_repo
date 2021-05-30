#include "EGL\egl.h"

#ifndef PLATFORM_ANDROID

#pragma once

struct OpenGLFunctions
{

	static EGLDisplay eglDisplay;
	static EGLint eglMajorVersion;
	static EGLint eglMinorVersion;
	static EGLConfig eglConfig;
	static EGLSurface eglSurface;
	static EGLContext eglContext;

	static void SwapBuffers()
	{
		BOOL B = eglSwapBuffers(eglDisplay, eglSurface);
	}

	static void InitOpenGL(HDC hDC, HWND hwnd);

};

#endif