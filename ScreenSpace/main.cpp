#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <array>
#include <cassert>
#include <sstream>
#include <numeric>

#include "SSRayMarching.h"

// Vulkan���C�u�����̃����N
#pragma comment(lib, "vulkan-1.lib")

const int WindowWidth = 1280;
const int WindowHeight = 1024;

const char* AppTitle = "ScreenSpace - RayMarching";

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, 0);
	auto window = glfwCreateWindow(WindowWidth, WindowHeight, AppTitle, nullptr, nullptr);

	//::AllocConsole();
	//FILE* fp;
	//freopen_s(&fp, "CONOUT$", "w", stdout);
	//freopen_s(&fp, "CONIN$", "r", stdin);

	// Vulkan ������
	SSRayMarching theApp;
	theApp.initialize(window, AppTitle);

	while (glfwWindowShouldClose(window) == GLFW_FALSE)
	{
		glfwPollEvents();
		theApp.render();
	}

	// Vulkan �I��
	theApp.terminate();
	glfwTerminate();

	//::FreeConsole();

	return 0;
}