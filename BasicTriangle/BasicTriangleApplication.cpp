#include "pch.h"
#include "BasicTriangleApplication.h"

#include "VulkanHelpers/ExtensionHelpers.h"
#include "VulkanHelpers/ValidationLayerHelpers.h"
#include "VulkanHelpers/DebugMessengerCallback.h"
#include "VulkanHelpers/PhysicalDeviceHelpers.h"
#include "VulkanHelpers/ShaderHelpers.h"

namespace
{
    vk::DebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo(
        PFN_vkDebugUtilsMessengerCallbackEXT pDebugCallback
    )
    {
        return {
            {},
            vk::DebugUtilsMessageSeverityFlagsEXT(
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
            ),
            vk::DebugUtilsMessageTypeFlagsEXT(
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
            ),
            pDebugCallback
        };
    }

    bool IsDeviceSuitable(PhysicalDevice& physicalDevice, vk::SurfaceKHR const& surface)
    {
        auto const device = physicalDevice.GetPDevice();

        auto const deviceFeatures = device.getFeatures();

        if (!deviceFeatures.geometryShader)
        {
            return false;
        }

        auto const queueFamilyIndices = physicalDevice.GetQueueFamilyIndices(surface);
        if (!queueFamilyIndices.isComplete())
        {
            return false;
        }

        auto const deviceExtensionSupport = device.enumerateDeviceExtensionProperties();

        std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

        for (auto const& supportedExtension : deviceExtensionSupport)
        {
            requiredExtensions.erase(supportedExtension.extensionName);
        }

        if (!requiredExtensions.empty())
        {
            return false;
        }

        auto const swapChainSupport = physicalDevice.GetSwapChainSupport(surface);
        if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
        {
            return false;
        }

        return true;
    }

    uint32_t ScorePhysicalDevice(PhysicalDevice& physicalDevice, vk::SurfaceKHR const& surface)
    {
        auto const deviceProperties = physicalDevice.GetPDevice().getProperties();

        uint32_t score = 1;

        if (!IsDeviceSuitable(physicalDevice, surface))
        {
            return 0;
        }

        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        {
            score += 1;
        }

        return score;
    }

    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace ==
                vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    vk::PresentModeKHR ChoosePresentMode(std::vector<vk::PresentModeKHR> const& availableModes)
    {
        std::vector preferredModeOrder = {
            vk::PresentModeKHR::eImmediate,
            vk::PresentModeKHR::eMailbox,
            vk::PresentModeKHR::eFifo
        };

        if (auto const found = std::ranges::find_first_of(preferredModeOrder, availableModes); found !=
            preferredModeOrder.end())
        {
            return *found;
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities, GLFWwindow* const pWindow)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        uint32_t width, height;
        glfwGetWindowSize(pWindow, reinterpret_cast<int*>(&width), reinterpret_cast<int*>(&height));

        return {
            std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
    }
}

void BasicTriangleApplication::run()
{
    initWindow();
    initVulcan();
    mainLoop();
    cleanup();
}

void BasicTriangleApplication::initWindow()
{
    glfwInit();
    // Disables OpenGL context creation
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

void BasicTriangleApplication::initVulcan()
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandPool();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffer();
    createSyncObjects();
}

void BasicTriangleApplication::createInstance()
{
    if (EnableValidationLayers && !AreValidationLayersSupported(ValidationLayers))
    {
        throw std::runtime_error("Not all required validation layers are supported");
    }

    auto requiredExtensions = GetRequiredExtensions(true);

    if (!AreRequiredExtensionsSupported(requiredExtensions))
    {
        throw std::runtime_error("Not all required extensions are supported");
    }

    std::vector<const char*> validationLayers;
    const void* pNext = nullptr;
    auto const debugMessageCreateInfo = GetDebugMessengerCreateInfo(&debugCallback);

    if (EnableValidationLayers)
    {
        validationLayers = ValidationLayers;
        pNext = &debugMessageCreateInfo;
    }

    constexpr vk::ApplicationInfo appInfo(
        "Basic Triangle",
        vk::makeApiVersion(0, 1, 0, 0),
        "No Engine",
        vk::makeApiVersion(0, 1, 0, 0),
        vk::ApiVersion10
    );

    vk::InstanceCreateInfo const instanceCreateInfo(
        {},
        &appInfo,
        validationLayers,
        requiredExtensions,
        pNext
    );

    auto const result = vk::createInstance(&instanceCreateInfo, nullptr, &m_instance);
    resultCheck(result, "Failure initializing the Vulkan instance");
}

void BasicTriangleApplication::setupDebugMessenger()
{
    if constexpr (!EnableValidationLayers)
    {
        return;
    }

    if (!LoadDebugUtilsMessengerExtFunctions(m_instance))
    {
        throw std::runtime_error("Failed to find required debug messenger functions");
    }

    m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(GetDebugMessengerCreateInfo(&debugCallback));
}

void BasicTriangleApplication::createSurface()
{
    if (auto const result = static_cast<vk::Result>(glfwCreateWindowSurface(
        m_instance,
        m_window,
        nullptr,
        reinterpret_cast<VkSurfaceKHR*>(&m_surface)
    ));
    result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Filed to create window surface, error = '" + to_string(result) + "'");
    }
}

void BasicTriangleApplication::pickPhysicalDevice()
{
    auto const bestDevice = FindBestPhysicalDevice(m_instance, m_surface, ScorePhysicalDevice);

    if (!bestDevice)
    {
        throw std::runtime_error("Failed to find suitable physical decive");
    }

    m_physicalDevice = *bestDevice;
}

void BasicTriangleApplication::createLogicalDevice()
{
    auto const& queueFamilyIndices = m_physicalDevice.GetQueueFamilyIndices(m_surface);

    std::set const uniqueQueueFamilyIndices = {
        *queueFamilyIndices.graphicsFamilyIndex,
        *queueFamilyIndices.presentFamilyIndex
    };

    std::vector queuePriority = { 1.0f };

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilyIndices.size());

