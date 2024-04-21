#pragma once

std::vector<const char*> GetRequiredExtensions(bool enableValidationLayers);
bool AreRequiredExtensionsSupported(std::vector<const char*> const& requiredExtensions);