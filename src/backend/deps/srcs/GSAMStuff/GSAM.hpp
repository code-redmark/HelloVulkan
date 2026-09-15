#pragma once

#include "ResourceRegistry.hpp"
#include "TypedResourceRegistry.hpp"
#include "Log.hpp"

#include <cstdint>
#include <cstddef>
#include <glm/glm.hpp>
#include <memory>
#include <utility>
#include <array>
#include <filesystem>

namespace GSAM
{
    /*
        GLAD library for OpenGL requires a GLADloadproc,
        you can get this from your Window Management library
        with functions like glfwGetProcAddress() (with GLFW) or
        SDL_GL_GetProcAddress (with SDL)

        This is needed so that glad can directly find the OpenGL
        function (proc) you are calling
    */
    using GS_GLADloadproc = void *(*)(const char *name);

    enum class BackendAPI
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

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    class TextureImplementation
    {
        public:

        ~TextureImplementation() = default;
    };

    struct GSTexture
    {
        glm::vec2 size;
        std::unique_ptr<TextureImplementation> impl;
    };


    class MeshImplementation
    {
        public:

        ~MeshImplementation() = default;
    };
    struct GSMesh
    {
        public:

        const std::filesystem::path& resource_path; 
        std::unique_ptr<MeshImplementation> impl;
    };

    namespace Vulkan
    {
        enum class QueueFamilyCapability
        {
            Graphics = 0,
            Presentation,
            
            Count
        };

        constexpr inline size_t capability_count() { return static_cast<size_t>(QueueFamilyCapability::Count); }

        class ApplicationRequirements
        {
        private:
            // requires, amount of queues required
        std::array<std::pair<bool, int>, capability_count()> requirements;

        public:
            bool requires(QueueFamilyCapability capability) const;
            int queue_requirement(QueueFamilyCapability capability) const;
            void set_requirement(QueueFamilyCapability capability, bool value, int queue_requirement);
        };

        template <typename E>
        constexpr inline int enum_index(E Enum) { return static_cast<int>(Enum); }

        template <typename E>
        constexpr inline E index_enum(int index) { return static_cast<E>(index); }
    }

}