    size_t i = 0;
    for (auto const& index : uniqueQueueFamilyIndices)
    {
        queueCreateInfos[i] = vk::DeviceQueueCreateInfo(
            {},
            index,
            queuePriority
        );
        i++;
    }

    std::vector<const char*> enabledLayerNames = {};

    if (EnableValidationLayers)
    {
        enabledLayerNames = ValidationLayers;
    }

    std::vector<const char*> enabledExtensionNames = DeviceExtensions;

    vk::PhysicalDeviceFeatures const deviceFeatures;

    m_logicalDevice = m_physicalDevice.GetPDevice().createDevice(
        vk::DeviceCreateInfo(
            {},
            queueCreateInfos,
            enabledLayerNames,
            enabledExtensionNames,
            &deviceFeatures
        )
    );

    m_gfxQueue = m_logicalDevice.getQueue(*queueFamilyIndices.graphicsFamilyIndex, 0);
    m_presentQueue = m_logicalDevice.getQueue(*queueFamilyIndices.presentFamilyIndex, 0);
}

void BasicTriangleApplication::createSwapChain()
{
    auto const swapChainSupport = m_physicalDevice.GetSwapChainSupport(m_surface);

    auto const surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    auto const presentMode = ChoosePresentMode(swapChainSupport.presentModes);
    auto const swapExtent = ChooseSwapExtent(swapChainSupport.capabilities, m_window);

    auto imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    auto const indices = m_physicalDevice.GetQueueFamilyIndices(m_surface);

    std::set queueFamilyIndicesSet = { *indices.graphicsFamilyIndex, *indices.presentFamilyIndex };
    std::vector queueFamilyIndices(queueFamilyIndicesSet.begin(), queueFamilyIndicesSet.end());

    bool const concurrent = queueFamilyIndices.size() > 1;

    vk::SwapchainCreateInfoKHR const swapchainCreateInfo(
        {},
        m_surface,
        imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        swapExtent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        concurrent ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        queueFamilyIndices,
        swapChainSupport.capabilities.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        presentMode,
        true
    );

    m_swapChain = m_logicalDevice.createSwapchainKHR(swapchainCreateInfo);

    m_swapChainImages = m_logicalDevice.getSwapchainImagesKHR(m_swapChain);
    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = swapExtent;
}

