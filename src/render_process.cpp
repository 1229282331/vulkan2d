#include "render_process.hpp"
#include "vkBase.hpp"



namespace vulkan2d{

RenderProcess::RenderProcess()
{
    /*创建管线布局*/
    pipelineLayout = createLayout();
    if(!pipelineLayout)
        throw std::runtime_error("[ Pipeline-Layout ]: Can't create pipeline layout!");
    /*创建渲染流程*/
    renderPass = createRenderPass();
    if(!renderPass)
        throw std::runtime_error("[ RenderPass ]: Can't create renderPass!");

    graphicsPipeline_triangle = nullptr;
    graphicsPipeline_line = nullptr;
    
}

RenderProcess::~RenderProcess()
{
    auto& base_instance = VkBase::self();
    
    base_instance.device.destroyRenderPass(renderPass);
    base_instance.device.destroyPipelineLayout(pipelineLayout);
}

vk::PipelineLayout RenderProcess::createLayout()
{
    vk::PipelineLayoutCreateInfo createInfo = {};
    createInfo.setSetLayouts(VkBase::self().shader->getDescriptorSetLayouts())          /*设置管线布局*/
              .setPushConstantRangeCount(0)     /*设置常量值个数*/
              .setPPushConstantRanges(nullptr); /*设置常量值*/
    
    return VkBase::self().device.createPipelineLayout(createInfo);
}

vk::RenderPass RenderProcess::createRenderPass()
{
    auto& base_instance = VkBase::self();
    /*0.设置渲染流程子流程依赖*/
    vk::SubpassDependency dependency = {};
    dependency.setSrcSubpass(vk::SubpassExternal)                                                                       /*设置依赖的上一个子流程的索引（隐藏子流程）*/
              .setDstSubpass(0)                                                                                         /*设置当前子流程的索引*/
              .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)                                       /*指定依赖的上一子流程等待到达的管线阶段*/
              .setSrcAccessMask(vk::AccessFlagBits::eNone)                                                              /*指定依赖的上一子流程进行的操作类型*/
              .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)                                       /*指定当前子流程等待到达的管线阶段*/
              .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead|vk::AccessFlagBits::eColorAttachmentWrite);    /*指定当前子流程进行的操作类型*/

    /*1.设置渲染前后frambuffer中附件的描述信息*/
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.setFormat(base_instance.swapchain->getFormat().format)  /*设置framebuffer图像格式*/
                   .setSamples(vk::SampleCountFlagBits::e1)                 /*设置采样率*/
                   .setLoadOp(vk::AttachmentLoadOp::eClear)                 /*设置渲染前颜色和深度缓冲的加载（读取）方式为清空读取*/
                   .setStoreOp(vk::AttachmentStoreOp::eStore)               /*设置渲染后颜色和深度缓冲的存储（写入）方式*/
                   .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)       /*设置渲染前模板缓冲的加载方式（不使用）*/
                   .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)     /*设置渲染后模板缓冲的存储方式（不使用）*/
                   .setInitialLayout(vk::ImageLayout::eUndefined)           /*设置渲染前的图像布局为任意*/
                   .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);        /*设置选然后的图像布局为显示布局*/
    
    /*2.设置子流程及其引用的附件*/
    vk::AttachmentReference attachmentReference = {};
    attachmentReference.setAttachment(0)                                    /*设置引用的附件索引值（同样对应shader中的layout(location = 0) out vec4 outColor）*/
                       .setLayout(vk::ImageLayout::eColorAttachmentOptimal);/*设置进入子流程后转换到的目标布局*/
    vk::SubpassDescription subpass = {};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)          /*设置管线类型为图形管线*/
           .setColorAttachmentCount(1)                                      /*设置颜色附件数量*/
           .setPColorAttachments(&attachmentReference);                     /*设置该子流程引用的颜色附件引用*/

    /*3.创建渲染流程*/
    vk::RenderPassCreateInfo createInfo = {};
    createInfo.setAttachmentCount(1)                /*设置渲染流程中的附件个数*/
              .setPAttachments(&colorAttachment)    /*设置附件数组*/
              .setSubpassCount(1)                   /*设置子流程个数*/
              .setPSubpasses(&subpass)              /*设置子流程数组*/
              .setDependencies(dependency);         /*设置渲染流程子流程依赖*/

    return base_instance.device.createRenderPass(createInfo);
}

