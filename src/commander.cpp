#include "commander.hpp"
#include "vkBase.hpp"


namespace vulkan2d{

Commander::Commander()
{
    /*0.创建fence同步cpu与Gpu*/
    vk::FenceCreateInfo fenceCreateInfo = {};
    m_fence = VkBase::self().device.createFence(fenceCreateInfo);
    /*1.创建临时用于转移数据的command pool(使用与图形命令队列)*/
    vk::CommandPoolCreateInfo cmdPoolCreateInfo = {};
    cmdPoolCreateInfo.setFlags(vk::CommandPoolCreateFlagBits::eTransient|vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                     .setQueueFamilyIndex(VkBase::self().queueFamilyIndex.graphicsIndex.value());  
    m_pool = VkBase::self().device.createCommandPool(cmdPoolCreateInfo);
    /*2.分配命令缓冲*/
    vk::CommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.setCommandBufferCount(1)           
                .setCommandPool(m_pool)
                .setLevel(vk::CommandBufferLevel::ePrimary);    /*设置命令缓冲等级为主命令缓冲，可直接提交至队列执行*/
    m_cmdBuffer = VkBase::self().device.allocateCommandBuffers(allocateInfo)[0];
}

Commander::~Commander()
{
    /*清除栅栏和命令池*/
    VkBase::self().device.destroyFence(m_fence);
    VkBase::self().device.freeCommandBuffers(m_pool, m_cmdBuffer);
    VkBase::self().device.destroyCommandPool(m_pool);
}

void Commander::copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size)
{
    /*1.记录命令*/
    beginSingleTimeCommands();
        vk::BufferCopy copyRegion = {}; /*设置复制缓冲的区域*/
        copyRegion.setSrcOffset(0)      /*源缓冲待复制的起始位置*/
                  .setDstOffset(0)      /*目的缓冲待复制的起始位置*/
                  .setSize(size);       /*复制缓冲区域的大小*/
        m_cmdBuffer.copyBuffer(src, dst, copyRegion); /*执行拷贝内存操作*/
    endSingleTimeCommands();

    /*2.提交命令*/
    submit();

    /*3.复位栅栏并重置命令缓冲*/
    VkBase::self().device.resetFences(m_fence);
    m_cmdBuffer.reset();
}

void Commander::copyBuffer(vk::Buffer srcBuffer, vk::Image dstImage, uint32_t width, uint32_t height)
{
    /*1.记录命令*/
    beginSingleTimeCommands();
        vk::ImageSubresourceLayers subresourceLayers;
        subresourceLayers.setAspectMask(vk::ImageAspectFlagBits::eColor)    /*设置数据被复制的图像范围*/
                         .setMipLevel(0)                                    /*设置子资源mipmap起始索引*/
                         .setBaseArrayLayer(0).setLayerCount(1);            /*设置子资源纹理数组起始索引与数组数量*/
        vk::BufferImageCopy copyRegion = {};
        copyRegion.setBufferOffset(0)                               /*设置内存缓冲偏移*/
                  .setBufferRowLength(0)                            /*设置内存数据对齐方式：紧凑对齐*/
                  .setBufferImageHeight(0)                          /*同上*/
                  .setImageSubresource(subresourceLayers)           /*设置图像子资源*/
                  .setImageOffset(vk::Offset3D{0, 0, 0})            /*设置数据被复制到图像对象的偏移量*/
                  .setImageExtent(vk::Extent3D{width, height, 1});  /*设置数据被复制到图像对象的区域*/
        m_cmdBuffer.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, copyRegion);
    endSingleTimeCommands();

    /*2.提交命令*/
    submit();

    /*3.复位栅栏并重置命令缓冲*/
    VkBase::self().device.resetFences(m_fence);
    m_cmdBuffer.reset();
}


void Commander::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    /*0.设置屏障依赖条件(操作类型依赖/管线阶段依赖)*/
    vk::PipelineStageFlags srcStage, dstStage;
    vk::AccessFlags srcAccess, dstAccess;
    if(oldLayout==vk::ImageLayout::eUndefined && newLayout==vk::ImageLayout::eTransferDstOptimal)
    {
        srcAccess = vk::AccessFlagBits::eNone;              /*将布局变换为传输布局不需要等待任何操作*/
        dstAccess = vk::AccessFlagBits::eTransferWrite;     /*后续传输操作需要等待该函数布局转换完成*/
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;   /*屏障需要依赖的前置shader阶段*/
        dstStage = vk::PipelineStageFlagBits::eTransfer;    /*后续管线阶段需要等待该函数布局转换完成*/
    }
    else if(oldLayout==vk::ImageLayout::eTransferDstOptimal && newLayout==vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        srcAccess = vk::AccessFlagBits::eTransferWrite;         /*将布局变换为shader只读布局前需等待操作*/
        dstAccess = vk::AccessFlagBits::eShaderRead;            /*后续传输操作需要等待该函数布局转换完成*/
        srcStage = vk::PipelineStageFlagBits::eTransfer;        /*屏障需要依赖的前置shader阶段*/
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;  /*后续管线阶段需要等待该函数布局转换完成*/
    }
    else
        throw std::runtime_error("Unsupported layout transition!");
    /*1.记录命令*/
    beginSingleTimeCommands();
        vk::ImageSubresourceRange subresourceRange;
        subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor) /*设置转换图像格式受影响的图像范围*/
                        .setBaseMipLevel(0).setLevelCount(1)            /*设置子资源mipmap起始索引和mipmap数组数量*/
                        .setBaseArrayLayer(0).setLayerCount(1);         /*设置子资源纹理数组起始索引与数组数量*/
        vk::ImageMemoryBarrier barrier = {};
        barrier.setOldLayout(oldLayout)                         /*设置旧图像布局*/
               .setNewLayout(newLayout)                         /*设置新图像布局*/
               .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)  /*不转移队列族所有权*/
               .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)  
               .setImage(image)                                 /*设置布局变换的图像对象*/
               .setSubresourceRange(subresourceRange)           /*设置布局变换受影响的区域*/
               .setSrcAccessMask(srcAccess)     /*设置布局变换依赖的上一资源操作类型*/
               .setDstAccessMask(dstAccess);    /*设置哪一操作需要等待该屏障（布局变换）完成*/
        m_cmdBuffer.pipelineBarrier(srcStage, dstStage, vk::DependencyFlags(0), nullptr, nullptr, barrier);
    endSingleTimeCommands();

    
    /*2.提交命令*/
    submit();
    /*3.复位栅栏并重置命令缓冲*/
    VkBase::self().device.resetFences(m_fence);
    m_cmdBuffer.reset();
}






void Commander::beginSingleTimeCommands()
{
    vk::CommandBufferBeginInfo cbBeginInfo = {};
    cbBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)  /*设置命令缓冲使用方法为提交一次*/
               .setPInheritanceInfo(nullptr);
    m_cmdBuffer.begin(cbBeginInfo);   /*开始录制命令...*/
}
void Commander::endSingleTimeCommands()
{
    m_cmdBuffer.end();
}
void Commander::submit()
{
    /*提交命令缓冲*/
    vk::SubmitInfo submitInfo = {};
    submitInfo.setCommandBuffers(m_cmdBuffer);    /*无需等待指定阶段或fencce/信号量*/
    VkBase::self().graphicsQueue.submit(submitInfo, m_fence);
    if(VkBase::self().device.waitForFences(m_fence, false, std::numeric_limits<uint64_t>::max())!=vk::Result::eSuccess)
        std::cout << "Waiting for copy buffer cmd fence timeout!" << std::endl;
}


    
}