void BasicTriangleApplication::createImageViews()
{
    auto fnGetImageView = [this](vk::Image const& img)
        {
            vk::ImageViewCreateInfo const imageViewCreateInfo(
                {},
                img,
                vk::ImageViewType::e2D,
                m_swapChainImageFormat,
                vk::ComponentMapping(),
                vk::ImageSubresourceRange(
                    vk::ImageAspectFlagBits::eColor,
                    0,
                    1,
                    0,
                    1
                )
            );

            return m_logicalDevice.createImageView(imageViewCreateInfo);
        };

    std::ranges::transform(m_swapChainImages, std::back_inserter(m_swapChainImageViews), fnGetImageView);
}

void BasicTriangleApplication::createDescriptorSetLayout()
{
    std::vector bindings = {
        vk::DescriptorSetLayoutBinding
        {
            0,
            vk::DescriptorType::eUniformBuffer,
            1,
            vk::ShaderStageFlagBits::eVertex
        }
    };

    vk::DescriptorSetLayoutCreateInfo layoutInfo
    {
        {},
        bindings
    };

    m_descriptorSetLayout = m_logicalDevice.createDescriptorSetLayout(layoutInfo);
}

void BasicTriangleApplication::createGraphicsPipeline()
{
    auto const vertShaderModule = CreateShaderModule(m_logicalDevice, "shaders/shader.vert.spv");
    auto const fragShaderModule = CreateShaderModule(m_logicalDevice, "shaders/shader.frag.spv");

    std::array vertexBindingDescriptions = { Vertex::getBindingDescription() };
    std::array vertexAttributeDescriptions = { Vertex::getAttributeDescriptions() };

    vk::PipelineVertexInputStateCreateInfo vertexInputState
    {
        {},
        vertexBindingDescriptions,
        vertexAttributeDescriptions
    };

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        { {}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main" },
        { {}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main" }
    };

    std::vector dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{ {}, dynamicStates };

    constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{
        {},
        vk::PrimitiveTopology::eTriangleList,
        false
    };

    constexpr vk::PipelineViewportStateCreateInfo viewportState{ {}, 1, nullptr, 1, nullptr };

    constexpr vk::PipelineRasterizationStateCreateInfo rasterizationState{
        {},
        false,
        false,
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eCounterClockwise,
        false,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    constexpr vk::PipelineMultisampleStateCreateInfo multisampleState{};

    std::vector colorBlendAttachments = {
        vk::PipelineColorBlendAttachmentState {
            false,
            {},
            {},
            {},
            {},
            {},
            {},
            vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags
        }
    };

    vk::PipelineColorBlendStateCreateInfo const colorBlendState{
        {},
        false,
        vk::LogicOp::eCopy,
        colorBlendAttachments
    };

    std::vector layouts = { m_descriptorSetLayout };

    vk::PipelineLayoutCreateInfo pipelineLayout
    {
        {},
        layouts
    };

    m_pipelineLayout = m_logicalDevice.createPipelineLayout(pipelineLayout);

    vk::GraphicsPipelineCreateInfo const pipelineCreateInfo{
        {},
        shaderStages,
        &vertexInputState,
        &inputAssemblyState,
        {},
        &viewportState,
        &rasterizationState,
        &multisampleState,
        {},
        &colorBlendState,
        &dynamicStateCreateInfo,
        m_pipelineLayout,
        m_renderPass,
        0
    };

    auto pipelineResult = m_logicalDevice.createGraphicsPipeline(nullptr, pipelineCreateInfo);

    resultCheck(pipelineResult.result, "Failed to create pipeline!");

    m_pipeline = pipelineResult.value;

    m_logicalDevice.destroyShaderModule(vertShaderModule);
    m_logicalDevice.destroyShaderModule(fragShaderModule);
}

void BasicTriangleApplication::createRenderPass()
{
    std::vector colorAttachments = {
        vk::AttachmentDescription {
            {},
            m_swapChainImageFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR
        }
    };

    std::vector colorAttachmentRefs = {
        vk::AttachmentReference  {
            0,
            vk::ImageLayout::eColorAttachmentOptimal
        }
    };

    std::vector subpasses = {
        vk::SubpassDescription {
            {},
            vk::PipelineBindPoint::eGraphics,
            {},
            colorAttachmentRefs,
            {}
        }
    };

    std::vector dependencies{
        vk::SubpassDependency {
            vk::SubpassExternal,
            0,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            {},
            vk::AccessFlagBits::eColorAttachmentWrite
        }
    };

    m_renderPass = m_logicalDevice.createRenderPass({ {}, colorAttachments, subpasses, dependencies });
}

void BasicTriangleApplication::createFrameBuffers()
{
    m_swapChainFrameBuffers.clear();
    m_swapChainFrameBuffers.reserve(m_swapChainImageViews.size());

    for (auto const& imageView : m_swapChainImageViews)
    {
        std::vector attachments = { imageView };

        vk::FramebufferCreateInfo frameBufferInfo = {
            {},
            m_renderPass,
            attachments,
            m_swapChainExtent.width,
            m_swapChainExtent.height,
            1
        };

        m_swapChainFrameBuffers.push_back(m_logicalDevice.createFramebuffer(frameBufferInfo));
    }
}

void BasicTriangleApplication::createCommandPool()
{
    auto queueFamilyIndices = m_physicalDevice.GetQueueFamilyIndices(m_surface);

    if (!queueFamilyIndices.graphicsFamilyIndex)
    {
        throw std::runtime_error("Graphics Family Index is always expected");
    }

    vk::CommandPoolCreateInfo poolInfo{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        queueFamilyIndices.graphicsFamilyIndex.value()
    };

    m_commandPool = m_logicalDevice.createCommandPool(poolInfo);
}

void BasicTriangleApplication::createVertexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();
    auto stagingBufferUsage = vk::BufferUsageFlagBits::eTransferSrc;
    auto stagingMemoryUsage = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    createBuffer(bufferSize, stagingBufferUsage, stagingMemoryUsage, stagingBuffer, stagingBufferMemory);

    { // Scoping raw pointer
        auto data = m_logicalDevice.mapMemory(stagingBufferMemory, 0, bufferSize);
        memcpy(data, m_vertices.data(), bufferSize);
        m_logicalDevice.unmapMemory(stagingBufferMemory);
    }

    auto vertexBufferUsage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    auto vertexMemoryUsage = vk::MemoryPropertyFlagBits::eDeviceLocal;

    createBuffer(bufferSize, vertexBufferUsage, vertexMemoryUsage, m_vertexBuffer, m_vertexBufferMemory);

    copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    m_logicalDevice.destroyBuffer(stagingBuffer);
    m_logicalDevice.freeMemory(stagingBufferMemory);
}

void BasicTriangleApplication::createIndexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();
    auto stagingBufferUsage = vk::BufferUsageFlagBits::eTransferSrc;
    auto stagingMemoryUsage = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    createBuffer(bufferSize, stagingBufferUsage, stagingMemoryUsage, stagingBuffer, stagingBufferMemory);

    { // Scoping raw pointer
        auto data = m_logicalDevice.mapMemory(stagingBufferMemory, 0, bufferSize);
        memcpy(data, m_indices.data(), bufferSize);
        m_logicalDevice.unmapMemory(stagingBufferMemory);
    }

    auto indexBufferUsage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    auto indexMemoryUsage = vk::MemoryPropertyFlagBits::eDeviceLocal;

    createBuffer(bufferSize, indexBufferUsage, indexMemoryUsage, m_indexBuffer, m_indexBufferMemory);

    copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    m_logicalDevice.destroyBuffer(stagingBuffer);
    m_logicalDevice.freeMemory(stagingBufferMemory);
}

