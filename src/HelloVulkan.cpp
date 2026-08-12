#define RGFW_IMPLEMENTATION
#define RGFW_PRINT_ERRORS
#include <RGFW.h>

#include "Umbrella.h"

#include <iostream>

static RGFW_window* RGFWSetup()
{
	RGFW_window* win = RGFW_createWindow("Hello Vulkan", 0, 0, 800, 670, RGFW_windowAllowDND | RGFW_windowCenter | RGFW_windowScaleToMonitor);
	if (win) return win;

	std::cerr << "Couldn't make RGFW window\n";
	return nullptr;
}

int main(void)
{  
	RGFW_window* window = RGFWSetup();
	VulkanContext context;

	FamilyQueueRequirements requirements;
	requirements.set_requirement(FamilyCapability::Graphics, true, 1);
	requirements.set_requirement(FamilyCapability::Presentation, true, 1);

	int w, h;
	RGFW_window_getSize(window, &w, &h);
	std::cout << "size of frame buffer:\n width: " << w << "\nheight: " << h << "\n";

	context.Init(RGFW_window_getHWND(window), requirements);
	


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
