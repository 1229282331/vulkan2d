#pragma once

#include "vulkan/vulkan.hpp"


namespace vulkan2d{

class Commander{
public:
    Commander();
    ~Commander();

    void copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size);
    void copyBuffer(vk::Buffer srcBuffer, vk::Image dstImage, uint32_t width, uint32_t height);
    void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

private:
    vk::Fence         m_fence;
    vk::CommandPool   m_pool;
    vk::CommandBuffer m_cmdBuffer;

    void beginSingleTimeCommands();
    void endSingleTimeCommands();
    void submit();

};



}