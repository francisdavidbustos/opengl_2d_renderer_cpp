#include <Windows.h>
#include <GL/GL.h>
#include "glext.h"
#include "wglext.h"
#pragma comment(lib, "opengl32.lib")

void render() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

int main() {

	HWND fakeWND = CreateWindow(				// Handle of Window
		"Core", "Fake Window",					// Window class, Title
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,		// Style
		0, 0,									// Window position
		1, 1,									// Window size
		NULL, NULL,								// Parent window, Menu
		NULL, NULL);						// Instance, param
	HDC fakeDC = GetDC(fakeWND);				// Device context (Handle of Context Device)

	PIXELFORMATDESCRIPTOR fakePFD;
	ZeroMemory(&fakePFD, sizeof(fakePFD));
	fakePFD.nSize = sizeof(fakePFD);
	fakePFD.nVersion = 1;
	fakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	fakePFD.iPixelType = PFD_TYPE_RGBA;
	fakePFD.cColorBits = 32;
	fakePFD.cAlphaBits = 8;
	fakePFD.cDepthBits = 24;

	int fakePFDID = ChoosePixelFormat(fakeDC, &fakePFD);
	if (fakePFDID == 0) {
		return 1;
	}

	if (SetPixelFormat(fakeDC, fakePFDID, &fakePFD) == false) {
		return 2;
	}

	HGLRC fakeRC = wglCreateContext(fakeDC);	// Rendering context

	if (fakeRC == 0) {
		return 3;
	}

	if (wglMakeCurrent(fakeDC, fakeRC) == false) {
		return 4;
	}

	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
	wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
	if (wglChoosePixelFormatARB == nullptr) {
		return 5;
	}

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
	wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
	if (wglCreateContextAttribsARB == nullptr) {
		return 6;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	SwapBuffers(fakeDC);

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(fakeRC);
	ReleaseDC(fakeWND, fakeDC);
	DestroyWindow(fakeWND);
	if (!wglMakeCurrent(fakeDC, fakeRC)) {
		return 7;
	}

	SetWindowText(fakeWND, reinterpret_cast<LPCSTR>(glGetString(GL_VERSION)));
	ShowWindow(fakeWND, 0);

	return 0;
}