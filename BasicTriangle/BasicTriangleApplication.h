#pragma once
#include "VulkanHelpers/PhysicalDeviceHelpers.h"

constexpr int32_t Width = 800;
constexpr int32_t Height = 600;

const std::vector ValidationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector DeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
constexpr bool EnableValidationLayers = false;
#else
constexpr bool EnableValidationLayers = true;
#endif

class BasicTriangleApplication
{
public:
    BasicTriangleApplication()
        : m_window(nullptr)
    {
    }
    void run();
private:
    void initWindow();
    void initVulcan();
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void mainLoop();
    void cleanup();

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT vkMessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT vkMessageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );

    GLFWwindow* m_window;
    vk::Instance m_instance;
    vk::DebugUtilsMessengerEXT m_debugMessenger;
    vk::SurfaceKHR m_surface;
    PhysicalDevice m_physicalDevice;
    vk::Device m_logicalDevice;
    vk::Queue m_gfxQueue;
    vk::Queue m_presentQueue;
    vk::SwapchainKHR m_swapChain;
    vk::Format m_swapChainImageFormat;
    vk::Extent2D m_swapChainExtent;
    std::vector<vk::Image> m_swapChainImages;
    std::vector<vk::ImageView> m_swapChainImageViews;
};