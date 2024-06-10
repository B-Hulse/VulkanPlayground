#include "pch.h"
#include "ShaderHelpers.h"

std::vector<uint32_t> ReadShaderFile(std::string const& fileName)
{
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Couldn't open shader file: " + fileName);
    }

    auto const fileSize = file.tellg();

    std::vector<uint32_t> buffer(fileSize);

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

    file.close();

    return buffer;
}

vk::ShaderModule CreateShaderModule(vk::Device const& device, std::string const& fileName)
{
    auto const shaderFile = ReadShaderFile(fileName);
    return device.createShaderModule(vk::ShaderModuleCreateInfo{ {}, shaderFile });
}
