#include "stdafx.h"

#ifndef PLATFORM_ANDROID

#include "EGL\egl.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include "opengl.h"


EGLDisplay OpenGLFunctions::eglDisplay;
EGLint OpenGLFunctions::eglMajorVersion;
EGLint OpenGLFunctions::eglMinorVersion;
EGLConfig OpenGLFunctions::eglConfig;
EGLSurface OpenGLFunctions::eglSurface;
EGLContext OpenGLFunctions::eglContext;


void OpenGLFunctions::InitOpenGL(HDC hDC, HWND hwnd)
{
	
		const EGLint ConfigAttribs[] = {
		EGL_LEVEL, 0,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
		EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
		EGL_RED_SIZE,       8,
		EGL_GREEN_SIZE,     8,
		EGL_BLUE_SIZE,      8,
		EGL_DEPTH_SIZE,     24,
		EGL_NONE
		};

		const EGLint ContextAttribs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 3,
			EGL_NONE
		};


		int Configs;
		bool Success;

		eglDisplay = eglGetDisplay(NativeDisplayType(hDC));
		Success = eglInitialize(eglDisplay, &eglMajorVersion, &eglMinorVersion);
		eglGetConfigs(eglDisplay, NULL, 0, &Configs);
		Success = eglChooseConfig(eglDisplay, ConfigAttribs, &eglConfig, 1, &Configs);
		eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, EGLNativeWindowType(hwnd), NULL);
		eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, ContextAttribs);
		eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
		bool b = eglSwapInterval(eglDisplay, 0);
}

#endif