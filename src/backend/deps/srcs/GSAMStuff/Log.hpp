#pragma once

#include <iostream>
#include <stdexcept>

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