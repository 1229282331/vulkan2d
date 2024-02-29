#include "command_manager.hpp"
#include "vkBase.hpp"


namespace vulkan2d{

CommandManager::CommandManager()
{
    m_pool = createCommandPool();
}

CommandManager::~CommandManager()
{
    VkBase::self().device.destroyCommandPool(m_pool);
}

std::vector<vk::CommandBuffer> CommandManager::allocateCommandBuffers(uint32_t count)
{
    vk::CommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.setCommandBufferCount(count)           
                .setCommandPool(m_pool)
                .setLevel(vk::CommandBufferLevel::ePrimary);    /*设置命令缓冲等级为主命令缓冲，可直接提交至队列执行*/
    return VkBase::self().device.allocateCommandBuffers(allocateInfo);
}

void CommandManager::freeCommandBuffers()
{
    auto& base_instance = VkBase::self();

    base_instance.device.freeCommandBuffers(m_pool, base_instance.renderer->getCommandBuffers());
}


vk::CommandPool CommandManager::createCommandPool()
{
    vk::CommandPoolCreateInfo createInfo = {};
    createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
              .setQueueFamilyIndex(VkBase::self().queueFamilyIndex.graphicsIndex.value());  /*设置命令缓冲提交到的队列索引*/

    return VkBase::self().device.createCommandPool(createInfo);
}




}