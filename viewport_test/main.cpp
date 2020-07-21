#include <GL/glew.h>
#include <Windows.h>
#include <opengl_viewport.h>
#include "resource.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#define RENDER 101
std::string read_file(const std::string& path) {
	std::ifstream file(path);
	std::stringstream contents;
	std::string line;
	while (std::getline(file, line)) contents << line << '\n';
	file.close();
	return contents.str();
}
static unsigned int create_shader(const std::string& path, unsigned int type) {
	std::string source = read_file(path);
	const char* src = source.c_str();
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char info_log[512];
		glGetShaderInfoLog(shader, 512, NULL, info_log);
		std::string msg = "error compiling shader: " + path + ": " + info_log + "\n";
		OutputDebugStringA(msg.c_str());
	}
	return shader;
}
LRESULT __stdcall wndproc(HWND window, unsigned int msg, WPARAM w_param, LPARAM l_param) {
	static opengl_viewport* vp;
	static unsigned int vao, vbo, shader;
	switch (msg) {
	case WM_CREATE:
	{
		CREATESTRUCT* cs = (CREATESTRUCT*)l_param;
		opengl_viewport::viewport_attribs vpa;
		ZeroMemory(&vpa, sizeof(opengl_viewport::viewport_attribs));
		vpa.parent = window;
		vpa.width = cs->cx;
		vpa.height = cs->cy;
		vpa.major_version = 4;
		vpa.minor_version = 6;
		vp = new opengl_viewport(vpa);
		struct vertex {
			glm::vec3 pos, color;
		};
		std::vector<vertex> vertices = {
			{ glm::vec3(-0.5f, -0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f) },
			{ glm::vec3( 0.5f, -0.5f, 0.f), glm::vec3(0.f, 1.f, 0.f) },
			{ glm::vec3( 0.0f,  0.5f, 0.f), glm::vec3(1.f, 0.f, 0.f) },
		};
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(vertex), (void*)offsetof(vertex, pos));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(vertex), (void*)offsetof(vertex, color));
		glEnableVertexAttribArray(1);
		glBindVertexArray(NULL);
		shader = glCreateProgram();
		unsigned int vs = create_shader("vertex.shader", GL_VERTEX_SHADER);
		unsigned int fs = create_shader("fragment.shader", GL_FRAGMENT_SHADER);
		glAttachShader(shader, vs);
		glAttachShader(shader, fs);
		glLinkProgram(shader);
		int success;
		glGetProgramiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char info_log[512];
			glGetProgramInfoLog(shader, 512, NULL, info_log);
			std::string msg = "error compiling program: " + std::string(info_log) + "\n";
			OutputDebugStringA(msg.c_str());
		}
		glDeleteShader(vs);
		glDeleteShader(fs);
		SetTimer(window, RENDER, 1000.0 / 60.0, NULL);
	}
		break;
	case WM_DESTROY:
		KillTimer(window, RENDER);
		glDeleteProgram(shader);
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
		delete vp;
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		switch (w_param) {
		case RENDER:
			vp->clear();
			glUseProgram(shader);
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			glBindVertexArray(NULL);
			glUseProgram(NULL);
			vp->swap_buffers();
			break;
		}
		break;
	case WM_COMMAND:
		switch (w_param) {
		case ID_FILE_EXIT:
			DestroyWindow(window);
			break;
		}
	default:
		return DefWindowProc(window, msg, w_param, l_param);
	}
	return 0;
}
int __stdcall WinMain(HINSTANCE instance, HINSTANCE previous, char* cmd_line, int cmd_show) {
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(WNDCLASS));
	wc.lpszClassName = TEXT("viewport_test");
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.lpfnWndProc = wndproc;
	wc.hInstance = instance;
	wc.hCursor = LoadCursor(instance, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	if (!RegisterClass(&wc)) return -1;
	CreateWindow(TEXT("viewport_test"), TEXT("opengl viewport test app"), WS_VISIBLE | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, instance, NULL);
	MSG msg;
	while (GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}