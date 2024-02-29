#include "buffer.hpp"
#include "vkBase.hpp"

namespace vulkan2d{

Buffer::Buffer(vk::BufferUsageFlags usage, size_t size, vk::MemoryPropertyFlags properties)
{
    auto& base_instance = VkBase::self();
    /*1.创建内存缓冲*/
    this->size = size;
    vk::BufferCreateInfo createInfo = {};
    createInfo.setUsage(usage)                              /*内存缓冲用途*/
              .setSize(size)                                /*内存缓冲大小（字节）*/
              .setSharingMode(vk::SharingMode::eExclusive); /*共享模式：独有*/
    buffer = base_instance.device.createBuffer(createInfo);
    if(!buffer)
        throw std::runtime_error("[ Buffer ]: Can't create  vulkan buffer!");
    /*2.获取内存需求*/
    vk::MemoryRequirements requirements = base_instance.device.getBufferMemoryRequirements(buffer);
    memorySize = requirements.size;
    /*3.分配内存*/
    uint32_t typeIndex = findMemoryType(requirements.memoryTypeBits, properties);
    vk::MemoryAllocateInfo allocateInfo = {};
    allocateInfo.setAllocationSize(requirements.size)
                .setMemoryTypeIndex(typeIndex);
    memory = base_instance.device.allocateMemory(allocateInfo);
    if(!memory)
        throw std::runtime_error("[ Buffer ]: Can't allocate memory!");     
    /*4.绑定内存和缓冲对象*/
    base_instance.device.bindBufferMemory(buffer, memory, 0);
    /*5.内存映射*/
    if(properties & vk::MemoryPropertyFlagBits::eHostVisible)
        data = base_instance.device.mapMemory(memory, 0, size);
    else
        data = nullptr;        
}   

Buffer::~Buffer()
{
    auto& base_instance = VkBase::self();
    
    base_instance.device.freeMemory(memory);
    base_instance.device.destroyBuffer(buffer);
}

uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties supportProperties = VkBase::self().physicalDevice.getMemoryProperties();
    for(uint32_t i=0; i<supportProperties.memoryTypeCount; i++)
    {
        if(typeFilter&(1<<i) && (supportProperties.memoryTypes[i].propertyFlags&properties)==properties)
            return i;
    }
    throw std::runtime_error("[ Buffer ]: Can't find the support memoryType!");
    return 0;
}

}