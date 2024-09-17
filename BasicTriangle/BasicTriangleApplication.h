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

struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription()
    {
        return {
            0,
            sizeof(Vertex),
            vk::VertexInputRate::eVertex
        };
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

        return  {
            vk::VertexInputAttributeDescription {
                0,
                0,
                vk::Format::eR32G32Sfloat,
                offsetof(Vertex, position)
            },
            vk::VertexInputAttributeDescription {
                1,
                0,
                vk::Format::eR32G32B32Sfloat,
                offsetof(Vertex, color)
            }
        };
    }
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

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
    void createSwapChain(bool recreate = false);
    void createImageViews();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createRenderPass();
    void createFrameBuffers();
    void createCommandPool();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::
                      DeviceMemory& bufferMemory) const;
    void copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) const;
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;
    void createCommandBuffer();
    void createSyncObjects();
    void recordCommandBuffer(vk::CommandBuffer buffer, uint32_t imageIndex);
    void mainLoop();
    void drawFrame();
    void updateUniformBuffer(size_t currentFrame);
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
    vk::DescriptorSetLayout m_descriptorSetLayout;
    vk::PipelineLayout m_pipelineLayout;
    vk::Pipeline m_pipeline;
    vk::CommandPool m_commandPool;
    std::vector<vk::CommandBuffer> m_commandBuffer;
    vk::Buffer m_vertexBuffer;
    vk::DeviceMemory m_vertexBufferMemory;
    vk::Buffer m_indexBuffer;
    vk::DeviceMemory m_indexBufferMemory;

    std::vector<vk::Buffer> m_uniformBuffers;
    std::vector<vk::DeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;

    vk::DescriptorPool m_descriptorPool;
    std::vector<vk::DescriptorSet> m_descriptorSets;

    std::vector<vk::Semaphore> m_imageAvailable;
    std::vector<vk::Semaphore> m_renderFinished;
    std::vector<vk::Fence> m_inFlight;

    bool m_frameBufferResized = false;

    const std::vector<Vertex> m_vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint16_t> m_indices = {
        0, 1, 2, 2, 3, 0
    };

};