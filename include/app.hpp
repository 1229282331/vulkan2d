#pragma once

#include "vkBase.hpp"
#include "window.hpp"
#include "myMath.hpp"


const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
}; 
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

namespace vulkan2d{

void initial();

void run();

void cleanup();



}