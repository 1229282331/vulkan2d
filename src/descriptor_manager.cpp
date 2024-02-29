#include "descriptor_manager.hpp"
#include "vkBase.hpp"

namespace vulkan2d{
    

DescriptorManager::DescriptorManager(uint32_t inflightCount) : m_inflightCount(inflightCount)
{
    m_pool = createDescriptorPool();
}

DescriptorManager::~DescriptorManager()
{
    VkBase::self().device.destroyDescriptorPool(m_pool);
}

std::vector<vk::DescriptorSet> DescriptorManager::allocateDescriptorSets(uint32_t count)
{
    std::vector<vk::DescriptorSetLayout> layouts(count, VkBase::self().shader->getDescriptorSetLayouts()[0]);
    /*分配描述符集*/
    vk::DescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.setDescriptorPool(m_pool)
                .setDescriptorSetCount(count)
                .setSetLayouts(layouts);
    return VkBase::self().device.allocateDescriptorSets(allocateInfo);
}

vk::DescriptorPool DescriptorManager::createDescriptorPool()
{
    /*设置描述符集信息*/
    vk::DescriptorPoolSize poolSize;
    poolSize.setType(vk::DescriptorType::eUniformBuffer)    /*设置描述符集里的描述符类型*/
            .setDescriptorCount(1);           /*设置描述符集中包含的描述符个数*/

    /*根据描述符集创建描述符池*/
    vk::DescriptorPoolCreateInfo createInfo = {};
    createInfo.setPoolSizes(poolSize)                   /*设置pool分配的描述符集信息*/  
              .setMaxSets(m_inflightCount);             /*设置最多描述符集个数*/

    return VkBase::self().device.createDescriptorPool(createInfo);
}




} // namespace vulkan2d
