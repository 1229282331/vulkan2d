#include "shader.hpp"
#include "vkBase.hpp"



namespace vulkan2d{

Shader::Shader(const std::vector<char>& vertexSource, const std::vector<char>& fragmentSource)
{
    /*1.创建着色器模组*/
    vk::ShaderModuleCreateInfo vertex_createInfo = {}; 
    vertex_createInfo.setPNext(nullptr)
                     .setCodeSize(vertexSource.size())
                     .setPCode(reinterpret_cast<const uint32_t*>(vertexSource.data()));
    m_vertexModule = VkBase::self().device.createShaderModule(vertex_createInfo);
    vk::ShaderModuleCreateInfo fragment_createInfo = {}; 
    fragment_createInfo.setPNext(nullptr)
                     .setCodeSize(fragmentSource.size())
                     .setPCode(reinterpret_cast<const uint32_t*>(fragmentSource.data()));
    m_fragmentModule = VkBase::self().device.createShaderModule(fragment_createInfo);
    
    /*2.初始化描述符集布局*/
    initDescriptorSetLayout();
}

Shader::~Shader()
{
    for(auto& e : m_descriptorSetLayouts)
        VkBase::self().device.destroyDescriptorSetLayout(e);
    m_descriptorSetLayouts.clear();
    VkBase::self().device.destroyShaderModule(m_fragmentModule);
    VkBase::self().device.destroyShaderModule(m_vertexModule);
}

void Shader::initDescriptorSetLayout()
{
    vk::DescriptorSetLayoutCreateInfo createInfo = {};
    vk::DescriptorSetLayoutBinding binding = {};
    binding.setBinding(0)
           .setDescriptorCount(1)
           .setDescriptorType(vk::DescriptorType::eUniformBuffer)
           .setStageFlags(vk::ShaderStageFlagBits::eVertex);
    createInfo.setBindings(binding);
    m_descriptorSetLayouts.push_back(VkBase::self().device.createDescriptorSetLayout(createInfo));
}




}