#pragma once

#include <vector>

#include "vulkan/vulkan.hpp"


namespace vulkan2d{


class Shader{
public:
    Shader(const std::vector<char>& vertexSource, const std::vector<char>& fragmentSource);
    ~Shader();

    vk::ShaderModule getVertexShaderModule() const { return m_vertexModule; }
    vk::ShaderModule getFragmentShaderModule() const { return m_fragmentModule; }
    const std::vector<vk::DescriptorSetLayout>& getDescriptorSetLayouts() const { return m_descriptorSetLayouts; }


private:
    vk::ShaderModule                     m_vertexModule;
    vk::ShaderModule                     m_fragmentModule;
    std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts; 

    void initDescriptorSetLayout();
};







}