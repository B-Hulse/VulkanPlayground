#include "pch.h"
#include "PhysicalDeviceHelpers.h"

vk::PhysicalDevice PhysicalDevice::GetPDevice() const
{
    return m_physicalDevice;
}

QueueFamilyIndices PhysicalDevice::getQueueFamilyIndices(vk::SurfaceKHR const& surface) const
{
    QueueFamilyIndices indices;

    auto const queueFamilies = m_physicalDevice.getQueueFamilyProperties();

    uint32_t queueFamilyIndex = 0;
    for (auto const& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamilyIndex = queueFamilyIndex;
        }

        if (m_physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, surface))
        {
            indices.presentFamilyIndex = queueFamilyIndex;
        }

        if (indices.isComplete())
        {
            break;
        }

        queueFamilyIndex++;
    }

    return indices;
}

SwapChainSupport PhysicalDevice::getSwapChainSupport(vk::SurfaceKHR const& surface) const
{
    return {
        m_physicalDevice.getSurfaceCapabilitiesKHR(surface),
        m_physicalDevice.getSurfaceFormatsKHR(surface),
        m_physicalDevice.getSurfacePresentModesKHR(surface)
    };
}

QueueFamilyIndices PhysicalDevice::GetQueueFamilyIndices(vk::SurfaceKHR const& surface, bool refresh /*= false*/)
{
    if (auto const props = m_surfaceMap.find(surface); !refresh && props != m_surfaceMap.end() && props->second.queueFamilyIndices)
    {
        return *props->second.queueFamilyIndices;
    }

    auto queueFamilyIndices = getQueueFamilyIndices(surface);

    m_surfaceMap[surface].queueFamilyIndices = { queueFamilyIndices };
    return queueFamilyIndices;
}

SwapChainSupport PhysicalDevice::GetSwapChainSupport(vk::SurfaceKHR const& surface, bool refresh /*= false*/)
{
    if (auto const props = m_surfaceMap.find(surface); !refresh && props != m_surfaceMap.end() && props->second.swapChainSupport)
    {
        return *props->second.swapChainSupport;
    }

    auto swapChainSupport = getSwapChainSupport(surface);

    m_surfaceMap[surface].swapChainSupport = { swapChainSupport };
    return swapChainSupport;
}

std::optional<PhysicalDevice> FindBestPhysicalDevice(vk::Instance const& instance, vk::SurfaceKHR const& surface,
                                                     std::function<uint32_t(
                                                         PhysicalDevice&,
                                                         vk::SurfaceKHR const&)> const& scoreFunc)
{
    auto const devices = instance.enumeratePhysicalDevices();

    uint32_t bestScore = 0;
    std::optional<PhysicalDevice> bestDevice;

    for (auto const& dvc : devices)
    {
        PhysicalDevice device(dvc);
        if (auto const score = scoreFunc(device, surface); score > bestScore)
        {
            bestScore = score;
            bestDevice = device;
        }
    }

    return bestDevice;
}
