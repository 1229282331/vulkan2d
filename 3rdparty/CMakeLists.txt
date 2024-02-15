
if(NOT TARGET glfw)
    option(GLFW_BUILD_EXAMPLES "" OFF)
    option(GLFW_BUILD_TESTS "" OFF)
    option(GLFW_BUILD_DOCS "" OFF)
    option(GLFW_INSTALL "" OFF)
    add_subdirectory(glfw)
endif()

if(NOT TARGET glm)
    option(GLM_BUILD_TESTS "" OFF)
    option(GLM_BUILD_INSTALL "" OFF)
    add_subdirectory(glm-1.0.0)
endif()