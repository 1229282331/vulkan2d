#pragma once

#include "vulkan/vulkan.hpp"


namespace vulkan2d{

class CommandManager{
public:
    CommandManager();
    ~CommandManager();

    std::vector<vk::CommandBuffer> allocateCommandBuffers(uint32_t count);
    void freeCommandBuffers();

private:
    vk::CommandPool m_pool;

    vk::CommandPool createCommandPool();



};









}