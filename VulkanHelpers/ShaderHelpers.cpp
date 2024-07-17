#include "pch.h"
#include "ShaderHelpers.h"

namespace
{
    std::vector<uint32_t> vectorCharToUInt32T(std::vector<char> input)
    {
        if (input.size() % 4 != 0)
        {
            throw std::runtime_error("SPIR-V Shader module size should be divisible by 4");
        }

        auto newVecSize = input.size() / 4;

        std::vector<uint32_t> newVec(newVecSize);

        for (size_t i = 0; i < newVecSize; ++i)
        {
            newVec[i] = *reinterpret_cast<uint32_t*>(input.data() + i*4);
        }

        return newVec;
    }
}

std::vector<uint32_t> ReadShaderFile(std::string const& fileName)
{
    std::ifstream file(fileName , std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);

    return vectorCharToUInt32T(buffer);
}

vk::ShaderModule CreateShaderModule(vk::Device const& device, std::string const& fileName)
{
    auto const shaderFile = ReadShaderFile(fileName);
    return device.createShaderModule(vk::ShaderModuleCreateInfo{ {}, shaderFile });
}
