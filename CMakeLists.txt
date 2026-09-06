cmake_minimum_required (VERSION 3.15)
project ("HelloVulkan")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)


  
file (GLOB_RECURSE CXX_SRC "src/*.cpp" "src/*.c")

set_source_files_properties(
    src/backend/deps/srcs/vma/vma_impl.cpp
    PROPERTIES
    COMPILE_OPTIONS "-Wno-nullability-completeness"
)

add_executable(HelloVulkan ${CXX_SRC})

set_property(TARGET HelloVulkan PROPERTY CXX_STANDARD 17)
set_property(TARGET HelloVulkan PROPERTY CXX_STANDARD_REQUIRED ON)

set(ASAN OFF)

if (ASAN)
target_compile_options(HelloVulkan PRIVATE /fsanitize=address)
target_link_options(HelloVulkan PRIVATE /fsanitize=address)
endif()  

target_include_directories (HelloVulkan PRIVATE libs)
target_include_directories(HelloVulkan PRIVATE src)
  
target_link_libraries(HelloVulkan PRIVATE
	Vulkan::Vulkan
)