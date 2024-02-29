#include "renderer.hpp"
#include "vkBase.hpp"


namespace vulkan2d{

Renderer::Renderer(int maxFlightCount) : m_currentFrame(0), m_maxFlightCount(maxFlightCount)
{
    size_t swapchainSize = VkBase::self().swapchain->images.size();
    m_flightCount = (swapchainSize>m_maxFlightCount) ? m_maxFlightCount : swapchainSize;
    m_commandbuffers = createCommandBuffers();
    m_descriptorSets = createDescriptorSets();
    initFences();
    initSemaphores();
}
Renderer::~Renderer()
{
    auto& base_instance = VkBase::self(); 
    for(int i=0; i<m_flightCount; i++)
    {
        base_instance.device.destroySemaphore(m_renderFinishedSemaphores[i]);
        base_instance.device.destroySemaphore(m_imageAvailbleSemaphores[i]);
    }
    for(int i=0; i<m_flightCount; i++)
        base_instance.device.destroyFence(m_inflightFences[i]);

}

void Renderer::updateDescriptorSets(const std::vector<std::unique_ptr<Buffer>>& buffers)
{
    /*配置描述符*/
    for(int i=0; i<m_flightCount; i++)
    {
        vk::DescriptorBufferInfo bufferInfo = {};
        bufferInfo.setBuffer(buffers[i]->buffer)            /*设置绑定的描述符缓冲*/
                  .setOffset(0)                             /*设置该缓冲偏移量*/
                  .setRange(sizeof(UniformBufferObject));   /*设置描述符缓冲大小*/
        
        vk::WriteDescriptorSet descriptorWrite = {};
        descriptorWrite.dstSet = m_descriptorSets[i];                       /*设置待更新描述符所属的描述符集*/
        descriptorWrite.dstBinding = 0;                                     /*设置该描述符集绑定的shader对应索引*/
        descriptorWrite.dstArrayElement = 0;                                /*设置目标描述符所在数组中的索引*/
        descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;/*设置描述符类型*/
        descriptorWrite.descriptorCount = 1;                                /*设置描述符数量*/
        descriptorWrite.pBufferInfo = &bufferInfo;                          /*设置描述符绑定的缓冲信息*/
        VkBase::self().device.updateDescriptorSets(descriptorWrite, nullptr);
    }
}


void Renderer::drawFrame()
{
    auto& base_instance = VkBase::self(); 
    if(base_instance.device.waitForFences(m_inflightFences[m_currentFrame], false, std::numeric_limits<uint64_t>::max())!=vk::Result::eSuccess)
        std::cout << "Waiting for signal fences error!" << std::endl;
    base_instance.device.resetFences(m_inflightFences[m_currentFrame]);

    /*1.从交换链获取一张图像*/
    auto res = base_instance.device.acquireNextImageKHR(base_instance.swapchain->swapchain, std::numeric_limits<uint64_t>::max(), m_imageAvailbleSemaphores[m_currentFrame]);
    if(res.result == vk::Result::eErrorOutOfDateKHR)
    {
        base_instance.recreateSwapchain();
        std::cout << "recreate swapchain!" << std::endl;
        return;
    }
    if(res.result != vk::Result::eSuccess && res.result != vk::Result::eSuboptimalKHR)
        throw std::runtime_error("[ Swapchian ]: Can't acquire next image from swapchian!");
    m_imageIndex = res.value;

    /*2.记录命令到命令缓冲*/
    m_commandbuffers[m_currentFrame].reset();
    recordCommandBuffer(m_commandbuffers[m_currentFrame], m_imageIndex);
    
    /*3.更新MVP矩阵*/
    base_instance.updateUniformBuffers(m_currentFrame);

    /*4.提交命令缓冲*/
    vk::SubmitInfo submitInfo = {};
    std::vector<vk::PipelineStageFlags>  waitPipelineStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setWaitSemaphores(m_imageAvailbleSemaphores[m_currentFrame])        /*设置该命令缓冲需要等待的信号量*/
              .setWaitDstStageMask(waitPipelineStages)          /*设置需要等待管线到达指定阶段*/
              .setCommandBuffers(m_commandbuffers[m_currentFrame])    /*设置待提交的命令缓冲*/
              .setSignalSemaphores(m_renderFinishedSemaphores[m_currentFrame]);    /*设置命令缓冲执行完成后发出的信号量*/
    base_instance.graphicsQueue.submit(submitInfo, m_inflightFences[m_currentFrame]);

    /*4.显示图像*/
    vk::PresentInfoKHR presentInfo = {};
    presentInfo.setWaitSemaphores(m_renderFinishedSemaphores[m_currentFrame])  /*设置该命令缓冲需要等待的信号量*/
               .setSwapchains(base_instance.swapchain->swapchain)         /*设置待显示图像所在的交换链*/
               .setImageIndices(m_imageIndex)                 /*设置待显示图像的索引*/
               .setPResults(nullptr);                       /*设置显示后的结果存储*/
    vk::Result result = base_instance.presentQueue.presentKHR(presentInfo);
    
    m_currentFrame = (m_currentFrame+1) % m_flightCount;
}

vk::Result Renderer::getSwapchainState()
{
    auto& base_instance = VkBase::self();
    vk::ResultValue res = base_instance.device.acquireNextImageKHR(base_instance.swapchain->swapchain, std::numeric_limits<uint64_t>::max(), m_imageAvailbleSemaphores[m_currentFrame]);
    return res.result;
}


std::vector<vk::CommandBuffer> Renderer::createCommandBuffers()
{
    return VkBase::self().commandManager->allocateCommandBuffers(m_flightCount);
}

std::vector<vk::DescriptorSet> Renderer::createDescriptorSets()
{
    return VkBase::self().descriptorManager->allocateDescriptorSets(m_flightCount);
}


void Renderer::initFences()
{
    m_inflightFences.resize(m_flightCount);
    vk::FenceCreateInfo createInfo = {};
    createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);    /*初始化fence时置位信号*/
    for(auto& e : m_inflightFences)
        e = VkBase::self().device.createFence(createInfo);
}

