#include "pch.h"
#include "ValidationLayerHelpers.h"

bool AreValidationLayersSupported(std::vector<const char*> const& requiredLayers)
{
    auto const supportedLayers = vk::enumerateInstanceLayerProperties();

    return std::ranges::all_of(
        requiredLayers,
        [supportedLayers](auto const& pExtension)
        {
            std::string extension(pExtension);
            return std::ranges::any_of(
                supportedLayers,
                [extension](vk::LayerProperties const& supportedLayer)
                {
                    return extension == supportedLayer.layerName;
                });
        }
    );
}
