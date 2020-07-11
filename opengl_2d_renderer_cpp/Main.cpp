#define GL_EXT_PROTOTYPES
#define WGL_EXT_PROTOTYPES
#include <Windows.h>
#include <GL/GL.h>
#include <GL/glext.h>
#define WGL_EXT_PROTOTYPES
#include <GL/wglext.h>
#pragma comment(lib, "opengl32.lib")

class Window {
public:
	struct Config {
		int width;
		int height;
		int posX;
		int posY;
		bool windowed;
	} config;

	LPTSTR windowClass;	// Window Class; Long Pointer to a Tchar STRing
	HGLRC RC;			// Rendering Context
	HDC DC;				// Device Context
	HWND WND;			// Window
	DWORD style;		// Double Word

	Window();
	~Window();
	int create(HINSTANCE hInstance, int nCmdShow);
	ATOM registerClass(HINSTANCE hInstance);
	void render();
	void swapBuffers();
	void adjustSize();
	void center();
	void destroy();
	static LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
};

Window::Window() {
	config.width = 1024;
	config.height = 720;
	config.posX = CW_USEDEFAULT;
	config.posY = 0;
	config.windowed = true;
	style = WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
}

Window::~Window() {
}

int Window::create(HINSTANCE hInstance, int nCmdShow) {

	windowClass = MAKEINTATOM(registerClass(hInstance));
	if (windowClass == 0) {
		MessageBox(0, "RegisterClass() failed.", "Window::create", MB_ICONERROR);
		return 1;
	}

	// Temporary window
	HWND fakeWND = CreateWindow(
		windowClass, "Fake Window",		// Window, Class Title
		style,						// Style
		0, 0,						// X Position, Y Position
		1, 1,						// Width, Height
		NULL, NULL,					// Parent Window, Menu
		hInstance, NULL);			// Instance, Param
	HDC fakeDC = GetDC(fakeWND);	// Device Context

	PIXELFORMATDESCRIPTOR fakePFD;
	ZeroMemory(&fakePFD, sizeof(fakePFD));	// Fills fakePFD with 0s
	fakePFD.nSize = sizeof(fakePFD);
	fakePFD.nVersion = 1;
	fakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	fakePFD.iPixelType = PFD_TYPE_RGBA;
	fakePFD.cColorBits = 32;
	fakePFD.cAlphaBits = 8;
	fakePFD.cDepthBits = 24;

	int fakePFDID = ChoosePixelFormat(fakeDC, &fakePFD);
	if (fakePFDID == 0) {
		MessageBox(0, "ChoosePixelFormat() failed.", "Window::create", MB_ICONERROR);
		return 1;
	}
	if (SetPixelFormat(fakeDC, fakePFDID, &fakePFD) == false) {
		MessageBox(0, "SetPixelFormat() failed.", "Window::create", MB_ICONERROR);
		return 1;
	}

	HGLRC fakeRC = wglCreateContext(fakeDC);	// Rendering Context
	if (fakeRC == 0) {
		MessageBox(0, "wglCreateContext() failed.", "Window::create", MB_ICONERROR);
		return 1;
	}
	if (wglMakeCurrent(fakeDC, fakeRC) == false) {
		MessageBox(0, "wglMakeCurrent() failed.", "Window::create", MB_ICONERROR);
		return 1;
	}

	// Get pointers to functions 
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
	wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
	if (wglChoosePixelFormatARB == nullptr) {
		MessageBox(0, "wglGetProcAddress() failed.", "Window::create", MB_ICONERROR);
		return 1;
	}

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
	wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
	if (wglCreateContextAttribsARB == nullptr) {
		MessageBox(0, "wglGetProcAddress() failed.", "Window::create", MB_ICONERROR);
		return 1;
	}

	if (config.windowed == true) {
		adjustSize();
		center();
	}

	// Do not create new windows or contexts because most variables in the Window object are still uninitialized
	WND = CreateWindow(
		windowClass, "OpenGL Window",		// Class Name, Window Name
		style,								// Style
		config.posX, config.posY,			// X Position, Y Position
		config.width, config.height,		// Width, Height
		NULL, NULL,							// Parent Window, Menu
		hInstance, NULL);					// Instance, Param

	DC = GetDC(WND);

	const int pixelAttribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
		WGL_SAMPLES_ARB, 4,
		0 };

	int pixelFormatID;
	UINT numFormats;
	const bool status = wglChoosePixelFormatARB(DC, pixelAttribs, NULL, 1, &pixelFormatID, &numFormats);

	if (status == false || numFormats == 0) {
		MessageBox(0, "wglChoosePixelFormatARB() failed.", "Window::create", MB_ICONERROR);
		return 1;
	}

	PIXELFORMATDESCRIPTOR PFD;
	DescribePixelFormat(DC, pixelFormatID, sizeof(PFD), &PFD);
	SetPixelFormat(DC, pixelFormatID, &PFD);

	const int major_min = 4, minor_min = 5;
	const int contextAttribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0 };

	RC = wglCreateContextAttribsARB(DC, 0, contextAttribs);
	if (RC == NULL) {
		MessageBox(0, "wglCreateContextAttribs() failed.", "Window::create", MB_ICONERROR);
		return 1;
	}

	// Delete temporary context and window
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(fakeRC);
	ReleaseDC(fakeWND, fakeDC);
	DestroyWindow(fakeWND);
	if (!wglMakeCurrent(DC, RC)) {
		MessageBox(0, "wglMakeCurrent() failed.", "Window::create", MB_ICONERROR);
		return 1;
	}

	// Initialize OpenGl loader in an extremely safe way
	SetWindowText(WND, (LPCSTR)glGetString(GL_VERSION));
	ShowWindow(WND, nCmdShow);

	return 0;
}

ATOM Window::registerClass(HINSTANCE hInstance) {
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);

	// CS_HREDRAW and CS_VREDRAW will redraw the window if it is moved or resized, CS_OWNDC will get a unique Device Context for each window in the class
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = WindowProcedure;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = "Core";

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK Window::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;	// Message handled
}

void Window::render() {

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_TRIANGLES);
	glVertex2f(-0.5f, -0.5f);
	glVertex2f(0.0f, 0.05f);
	glVertex2f(0.5f, 0.5f);
	glEnd();
}

void Window::swapBuffers() {
	SwapBuffers(DC);
}

void Window::adjustSize() {
	RECT rect = { 0, 0, config.width, config.height };
	AdjustWindowRect(&rect, style, false);
	config.width = rect.right - rect.left;
	config.height = rect.bottom - rect.top;
}

void Window::center() {
	RECT primaryDisplaySize;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &primaryDisplaySize, 0);
	config.posX = (primaryDisplaySize.right - config.width) / 2;
	config.posY = (primaryDisplaySize.bottom - config.height) / 2;
}

void Window::destroy() {
	wglMakeCurrent(NULL, NULL);
	if (RC) {
		wglDeleteContext(RC);
	}
	if (DC) {
		ReleaseDC(WND, DC);
	}
	if (WND) {
		DestroyWindow(WND);
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	Window window;

	ReleaseDC(window.WND, window.DC);
	
	if (window.create(hInstance, nCmdShow) != 0) {
		PostQuitMessage(1);
	}

	MSG msg;
	bool active = true;
	while (active) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				active = false;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		window.render();
		window.swapBuffers();
	}
	window.destroy();
	return msg.wParam;
}