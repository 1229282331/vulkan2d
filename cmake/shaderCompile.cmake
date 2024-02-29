set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shader/glsl)
file(GLOB_RECURSE SHADER_SOURCE_FILES RELATIVE ${SRC_DIR} 
                                        "${SRC_DIR}/*.vert"
                                        "${SRC_DIR}/*.frag"
)

set(DST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shader/generated)
if(WIN32)
    set(GLSLANG_BIN ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/VulkanSDK/bin/Win32/glslangValidator.exe)
elseif(UNIX)
    set(GLSLANG_BIN ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/VulkanSDK/bin/Linux/glslangValidator)
    execute_process(COMMAND chmod a+x ${GLSLANG_BIN})
endif()


foreach(shader ${SHADER_SOURCE_FILES})
    execute_process(COMMAND ${GLSLANG_BIN} -V ${SRC_DIR}/${shader} -o ${DST_DIR}/${shader}.spv)
endforeach()