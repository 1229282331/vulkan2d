#pragma once

#include <array>
#include "glm/glm.hpp"
#include "vulkan/vulkan.hpp"



struct Vertex{
    glm::vec2 pos;
    glm::vec3 color;

    static std::array<vk::VertexInputBindingDescription,1> getBindingDescriptions()
    {
        std::array<vk::VertexInputBindingDescription,1> bindingDescriptions = {};
        bindingDescriptions[0].setBinding(0)                                /*设置绑定描述符的索引*/
                              .setStride(sizeof(Vertex))                    /*设置顶点输入步长*/
                              .setInputRate(vk::VertexInputRate::eVertex);  /*设置顶点输入速率：逐顶点输入*/
        return bindingDescriptions;
    }
    static std::array<vk::VertexInputAttributeDescription,2> getAttributeDescriptions()
    {
        std::array<vk::VertexInputAttributeDescription,2> attributeDescriptions = {};
        /*设置Vertex.pos属性的顶点输入描述*/
        attributeDescriptions[0].setBinding(0)                          /*设置当前属性描述符所属的绑定描述符的索引（即当前属性的来源）*/
                                .setFormat(vk::Format::eR32G32Sfloat)   /*设置该属性格式（颜色/大小）：vec2*/
                                .setLocation(0)                         /*设置对应顶点着色器中的location值*/
                                .setOffset(offsetof(Vertex, pos));      /*设置该属性偏移量*/
        /*设置Vertex.color属性的顶点输入描述*/
        attributeDescriptions[1].setBinding(0)                          /*设置当前属性描述符所属的绑定描述符的索引（即当前属性的来源）*/
                                .setFormat(vk::Format::eR32G32B32Sfloat)/*设置该属性格式（颜色/大小）：vec3*/
                                .setLocation(1)                         /*设置对应顶点着色器中的location值*/
                                .setOffset(offsetof(Vertex, color));    /*设置该属性偏移量*/
        return attributeDescriptions;
    }


};







