#pragma once

inline PFN_vkCreateDebugUtilsMessengerEXT  FnVkCreateDebugUtilsMessengerEXT = nullptr;
inline PFN_vkDestroyDebugUtilsMessengerEXT FnVkDestroyDebugUtilsMessengerEXT = nullptr;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkDebugUtilsMessengerEXT* pMessenger);

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const* pAllocator);

bool LoadDebugUtilsMessengerExtFunctions(vk::Instance const& instance);