void BasicTriangleApplication::createUniformBuffers()
{
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniformBuffers.resize(m_maxFramesInFlight);
    m_uniformBuffersMemory.resize(m_maxFramesInFlight);
    m_uniformBuffersMapped.resize(m_maxFramesInFlight);

    for (size_t i = 0; i < m_maxFramesInFlight; i++)
    {
        createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, 
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 
                     m_uniformBuffers[i], 
                     m_uniformBuffersMemory[i]);

        m_uniformBuffersMapped[i] = m_logicalDevice.mapMemory(m_uniformBuffersMemory[i], 0, bufferSize);
    }
}

void BasicTriangleApplication::createDescriptorPool()
{
    std::vector poolSizes = {
        vk::DescriptorPoolSize
        {
            vk::DescriptorType::eUniformBuffer,
            static_cast<uint32_t>(m_maxFramesInFlight)
        }
    };

    vk::DescriptorPoolCreateInfo poolInfo
    {
        {},
        static_cast<uint32_t>(m_maxFramesInFlight),
        poolSizes
    };

    m_descriptorPool = m_logicalDevice.createDescriptorPool(poolInfo);
}

void BasicTriangleApplication::createDescriptorSets()
{
    std::vector descriptorSetLayouts(m_maxFramesInFlight, m_descriptorSetLayout);

    vk::DescriptorSetAllocateInfo allocInfo
    {
        m_descriptorPool,
        descriptorSetLayouts
    };

    m_descriptorSets = m_logicalDevice.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < m_maxFramesInFlight; i++)
    {
        std::vector bufferInfos = {
            vk::DescriptorBufferInfo
            {
                m_uniformBuffers[i],
                0,
                sizeof(UniformBufferObject)
            }
        };

        std::vector descriptorWrites = {
            vk::WriteDescriptorSet
            {
                m_descriptorSets[i],
                0,
                0,
                vk::DescriptorType::eUniformBuffer,
                {},
                bufferInfos
            }
        };

        m_logicalDevice.updateDescriptorSets(descriptorWrites,{});
    }
}

