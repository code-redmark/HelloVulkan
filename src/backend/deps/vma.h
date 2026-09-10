#pragma once

#ifdef __clang__

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"

#include "srcs/vma/vk_mem_alloc.h"

#pragma clang diagnostic pop

#else

#include "srcs/vma/vk_mem_alloc.h"

#endif
