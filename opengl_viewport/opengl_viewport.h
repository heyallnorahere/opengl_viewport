#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#ifdef OPENGL_VIEWPORT_LIB_BUILD
#define OPENGL_VIEWPORT_DLL_ENTRY __declspec(dllexport)
#else
#define OPENGL_VIEWPORT_DLL_ENTRY __declspec(dllimport)
#endif
class opengl_viewport {
public:
	struct viewport_attribs {
		HWND parent;
		int x, y, width, height, major_version, minor_version;
	};
	OPENGL_VIEWPORT_DLL_ENTRY opengl_viewport(const viewport_attribs& attribs);
	OPENGL_VIEWPORT_DLL_ENTRY void clear();
	OPENGL_VIEWPORT_DLL_ENTRY void swap_buffers();
	OPENGL_VIEWPORT_DLL_ENTRY ~opengl_viewport();
	OPENGL_VIEWPORT_DLL_ENTRY static void use(opengl_viewport* viewport);
	OPENGL_VIEWPORT_DLL_ENTRY static LRESULT __stdcall window_proc(HWND window, unsigned int msg, WPARAM w_param, LPARAM l_param);
private:
	HDC dc;
	HWND window;
	HGLRC context;
};
#define PREVIOUS_VIEWPORT ((opengl_viewport*)1)
