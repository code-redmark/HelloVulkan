#define RGFW_IMPLEMENTATION
#define RGFW_PRINT_ERRORS
#include <RGFW.h>

#include "Umbrella.h"

#include <iostream>
#include <ctime>

static RGFW_window* RGFWSetup()
{
	RGFW_window* win = RGFW_createWindow("Hello Vulkan", 0, 0, 800, 670, RGFW_windowAllowDND | RGFW_windowCenter | RGFW_windowScaleToMonitor);
	if (win) return win;

	std::cerr << "Couldn't make RGFW window\n";
	return nullptr;
}

int main(void)
{  
	std::cout << __cplusplus << '\n';
	std::cout << "Hello World!\n";
	std::cout << "sizeof(VkGPuMesh): " << sizeof(VulkanBackend::GpuMesh) << "\n";

	RGFW_window* window = RGFWSetup();
	
	GSAM::Vulkan::ApplicationRequirements requirements;
	requirements.set_requirement(GSAM::Vulkan::QueueFamilyCapability::Graphics, true, 1);
	requirements.set_requirement(GSAM::Vulkan::QueueFamilyCapability::Presentation, true, 1);
	
	VulkanBackend::Context context(RGFW_window_getHWND(window), requirements);	

	context.CreateMesh("assets/tree.obj");
	
	while (!RGFW_window_shouldClose(window))
	{
		RGFW_event event;
		while (RGFW_window_checkEvent(window, &event))
		{
		}
	}
	RGFW_window_close(window); 

	context.shutdown();

	return 0;
}
