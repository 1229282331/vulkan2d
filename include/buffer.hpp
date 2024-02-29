#pragma once

#include "vulkan/vulkan.hpp"


namespace vulkan2d{

struct Buffer{
    vk::Buffer       buffer;
    size_t           size;
    vk::DeviceMemory memory;
    size_t           memorySize;
    void*            data;


    Buffer(vk::BufferUsageFlags usage, size_t size, vk::MemoryPropertyFlags properties);
    ~Buffer();

};

uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);





}