void BasicTriangleApplication::createBuffer(
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties,
    vk::Buffer& buffer,
    vk::DeviceMemory& bufferMemory
) const
{
    vk::BufferCreateInfo bufferInfo{
        {},
        size,
        usage,
        vk::SharingMode::eExclusive
    };

    buffer = m_logicalDevice.createBuffer(bufferInfo);

    auto memoryRequirements = m_logicalDevice.getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocInfo{
        memoryRequirements.size,
        findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };

    bufferMemory = m_logicalDevice.allocateMemory(allocInfo);

    m_logicalDevice.bindBufferMemory(buffer, bufferMemory, 0);
}

void BasicTriangleApplication::copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) const
{
    vk::CommandBufferAllocateInfo allocInfo{
        m_commandPool,
        vk::CommandBufferLevel::ePrimary,
        1
    };

    auto copyBuffers = m_logicalDevice.allocateCommandBuffers(allocInfo);

    auto& copyBuffer = copyBuffers.front();

    vk::CommandBufferBeginInfo beginInfo{
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };

    copyBuffer.begin(beginInfo);

    std::vector copyRegions{
        vk::BufferCopy { 0, 0, size }
    };

    copyBuffer.copyBuffer(src, dst, copyRegions);

    copyBuffer.end();

    vk::SubmitInfo submitInfo{
        {},
        {},
        copyBuffers
    };

    m_gfxQueue.submit(submitInfo);
    m_gfxQueue.waitIdle();

    m_logicalDevice.freeCommandBuffers(m_commandPool, copyBuffers);
}

