#include "pch.h"
#include "ExtensionHelpers.h"

std::vector<const char*> GetRequiredExtensions(bool enabledValidationLayers)
{
    uint32_t requiredExtensionCount = 0;
    // Returned string memory is owned by GLFW and will be cleaned up during termination
    auto const pRequiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);

    std::vector requiredExtensions(pRequiredExtensions, pRequiredExtensions + requiredExtensionCount);

    if (enabledValidationLayers)
    {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return requiredExtensions;
}

bool AreRequiredExtensionsSupported(std::vector<const char*> const& requiredExtensions)
{
    auto supportedExtensions = vk::enumerateInstanceExtensionProperties(nullptr);

    return std::ranges::all_of(
        requiredExtensions,
        [supportedExtensions](auto const& pExtension)
        {
            std::string extension(pExtension);
            return std::ranges::any_of(
                supportedExtensions,
                [extension](vk::ExtensionProperties const& supportedExtension)
                {
                    return extension == supportedExtension.extensionName;
                });
        }
    );
}