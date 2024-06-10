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

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
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
    createGraphicsPipeline();
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
    const void* pNext;
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
        vk::ApiVersion13
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

    std::vector queuePriority = {1.0f};

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

    std::set queueFamilyIndicesSet = {*indices.graphicsFamilyIndex, *indices.presentFamilyIndex};
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

void BasicTriangleApplication::createGraphicsPipeline()
{
    auto const vertShaderModule = CreateShaderModule(m_logicalDevice, "shaders/shader.vert.spv");
    auto const fragShaderModule = CreateShaderModule(m_logicalDevice, "shaders/shader.frag.spv");

    vk::PipelineShaderStageCreateInfo vertShaderStageCreate(
        {},
        vk::ShaderStageFlagBits::eVertex,
        vertShaderModule,
        "main"
    );
    vk::PipelineShaderStageCreateInfo fragShaderStageCreate(
        {},
        vk::ShaderStageFlagBits::eFragment,
        fragShaderModule,
        "main"
    );

    vk::PipelineVertexInputStateCreateInfo const vertexInputCreate{};

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        {{}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main"},
        {{}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main"}
    };

    std::vector dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{{}, dynamicStates};

    vk::PipelineInputAssemblyStateCreateInfo const inputAssemblyCreateInfo{
        {},
        vk::PrimitiveTopology::eTriangleList,
        false
    };

    vk::PipelineViewportStateCreateInfo const viewportStateCreateInfo{{}, 1, nullptr, 1, nullptr};

    vk::PipelineRasterizationStateCreateInfo const rasterizationStateCreateInfo{
        {},
        false,
        false,
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise,
        false,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    vk::PipelineMultisampleStateCreateInfo const multisampleStateCreateInfo{ {}, vk::SampleCountFlagBits::e1, false };
    

    m_logicalDevice.destroyShaderModule(vertShaderModule);
    m_logicalDevice.destroyShaderModule(fragShaderModule);
}

void BasicTriangleApplication::mainLoop()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
    }
}

void BasicTriangleApplication::cleanup()
{
    for (auto const& imageView : m_swapChainImageViews)
    {
        m_logicalDevice.destroyImageView(imageView);
    }

    m_logicalDevice.destroySwapchainKHR(m_swapChain);

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

VKAPI_ATTR VkBool32 VKAPI_CALL BasicTriangleApplication::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT vkMessageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT vkMessageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
)
{
    std::cout << "Validation Layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
