#include <GL/glew.h>
#include <GL/wglew.h>
#include "opengl_viewport.h"
#define CLASS_NAME "opengl_viewport"
#include <map>
#include <string>
int __stdcall DllMain(HINSTANCE instance, unsigned long reason, void* reserved) {
	switch (reason) {
	case DLL_PROCESS_ATTACH:
	{
		WNDCLASSA wc;
		ZeroMemory(&wc, sizeof(WNDCLASSA));
		wc.lpszClassName = CLASS_NAME;
		wc.lpfnWndProc = opengl_viewport::window_proc;
		wc.style = CS_VREDRAW | CS_HREDRAW;
		wc.hInstance = instance;
		wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		return !!RegisterClassA(&wc);
	}
		break;
	case DLL_PROCESS_DETACH:
		return UnregisterClassA(CLASS_NAME, instance);
		break;
	}
	return true;
}
struct context_attribs {
	opengl_viewport* owner;
	int major_version, minor_version;
};
opengl_viewport::opengl_viewport(const viewport_attribs& attribs) {
	context_attribs* ca = new context_attribs;
	ca->owner = this;
	ca->major_version = attribs.major_version;
	ca->minor_version = attribs.minor_version;
	CreateWindowA(CLASS_NAME, NULL, WS_VISIBLE | WS_CHILDWINDOW, attribs.x, attribs.y, attribs.width, attribs.height, attribs.parent, NULL, NULL, ca);
}
void opengl_viewport::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
void opengl_viewport::swap_buffers() {
	SwapBuffers(this->dc);
}
opengl_viewport::~opengl_viewport() {
	DestroyWindow(this->window);
}
opengl_viewport* previous = NULL;
opengl_viewport* current = NULL;
void opengl_viewport::use(opengl_viewport* viewport) {
	switch ((size_t)viewport) {
	case NULL:
		wglMakeCurrent(NULL, NULL);
		previous = current;
		current = viewport;
		break;
	case (size_t)PREVIOUS_VIEWPORT:
	{
		opengl_viewport* temp = previous;
		previous = current;
		current = temp;
		if (!current) return;
		wglMakeCurrent(current->dc, current->context);
	}
		break;
	default:
		wglMakeCurrent(viewport->dc, viewport->context);
		previous = current;
		current = viewport;
		break;
	}
}
LRESULT __stdcall opengl_viewport::window_proc(HWND window, unsigned int msg, WPARAM w_param, LPARAM l_param) {
	static std::map<HWND, opengl_viewport*> viewports;
	switch (msg) {
	case WM_CREATE:
	{
		CREATESTRUCT cs = *(CREATESTRUCT*)l_param;
		context_attribs* attr = (context_attribs*)cs.lpCreateParams;
		opengl_viewport* me = attr->owner;
		me->window = window;
		me->dc = GetDC(me->window);
		HDC& dc = me->dc;
		PIXELFORMATDESCRIPTOR pfd;
		ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 32;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int fmt = ChoosePixelFormat(dc, &pfd);
		if (!fmt)
			goto error;
		bool succeeded = SetPixelFormat(dc, fmt, &pfd);
		if (!succeeded)
			goto error;
		HGLRC temp = wglCreateContext(dc);
		wglMakeCurrent(dc, temp);
		unsigned int glew_error = glewInit();
		if (glew_error != GLEW_OK)
			goto error;
		int attribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, attr->major_version,
			WGL_CONTEXT_MINOR_VERSION_ARB, attr->minor_version,
			WGL_CONTEXT_FLAGS_ARB, NULL,
			NULL
		};
		if (wglewIsSupported("WGL_ARB_create_context")) {
			me->context = wglCreateContextAttribsARB(dc, NULL, attribs);
			if (!me->context)
				goto error;
			wglDeleteContext(temp);
			wglMakeCurrent(dc, me->context);
		}
		else {
			MessageBoxA(cs.hwndParent, "could not get 3.x context, instead created 2.1 context", "opengl viewport lib", NULL);
			me->context = temp;
		}
		std::string msg = "[opengl viewport lib] successfully created context with version " + std::string((char*)glGetString(GL_VERSION)) + "\n";
		OutputDebugStringA(msg.c_str());
		viewports.insert(std::pair<HWND, opengl_viewport*>(window, me));
		delete attr;
		break;
	}
	{
	error:
		char error_str[256];
		unsigned long last_error = GetLastError();
		ultoa(last_error, error_str, 16);
		OutputDebugString(TEXT("[opengl viewport library] error creating gl context: 0x"));
		OutputDebugStringA(error_str);
		OutputDebugString(TEXT("\n"));
		DestroyWindow(window);
		return last_error;
		break;
	}
	case WM_DESTROY:
		if (viewports[window])
			wglDeleteContext(viewports[window]->context);
		break;
	default:
		return DefWindowProc(window, msg, w_param, l_param);
	}
	return 0;
}
