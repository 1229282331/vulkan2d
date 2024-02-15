#pragma once

#include <string>
#include <vector>

#include "vulkan/vulkan.hpp"
#include "utils.hpp"
#include "app.hpp"

namespace vulkan2d{

class VkBase{
public:
    friend void initial(const std::vector<const char*>& extensions_);

public:
    static VkBase& self() { return *m_self_instance; }
    static void init(const std::vector<const char*>& extensions_);
    static void destroy();

    vk::Instance               instance;
    vk::DebugUtilsMessengerEXT debugMessenger;
    vk::PhysicalDevice         physicalDevice;
    std::vector<const char*>   extensions;
    std::vector<const char*>   layers;


private:
    static VkBase*                   m_self_instance;  /*单例*/
    static bool                      m_enableValidationLayer;

    VkBase(const std::vector<const char*>& extensions_);
    ~VkBase();


    vk::Instance createInstance();
    vk::DispatchLoaderDynamic loadInstanceDynamicLoader();
    vk::DebugUtilsMessengerEXT createDebugMessenger();
    vk::PhysicalDevice pickPhysicalDevice();


};

}