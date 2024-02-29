#include "swapchain.hpp"
#include "vkBase.hpp"

namespace vulkan2d{

Swapchain::Swapchain(vk::SurfaceKHR surface_) : surface(surface_), m_oldSwapchain(nullptr)
{
    /*1.获取物理设备支持的surface属性*/
    if(!getSwapchainSupportInfo(surface))
        abort();
    /*2.设置指定的surface显示属性*/
    vk::SurfaceFormatKHR format = {vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear};
    setSurfaceProperty(format, vk::PresentModeKHR::eFifo, 800, 600);
    /*3.创建交换链*/
    swapchain = createSwapchain();
    if(!swapchain)
        throw std::runtime_error("[ swapchain ]: Can't create swapchain!");
    /*4.创建imageView*/
    createImageAndViews();
}

Swapchain::~Swapchain()
{
    for(auto& fb: framebuffers)
        VkBase::self().device.destroyFramebuffer(fb);
    for(auto& img : images)
        VkBase::self().device.destroyImageView(img.view);
    VkBase::self().device.destroySwapchainKHR(swapchain);
    VkBase::self().instance.destroySurfaceKHR(surface);
}

void Swapchain::initFramebuffers()
{
    setFramebuffers();
}

bool Swapchain::getSwapchainSupportInfo(vk::SurfaceKHR surface_)
{
    vk::PhysicalDevice physicalDevice_ = VkBase::self().physicalDevice;
    m_swapchainSupportInfo.capabilities = physicalDevice_.getSurfaceCapabilitiesKHR(surface_);
    m_swapchainSupportInfo.formats = physicalDevice_.getSurfaceFormatsKHR(surface_);
    m_swapchainSupportInfo.presentModes = physicalDevice_.getSurfacePresentModesKHR(surface_);

    if(m_swapchainSupportInfo.formats.empty() || m_swapchainSupportInfo.presentModes.empty())
    {
        throw std::runtime_error("[ Swapchain ]: The physical device dosen't have support formats/presentModes!");
        return false;
    }
        return true;

}

void Swapchain::setSurfaceProperty(vk::SurfaceFormatKHR format, vk::PresentModeKHR presentMode, uint32_t default_width, uint32_t default_height)
{
    m_surfaceProperty.format = querySurfaceFormat(format);
    m_surfaceProperty.extent = querySurfaceExtent(default_width, default_height);
    m_surfaceProperty.presentMode = querySurfacePresentMode(presentMode);
    m_surfaceProperty.minImageCount = std::clamp(m_swapchainSupportInfo.capabilities.minImageCount+1, 
                                                m_swapchainSupportInfo.capabilities.minImageCount, m_swapchainSupportInfo.capabilities.maxImageCount); 
    m_surfaceProperty.preTransform = m_swapchainSupportInfo.capabilities.currentTransform;
}

vk::SurfaceFormatKHR Swapchain::querySurfaceFormat(vk::SurfaceFormatKHR format)
{
    for(auto& e : m_swapchainSupportInfo.formats)
    {
        if(e.format==format.format && e.colorSpace==format.colorSpace)
            return e;
    }
    return m_swapchainSupportInfo.formats[0];
}

vk::PresentModeKHR Swapchain::querySurfacePresentMode(vk::PresentModeKHR presentMode)
{
    for(auto& e : m_swapchainSupportInfo.presentModes)
    {
        if(e == presentMode)
            return e;
    }
    return m_swapchainSupportInfo.presentModes[0];
}

vk::Extent2D Swapchain::querySurfaceExtent(uint32_t default_width, uint32_t default_height)
{
    if(m_swapchainSupportInfo.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return m_swapchainSupportInfo.capabilities.currentExtent;
    vk::Extent2D extent;
    extent.setWidth(std::clamp(default_width, m_swapchainSupportInfo.capabilities.minImageExtent.width, m_swapchainSupportInfo.capabilities.maxImageExtent.width))
          .setHeight(std::clamp(default_height, m_swapchainSupportInfo.capabilities.minImageExtent.height, m_swapchainSupportInfo.capabilities.maxImageExtent.height));

    return extent;
}

vk::SwapchainKHR Swapchain::createSwapchain()
{
    vk::SwapchainCreateInfoKHR createInfo = {};
    createInfo.setPNext(nullptr)
              .setClipped(true)
              .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
              .setImageColorSpace(m_surfaceProperty.format.colorSpace)
              .setImageFormat(m_surfaceProperty.format.format)
              .setImageExtent(m_surfaceProperty.extent)
              .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
              .setImageArrayLayers(1)   /*2D-monitor*/
              .setMinImageCount(m_surfaceProperty.minImageCount)
              .setSurface(surface)
              .setPresentMode(m_surfaceProperty.presentMode)
              .setPreTransform(m_surfaceProperty.preTransform)
              .setOldSwapchain(m_oldSwapchain);
    
    /*若图形和显示队列相同则无需设置共享image*/
    if(VkBase::self().queueFamilyIndex.graphicsIndex.value() == VkBase::self().queueFamilyIndex.presentIndex.value())
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    else
    {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        std::vector<uint32_t> queueIndices = {VkBase::self().queueFamilyIndex.graphicsIndex.value(), VkBase::self().queueFamilyIndex.presentIndex.value()};
        createInfo.setQueueFamilyIndices(queueIndices);
    }
    
    return VkBase::self().device.createSwapchainKHR(createInfo);
}

void Swapchain::createImageAndViews()
{
    std::vector<vk::Image> swapchainImages = VkBase::self().device.getSwapchainImagesKHR(swapchain);
    for(auto img : swapchainImages)
    {
        Image image = {};
        image.image = img;
        /*创建imageView*/
        vk::ImageSubresourceRange subresourceRange = {};    
        subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor) /*设置纹理贴图类型*/
                        .setLevelCount(1)                               /*设置mipmap数量*/
                        .setBaseMipLevel(0)                             /*设置mipmap起始索引*/
                        .setLayerCount(1)                               /*设置纹理数组数量*/    
                        .setBaseArrayLayer(0);                          /*设置纹理数组起始索引*/

        vk::ImageViewCreateInfo createInfo = {};
        createInfo.setPNext(nullptr)
                  .setFormat(m_surfaceProperty.format.format)   /*设置纹理图片格式*/
                  .setViewType(vk::ImageViewType::e2D)          /*设置纹理类型为2D纹理*/
                  .setComponents(vk::ComponentMapping{})        /*设置图像rgba通道映射为默认*/
                  .setSubresourceRange(subresourceRange)        /*设置纹理贴图细分子资源*/
                  .setImage(image.image);                       /*设置原始图像*/
        image.view = VkBase::self().device.createImageView(createInfo);
        this->images.push_back(image);
    }
}

void Swapchain::setFramebuffers()
{
    framebuffers.clear();
    for(auto& view: images)
    {
        vk::FramebufferCreateInfo createInfo = {};
        createInfo.setRenderPass(VkBase::self().renderProcess->renderPass)  /*设置兼容的渲染流程（该frambuffer对所有兼容的渲染流程均通用）*/
                  .setAttachments(view.view)                                /*设置该framebuffer绑定的附件数组*/
                  .setWidth(m_surfaceProperty.extent.width)                 /*设置每帧framebuffer的大小*/
                  .setHeight(m_surfaceProperty.extent.height)               
                  .setLayers(1);                                            /*设置图层数*/
        framebuffers.push_back(VkBase::self().device.createFramebuffer(createInfo));
    }
}

}