#pragma once

#include <vector>

#include "vulkan/vulkan.hpp"

namespace vulkan2d{

struct SurfaceInfo{
   vk::SurfaceFormatKHR            format;
   vk::Extent2D                    extent;
   vk::PresentModeKHR              presentMode;
   uint32_t                        minImageCount;
   vk::SurfaceTransformFlagBitsKHR preTransform;
};

struct SwapchainSupportInfo{
    vk::SurfaceCapabilitiesKHR        capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR>   presentModes;
};

struct Image{
    vk::Image image;
    vk::ImageView view;
};

class Swapchain{
public:
    vk::SurfaceKHR               surface;
    vk::SwapchainKHR             swapchain;
    std::vector<Image>           images;
    std::vector<vk::Framebuffer> framebuffers;

    Swapchain(vk::SurfaceKHR surface_);
    ~Swapchain();
    vk::SwapchainKHR createSwapchain();
    void initFramebuffers();
    void getOldSwapchain(vk::SwapchainKHR oldSwapchain) { m_oldSwapchain = oldSwapchain; }

    vk::SurfaceFormatKHR getFormat() { return m_surfaceProperty.format; }
    vk::Extent2D getExtent() { return m_surfaceProperty.extent; }

private:
    vk::SwapchainKHR     m_oldSwapchain;
    SurfaceInfo          m_surfaceProperty;
    SwapchainSupportInfo m_swapchainSupportInfo;

    bool getSwapchainSupportInfo(vk::SurfaceKHR surface_);
    void setSurfaceProperty(vk::SurfaceFormatKHR format={vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear}, vk::PresentModeKHR presentMode=vk::PresentModeKHR::eFifo, uint32_t default_width=800, uint32_t default_height=600);
    vk::SurfaceFormatKHR querySurfaceFormat(vk::SurfaceFormatKHR format={vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear});
    vk::PresentModeKHR querySurfacePresentMode(vk::PresentModeKHR presentMode=vk::PresentModeKHR::eFifo);
    vk::Extent2D querySurfaceExtent(uint32_t default_width=800, uint32_t default_height=600);
    void createImageAndViews();
    void setFramebuffers();

};





}