vk::Pipeline RenderProcess::createGraphicsPipeline(const Shader& shader, vk::PrimitiveTopology topology) 
{
    /* [可编程部分]: shader */
    /*0.设置shader在管线中的对应信息*/
    std::array<vk::PipelineShaderStageCreateInfo,2> shaderStageCreateInfos;
    shaderStageCreateInfos[0].setPName("main")                              /*设置着色器程序入口函数*/
                             .setModule(shader.getVertexShaderModule())     /*设置顶点着色器*/
                             .setStage(vk::ShaderStageFlagBits::eVertex)    /*设置顶点着色器位于管线顶点着色阶段*/
                             .setPSpecializationInfo(nullptr);              /*设置CPU->GPU传递的特殊值*/
    shaderStageCreateInfos[1].setPName("main")                              /*设置着色器程序入口函数*/
                             .setModule(shader.getFragmentShaderModule())   /*设置片段着色器*/
                             .setStage(vk::ShaderStageFlagBits::eFragment)  /*设置顶点着色器位于管线顶点着色阶段*/
                             .setPSpecializationInfo(nullptr);              /*设置CPU->GPU传递的特殊值*/
                             

    /* [固定部分]: 设置管线固定部分的参数 */
    /*1.顶点输入*/
    vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo = {};   
    auto bindingDescrptions = Vertex::getBindingDescriptions();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputStateInfo.setVertexBindingDescriptions(bindingDescrptions)     /*设置绑定描述体数组，设置数据间距和组织方式（逐顶点/逐实例）*/
                        .setVertexAttributeDescriptions(attributeDescriptions);  /*设置属性描述体数组，将属性传递给顶点着色器中的变量*/

    /*2.输入装配*/
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo = {};
    inputAssemblyStateInfo.setTopology(topology)                /*设置渲染管线使用的图元拓扑*/
                          .setPrimitiveRestartEnable(false);    /*是否启用图元重启（特殊index之后重置index=0）*/

    /*3.视口和裁剪*/
    vk::Viewport viewport = {};
    viewport.setX(0.0).setY(0.0)
            .setWidth(float(VkBase::self().swapchain->getExtent().width)).setHeight(float(VkBase::self().swapchain->getExtent().height))
            .setMinDepth(0.0).setMaxDepth(1.0);
    vk::Rect2D scissor = {};
    scissor.setOffset(vk::Offset2D(0, 0))
           .setExtent(VkBase::self().swapchain->getExtent());
    vk::PipelineViewportStateCreateInfo viewportStateInfo = {};
    viewportStateInfo.setViewportCount(1)       /*设置视口数量*/
                     .setPViewports(&viewport)  /*设置视口数组*/
                     .setScissorCount(1)        /*设置裁剪数量*/
                     .setPScissors(&scissor);   /*设置裁剪数组*/

    /*4.光栅化*/
    vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo = {};
    rasterizationStateInfo.setFrontFace(vk::FrontFace::eCounterClockwise)      /*设置多边形正面索引方向（索引顺时针/逆时针）*/
                          .setCullMode(vk::CullModeFlagBits::eBack)     /*设置剔除模式为背面剔除*/
                          .setPolygonMode(vk::PolygonMode::eFill)       /*设置多边形在片段着色器着色方式为填充模式*/
                          .setLineWidth(1.0)                            /*设置光栅化后线段宽度（像素）*/
                          .setRasterizerDiscardEnable(false)            /*是否丢弃光栅化过程*/
                          .setDepthClampEnable(false)                   /*是否进行深度截断*/
                          .setDepthBiasEnable(false);                   /*是否设置深度附加值（自定义算法所需）*/

    /*5.多重采样*/
    vk::PipelineMultisampleStateCreateInfo multisampleStateInfo = {};
    multisampleStateInfo.setSampleShadingEnable(false)
                        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                        .setMinSampleShading(1.0)
                        .setPSampleMask(nullptr)
                        .setAlphaToCoverageEnable(false)
                        .setAlphaToOneEnable(false);

    /*6.深度和模板测试*/
     
    /*7.颜色混合*/
    // finalRGB = 1*newRGB + 0*oldRGB
    // finalA   = 1*newA + 0*oldA
    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState = {};   /*设置绑定的帧缓冲颜色混合*/
    colorBlendAttachmentState.setBlendEnable(true)
                             .setSrcColorBlendFactor(vk::BlendFactor::eOne)     /*设置新缓冲区rgb值系数*/
                             .setDstColorBlendFactor(vk::BlendFactor::eZero)    /*设置旧缓冲区rgb值系数*/
                             .setColorBlendOp(vk::BlendOp::eAdd)                /*设置混合操作*/
                             .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)     /*设置新缓冲区alpha值系数*/
                             .setDstAlphaBlendFactor(vk::BlendFactor::eZero)    /*设置新缓冲区alpha值系数*/
                             .setAlphaBlendOp(vk::BlendOp::eAdd)                /*设置混合操作*/
                             .setColorWriteMask(vk::ColorComponentFlagBits::eR| /*设置颜色混合写入的颜色通道*/
                                                vk::ColorComponentFlagBits::eG|
                                                vk::ColorComponentFlagBits::eB|
                                                vk::ColorComponentFlagBits::eA);
    vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo = {}; /*设置全局帧缓冲颜色混合*/
    colorBlendStateInfo.setLogicOpEnable(false)                         /*禁止位运算方式混合颜色，使用第一类混合方式*/
                       .setAttachmentCount(1)                           /*设置帧缓冲（颜色附件）个数*/
                       .setPAttachments(&colorBlendAttachmentState);    /*每个帧缓冲的颜色混合设置数组*/

    /*8.动态参数*/
    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
        vk::DynamicState::eLineWidth
    };
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.setDynamicStateCount(dynamicStates.size())
                    .setDynamicStates(dynamicStates);
    
    //创建渲染管线
    vk::GraphicsPipelineCreateInfo createInfo = {};
    createInfo.setStageCount(shaderStageCreateInfos.size())
              .setStages(shaderStageCreateInfos)                    /*设置shader管线阶段信息*/
              .setPVertexInputState(&vertexInputStateInfo)          /*设置顶点输入信息*/
              .setPInputAssemblyState(&inputAssemblyStateInfo)      /*设置输入装配信息*/
              .setPViewportState(&viewportStateInfo)                /*设置视口和裁剪信息*/
              .setPRasterizationState(&rasterizationStateInfo)      /*设置光栅化过程信息*/
              .setPMultisampleState(&multisampleStateInfo)          /*设置多重采样信息*/
              .setPDepthStencilState(nullptr)                       /*设置深度和模板信息*/
              .setPColorBlendState(&colorBlendStateInfo)            /*设置渲染前后颜色混合信息*/
              .setPDynamicState(&dynamicStateInfo)                  /*设置渲染过程可变参数信息*/
              .setLayout(pipelineLayout)                            /*设置管线布局（常量）*/
              .setRenderPass(renderPass)                            /*设置渲染流程*/
              .setSubpass(0)                                        /*设置渲染子流程索引index*/
              .setBasePipelineHandle(nullptr)                       /*设置基类管线句柄*/
              .setBasePipelineIndex(-1);                            /*或设置基类管线索引*/
    auto res = VkBase::self().device.createGraphicsPipeline(nullptr, createInfo);
    if(res.result!=vk::Result::eSuccess)
    {
        std::cout << "Failed to create graphics pipeline!" << std::endl;
        abort();
    }
    return res.value;
}





}