uint32_t BasicTriangleApplication::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
    auto memoryProperties = m_physicalDevice.GetPDevice().getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if (typeFilter & 1 << i && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void BasicTriangleApplication::createCommandBuffer()
{
    vk::CommandBufferAllocateInfo bufferAllocInfo{
        m_commandPool,
        vk::CommandBufferLevel::ePrimary,
        static_cast<uint32_t>(m_maxFramesInFlight)
    };

    m_commandBuffer = m_logicalDevice.allocateCommandBuffers(bufferAllocInfo);
}

void BasicTriangleApplication::createSyncObjects()
{
    for (size_t i = 0; i <= m_maxFramesInFlight; i++)
    {
        m_imageAvailable.push_back(m_logicalDevice.createSemaphore({}));
        m_renderFinished.push_back(m_logicalDevice.createSemaphore({}));
        m_inFlight.push_back(m_logicalDevice.createFence({ vk::FenceCreateFlagBits::eSignaled }));
    }
}

void BasicTriangleApplication::recordCommandBuffer(vk::CommandBuffer buffer, uint32_t imageIndex /*TODO: Potential refactor */)
{
    vk::CommandBufferBeginInfo beginInfo{};

    buffer.begin(beginInfo);

    std::vector clearColors = {
        vk::ClearValue {
            std::array {0.0f,0.0f,0.0f,1.0f}
        }
    };

    vk::RenderPassBeginInfo renderPassInfo{
        m_renderPass,
        m_swapChainFrameBuffers[imageIndex],
        vk::Rect2D {{0,0}, m_swapChainExtent},
        clearColors
    };

    buffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

    std::vector viewports = {
        vk::Viewport {
            0.0f,
            0.0f,
            static_cast<float>(m_swapChainExtent.width),
            static_cast<float>(m_swapChainExtent.height),
            0.0f,
            0.0f
        }
    };

    buffer.setViewport(0, viewports);

    std::vector scissors = {
        vk::Rect2D {
            {0, 0},
            m_swapChainExtent
        }
    };

    buffer.setScissor(0, scissors);

    std::vector vBuffers = { m_vertexBuffer };
    std::vector<vk::DeviceSize> vOffsets = { 0 };

    buffer.bindVertexBuffers(0, vBuffers, vOffsets);

    buffer.bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint16);

    std::vector currentDescriptorSets = { m_descriptorSets[m_currentFrame] };

    buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, currentDescriptorSets, {});

    buffer.drawIndexed(static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);

    buffer.endRenderPass();

    buffer.end();
}

void BasicTriangleApplication::mainLoop()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        drawFrame();
    }

    m_logicalDevice.waitIdle();
}

void BasicTriangleApplication::drawFrame()
{
    auto& currentCommandBuffer = m_commandBuffer[m_currentFrame];
    auto& currentImageAvailable = m_imageAvailable[m_currentFrame];
    auto& currentRenderFinished = m_renderFinished[m_currentFrame];
    auto& currentInFlight = m_inFlight[m_currentFrame];

    const std::vector inFlightFences = { currentInFlight };

    std::ignore = m_logicalDevice.waitForFences(inFlightFences, vk::True, UINT64_MAX);

    unsigned int nextImage;
    try
    {
        auto nextImageResult = m_logicalDevice.acquireNextImageKHR(m_swapChain, UINT64_MAX, currentImageAvailable);
        if (nextImageResult.result != vk::Result::eSuccess && nextImageResult.result != vk::Result::eSuboptimalKHR)
        {
            throw std::runtime_error("failed to acquire swapchain image!");
        }
        nextImage = nextImageResult.value;
    }
    catch (vk::OutOfDateKHRError const&)
    {
        recreateSwapChain();
        return;
    }

    m_logicalDevice.resetFences(inFlightFences);

    currentCommandBuffer.reset();

    recordCommandBuffer(currentCommandBuffer, nextImage);

    updateUniformBuffer(m_currentFrame);

    std::vector waitSemaphores = { currentImageAvailable };
    std::vector<vk::PipelineStageFlags> waitDstStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    std::vector commandBuffers = { currentCommandBuffer };
    std::vector signalSemaphores = { currentRenderFinished };

    std::vector submitInfos{
        vk::SubmitInfo {
            waitSemaphores,
            waitDstStages,
            commandBuffers,
            signalSemaphores
        }
    };

    m_gfxQueue.submit(submitInfos, currentInFlight);

    std::vector swapchains = { m_swapChain };
    std::vector imageIndices = { nextImage };

    vk::PresentInfoKHR presentInfo{
        signalSemaphores,
        swapchains,
        imageIndices
    };

    try
    {
        auto presentResult = m_presentQueue.presentKHR(presentInfo);

        if (presentResult == vk::Result::eSuboptimalKHR || m_frameBufferResized)
        {
            recreateSwapChain();
            m_frameBufferResized = false;
        }
    }
    catch (vk::OutOfDateKHRError const&)
    {
        recreateSwapChain();
    }

    m_currentFrame = (m_currentFrame + 1) % m_maxFramesInFlight;
}

