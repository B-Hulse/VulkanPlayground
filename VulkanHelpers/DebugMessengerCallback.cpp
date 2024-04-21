#include "pch.h"
#include "DebugMessengerCallback.h"

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger)
{
    if (!FnVkCreateDebugUtilsMessengerEXT)
    {
        throw std::runtime_error("vkCreateDebugUtilsMessengerEXT was called before it was loaded");
    }

    return FnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const* pAllocator)
{
    if (!FnVkDestroyDebugUtilsMessengerEXT)
    {
        throw std::runtime_error("vkDestroyDebugUtilsMessengerEXT was called before it was loaded");
    }

    return FnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

bool LoadDebugUtilsMessengerExtFunctions(vk::Instance const& instance)
{
    FnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
    FnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));

    return FnVkCreateDebugUtilsMessengerEXT && FnVkDestroyDebugUtilsMessengerEXT;
}