#pragma once

#include <array>

#include "vulkan/vulkan.hpp"
#include "shader.hpp"

namespace vulkan2d{
    
class RenderProcess{
public:
    RenderProcess();
    ~RenderProcess();

    vk::Pipeline createGraphicsPipeline(const Shader& shader, vk::PrimitiveTopology topology);

    vk::PipelineLayout pipelineLayout;
    vk::RenderPass     renderPass;
    vk::Pipeline       graphicsPipeline_triangle;
    vk::Pipeline       graphicsPipeline_line;

private:
    vk::PipelineLayout createLayout();
    vk::RenderPass createRenderPass();



};






}