void BasicTriangleApplication::updateUniformBuffer(size_t currentFrame)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();

    float time = std::chrono::duration<float>(currentTime - startTime).count();

    UniformBufferObject ubo
    {
        .model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .proj = glm::perspective(glm::radians(45.0f), static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height), 0.1f, 10.0f)
    };

    ubo.proj[1][1] *= -1;

    memcpy(m_uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

void BasicTriangleApplication::cleanupSwapChain()
{
    for (auto frameBuffer : m_swapChainFrameBuffers)
    {
        m_logicalDevice.destroyFramebuffer(frameBuffer);
    }
    m_swapChainFrameBuffers.clear();

    for (auto const& imageView : m_swapChainImageViews)
    {
        m_logicalDevice.destroyImageView(imageView);
    }
    m_swapChainImageViews.clear();

    m_logicalDevice.destroySwapchainKHR(m_swapChain);
}

void BasicTriangleApplication::recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);

    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_logicalDevice);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createFrameBuffers();
}

void BasicTriangleApplication::cleanup()
{
    for (auto const& sem : m_imageAvailable)
    {
        m_logicalDevice.destroySemaphore(sem);
    }
    for (auto const& sem : m_renderFinished)
    {
        m_logicalDevice.destroySemaphore(sem);
    }
    for (auto const& fence : m_inFlight)
    {
        m_logicalDevice.destroyFence(fence);
    }

    m_logicalDevice.destroyCommandPool(m_commandPool);

    m_logicalDevice.destroyBuffer(m_indexBuffer);
    m_logicalDevice.freeMemory(m_indexBufferMemory);

    m_logicalDevice.destroyBuffer(m_vertexBuffer);
    m_logicalDevice.freeMemory(m_vertexBufferMemory);

    cleanupSwapChain();

    for (size_t i = 0; i < m_maxFramesInFlight; i++)
    {
        m_logicalDevice.destroyBuffer(m_uniformBuffers[i]);
        m_uniformBuffersMapped[i] = nullptr;
        m_logicalDevice.freeMemory(m_uniformBuffersMemory[i]);
    }

    m_logicalDevice.destroyDescriptorPool(m_descriptorPool);

    m_logicalDevice.destroyDescriptorSetLayout(m_descriptorSetLayout);

    m_logicalDevice.destroyPipeline(m_pipeline);

    m_logicalDevice.destroyPipelineLayout(m_pipelineLayout);

    m_logicalDevice.destroyRenderPass(m_renderPass);

    m_logicalDevice.destroy();

    if (EnableValidationLayers)
    {
        m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger);
    }

    m_instance.destroySurfaceKHR(m_surface);

    m_instance.destroy();

    glfwDestroyWindow(m_window);
    m_window = nullptr;

    glfwTerminate();
}

void BasicTriangleApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = static_cast<BasicTriangleApplication*>(glfwGetWindowUserPointer(window));

    app->m_frameBufferResized = true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL BasicTriangleApplication::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT vkMessageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT vkMessageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
)
{
    std::cout << "Validation Layer: " << pCallbackData->pMessage << '\n';

    return VK_FALSE;
}
