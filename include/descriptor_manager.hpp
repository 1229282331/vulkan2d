#pragma once

#include "vulkan/vulkan.hpp"


namespace vulkan2d{

class DescriptorManager{
public:
    DescriptorManager(uint32_t inflightCount);
    ~DescriptorManager();

    std::vector<vk::DescriptorSet> allocateDescriptorSets(uint32_t count);

private:
    vk::DescriptorPool m_pool;
    int m_inflightCount;
    
    vk::DescriptorPool createDescriptorPool();



};



}