#pragma once

#include "vulkan/vulkan.hpp"
#include "buffer.hpp"


namespace vulkan2d{

class Renderer{
public:
    Renderer(int maxFlightCount=5);
    ~Renderer();

    int getFlightCount() { return m_flightCount; }
    std::vector<vk::CommandBuffer>& getCommandBuffers() { return m_commandbuffers; }
    std::vector<vk::DescriptorSet>& getDescriptorSets() { return m_descriptorSets; }
    vk::Result getSwapchainState();

    void updateDescriptorSets(const std::vector<std::unique_ptr<Buffer>>& buffers);
    void drawFrame();

private:
    int                             m_currentFrame;
    uint32_t                        m_imageIndex;
    int                             m_flightCount;
    int                             m_maxFlightCount;
    std::vector<vk::CommandBuffer>  m_commandbuffers;
    std::vector<vk::DescriptorSet>  m_descriptorSets;
    std::vector<vk::Semaphore>      m_imageAvailbleSemaphores;
    std::vector<vk::Semaphore>      m_renderFinishedSemaphores;
    std::vector<vk::Fence>          m_inflightFences;

    std::vector<vk::CommandBuffer> createCommandBuffers();
    std::vector<vk::DescriptorSet> createDescriptorSets();
    void initFences();
    void initSemaphores();
    void recordCommandBuffer(vk::CommandBuffer& commandBuffer, uint32_t imageIndex);
    


};


}