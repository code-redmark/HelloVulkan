#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#include "Umbrella.h"

#include <iostream>
#include <ctime>

static GLFWwindow* GLFWSetup()
{
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
	int init = glfwInit();
	if (!init)
	{
		glfwTerminate();
		GSAM_THROW_ERROR("GLFW Initialization failed");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* win = glfwCreateWindow(800, 600, "Hello Vulkan", NULL, NULL);
	if (win == NULL) 
		{
			GSAM_THROW_ERROR("glfwCreateWindow failed");
		}
	return win;
}

int main(void)
{  
	GLFWwindow* window = GLFWSetup();
	if (!window) 
	{
		glfwTerminate();
		GSAM_THROW_ERROR("GLFW setup failed");
	}


	
	Display* dpy = glfwGetX11Display();
	if (dpy == NULL) GSAM_THROW_ERROR("Couldn't get X11 display");

	Window win = glfwGetX11Window(window);
	if (win == None) GSAM_THROW_ERROR("Couldn't get X11 window handle");

	GSAM::Vulkan::ApplicationRequirements requirements;
	requirements.set_requirement(GSAM::Vulkan::QueueFamilyCapability::Graphics, true, 1);
	requirements.set_requirement(GSAM::Vulkan::QueueFamilyCapability::Presentation, true, 1);

	VulkanBackend::Context context(dpy, win, requirements);	

	context.CreateMesh("assets/tree.obj");

	VulkanBackend::ImageData imgData = VulkanBackend::load_image("assets/cobblestone.png");
	context.CreateTexture(imgData);

	while (!glfwWindowShouldClose(window))
	{	
		glfwPollEvents();
	}

	context.shutdown();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
