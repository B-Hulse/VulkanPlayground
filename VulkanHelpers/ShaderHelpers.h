#pragma once

std::vector<uint32_t> ReadShaderFile(std::string const& fileName);

vk::ShaderModule CreateShaderModule(vk::Device const& device, std::string const& fileName);