void Renderer::initSemaphores()
{
    m_imageAvailbleSemaphores.resize(m_flightCount);
    m_renderFinishedSemaphores.resize(m_flightCount);
    vk::SemaphoreCreateInfo createInfo = {};
    for(int i=0; i<m_flightCount; i++)
    {
        m_imageAvailbleSemaphores[i] = VkBase::self().device.createSemaphore(createInfo);
        m_renderFinishedSemaphores[i] = VkBase::self().device.createSemaphore(createInfo);
    }
}

void Renderer::recordCommandBuffer(vk::CommandBuffer& commandBuffer, uint32_t imageIndex)
{
    auto& base_instance = VkBase::self(); 
    /*开始命令缓冲*/
    vk::CommandBufferBeginInfo cbBeginInfo = {};
    cbBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)  /*设置命令缓冲使用方法为可同时提交*/
               .setPInheritanceInfo(nullptr);
    commandBuffer.begin(cbBeginInfo);

    /*设置渲染过程开始信息*/
    vk::ClearValue clearColor;
    clearColor.setColor(vk::ClearColorValue(std::array<float,4>{0.0, 0.0, 0.0, 1}));
    vk::RenderPassBeginInfo passBeginInfo = {};
    passBeginInfo.setRenderPass(base_instance.renderProcess->renderPass)                  /*设置渲染流程*/
                 .setFramebuffer(base_instance.swapchain->framebuffers[imageIndex])       /*设置待渲染的framebuffer*/
                 .setRenderArea(vk::Rect2D({0,0}, base_instance.swapchain->getExtent()))  /*设置渲染区域*/
                 .setClearValues(clearColor);                                             /*设置VK_ATTACHMENT_LOAD_OP_CLEAR渲染前清屏值*/

    /*渲染过程*/
    commandBuffer.beginRenderPass(passBeginInfo, vk::SubpassContents::eInline); /*设置如何提供命令（是否有辅助命令缓冲）*/
    {
        /*绑定渲染管线*/
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, base_instance.renderProcess->graphicsPipeline_triangle);
        /*绑定顶点缓冲*/
        std::vector<vk::Buffer> buffers = { base_instance.vertexBuffer->buffer };  
        std::vector<vk::DeviceSize> offsets = {0};
        commandBuffer.bindVertexBuffers(0, buffers, offsets);
        /*绑定顶点索引*/
        commandBuffer.bindIndexBuffer(base_instance.indexBuffer->buffer, 0, vk::IndexType::eUint16);
        /*绑定uniform变量*/
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, base_instance.renderProcess->pipelineLayout, 0, base_instance.renderer->getDescriptorSets()[m_currentFrame], {});
        /*重新设置一下视口和裁剪*/
        vk::Viewport viewport = {};
        viewport.setX(0).setY(0)
                .setWidth(base_instance.swapchain->getExtent().width).setHeight(base_instance.swapchain->getExtent().height)
                .setMinDepth(0.0).setMaxDepth(1.0);
        commandBuffer.setViewport(0, viewport);
        vk::Rect2D scissor = {};
        scissor.setOffset({0, 0}).setExtent(base_instance.swapchain->getExtent());
        commandBuffer.setScissor(0, scissor);
        /*绘制*/
        commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);
    }
    commandBuffer.endRenderPass();

    /*结束命令缓冲*/
    commandBuffer.end();
}





}