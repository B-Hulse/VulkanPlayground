#pragma once

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamilyIndex = std::nullopt;
    std::optional<uint32_t> presentFamilyIndex = std::nullopt;

    bool isComplete() const
    {
        return graphicsFamilyIndex.has_value() && presentFamilyIndex.has_value();
    }
};

struct SwapChainSupport
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct PDeviceToSurfaceProps
{
    std::optional<QueueFamilyIndices> queueFamilyIndices;
    std::optional<SwapChainSupport> swapChainSupport;
};

class PhysicalDevice
{
public:
    PhysicalDevice() = default;
    PhysicalDevice(vk::PhysicalDevice device)
        : m_physicalDevice(device)
    {}

    vk::PhysicalDevice GetPDevice() const;
    QueueFamilyIndices GetQueueFamilyIndices(vk::SurfaceKHR const& surface);
    SwapChainSupport GetSwapChainSupport(vk::SurfaceKHR const& surface);

private:
    QueueFamilyIndices getQueueFamilyIndices(vk::SurfaceKHR const& surface) const;
    SwapChainSupport getSwapChainSupport(vk::SurfaceKHR const& surface) const;


    vk::PhysicalDevice m_physicalDevice;
    std::unordered_map<VkSurfaceKHR, PDeviceToSurfaceProps> m_surfaceMap;
};

std::optional<PhysicalDevice> FindBestPhysicalDevice(vk::Instance const& instance, vk::SurfaceKHR const& surface,
                                                     std::function<uint32_t(
                                                         PhysicalDevice&,
                                                         vk::SurfaceKHR const&)> const& scoreFunc);