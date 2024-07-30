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
    BasicTriangleApplication(size_t maxFramesInFlight)
        : m_window(nullptr), m_maxFramesInFlight(maxFramesInFlight), m_currentFrame(0)
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
    void createGraphicsPipeline();
    void createRenderPass();
    void createFrameBuffers();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();
    void recordCommandBuffer(vk::CommandBuffer buffer, uint32_t imageIndex);
    void mainLoop();
    void drawFrame();
    void cleanupSwapChain();
    void recreateSwapChain();
    void cleanup();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT vkMessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT vkMessageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );

    GLFWwindow* m_window;

    size_t m_maxFramesInFlight;
    size_t m_currentFrame;

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
    std::vector<vk::Framebuffer> m_swapChainFrameBuffers;
    vk::RenderPass m_renderPass;
    vk::PipelineLayout m_pipelineLayout;
    vk::Pipeline m_pipeline;
    vk::CommandPool m_commandPool;
    std::vector<vk::CommandBuffer> m_commandBuffer;

    std::vector<vk::Semaphore> m_imageAvailable;
    std::vector<vk::Semaphore> m_renderFinished;
    std::vector<vk::Fence> m_inFlight;

    bool m_frameBufferResized = false;
};