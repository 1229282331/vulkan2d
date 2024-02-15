cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(vulkan2D VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")  # 对于C语言
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>") # 对于C++语言

include(CMakeDependentOption)

# add_definitions("-DNDEBUG")

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

set(VULKANLEARN_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_INSTALL_PREFIX "${VULKANLEARN_ROOT_DIR}/bin")
set(BINARY_ROOT_DIR "${CMAKE_INSTALL_PREFIX}/")
set(THIRD_PARTY_DIR "${VULKANLEARN_ROOT_DIR}/3rdparty")

add_subdirectory(3rdparty)

set(TARGET_NAME ${CMAKE_PROJECT_NAME})
# 设置VS中的启动项目
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/MP")
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET_NAME})
endif()

set(glfw_include ${THIRD_PARTY_DIR}/glfw/include)
set(glm_include ${THIRD_PARTY_DIR}/glm-1.0.0)

# 设置vulkan跨平台编译属性
set(vulkan_include ${THIRD_PARTY_DIR}/VulkanSDK/include)
if(WIN32)
    set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Win32/vulkan-1.lib)
    set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32/glslangValidator.exe)
    # add_compile_definitions("VULKANLEARN_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32")   # 添加预编译宏VULKANLEARN_VK_LAYER_PATH
elseif(UNIX)
    if(APPLE)
      set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/MacOS/libvulkan.1.dylib)
      set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS/glslangValidator)
    #   add_compile_definitions("VULKANLEARN_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS")
    #   add_compile_definitions("VULKANLEARN_VK_ICD_FILENAMES=${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS/MoltenVK_icd.json")
    else()
      set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Linux/libvulkan.so.1)
      set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux/glslangValidator)
    #   add_compile_definitions("VULKANLEARN=${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux")
    endif()
else()
  message(FATAL_ERROR "Unknown Platform")
endif()

# 设置项目源代码和头文件
file(GLOB_RECURSE HEADER_FILES "include/*.hpp")
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE TEST_FILES "test/*.cpp")

# 在VS中生成文件树
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCE_FILES} ${TEST_FILES} ${HEADER_FILES})


add_executable(${TARGET_NAME} main.cpp ${SOURCE_FILES} ${TEST_FILES} ${HEADER_FILES})
target_link_libraries(${TARGET_NAME} PUBLIC glm)
target_link_libraries(${TARGET_NAME} PUBLIC glfw)
target_link_libraries(${TARGET_NAME} PUBLIC ${vulkan_lib})
target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${glm_include}>)
target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${glfw_include}>)
target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${vulkan_include}>)
