#pragma once

#include <cstdint>
#include <cstddef>
#include <glm/glm.hpp>

/*
	GLAD library for OpenGL requires a GLADloadproc,
	you can get this from your Window Management library
	with functions like glfwGetProcAddress() (with GLFW) or
	SDL_GL_GetProcAddress (with SDL)

	This is needed so that glad can directly find the OpenGL
	function (Procedure) you are calling
*/
using GS_GLADloadproc = void *(*)(const char *name);

enum class GSContextAPI
{
	OpenGL46
};

enum class BufferUsageHint
{
    Static,
    Dynamic,
    Stream
};

struct BufferData
{   
    const void* raw;
    std::size_t size;
    BufferUsageHint usage_hint;
};

struct ResourceHandle
{
	uint32_t generation;
	uint64_t id;
};



struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};