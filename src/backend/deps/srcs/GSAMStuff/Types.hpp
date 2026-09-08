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

// Logging

#define GSAM_LOG_INFO(msg) std::cout << "\n[" << GSAM_FUNC_NAME << "] INFO: " << msg << "\n"

#ifndef NDEBUG
    #define GSAM_LOG_DEBUG(msg) \
        std::cout << "\n[" << GSAM_FUNC_NAME << "] DEBUG: " << msg << "\n"
#else
    #define GSAM_LOG_DEBUG(msg) \
        do {} while(0)
#endif

#define GSAM_THROW_ERROR(msg) throw std::runtime_error(std::string("[") + GSAM_FUNC_NAME +  std::string("] ERROR: ") + std::string(msg))
#define GSAM_LOG_ERROR(msg) std::cout << "\n[" << GSAM_FUNC_NAME << "] ERROR: " << msg << "\n"

#if defined(__GNUC__) || defined(__clang__)
    #define GSAM_FUNC_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
    #define GSAM_FUNC_NAME __FUNCSIG__
#else
    #define GSAM_FUNC_NAME __func__ // Fallback
#endif

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};