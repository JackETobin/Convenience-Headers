#ifndef VULKAN_CORE_H
#define VULKAN_CORE_H

#include "../platform.h"
#include "../memory.h"

#if defined(PLATFORM_WINDOWS)
    #define VK_USE_PLATFORM_WIN32_KHR
#endif // PLATFORM_WINDOWS

#pragma comment( lib, "vulkan-1" )
#include <vulkan/vulkan.h>

#endif // VULKAN_CORE_H