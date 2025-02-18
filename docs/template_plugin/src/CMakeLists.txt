# Copyright (C) 2018-2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

# [cmake:plugin]
set(TARGET_NAME "templatePlugin")

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

# adds a shared library with plugin
ie_add_plugin(NAME ${TARGET_NAME}
              DEVICE_NAME "TEMPLATE"
              SOURCES ${SOURCES} ${HEADERS}
              SKIP_INSTALL # ATTENTION: uncomment to install component
              VERSION_DEFINES_FOR template_plugin.cpp
              ADD_CLANG_FORMAT)

# Enable support of CC for the plugin
ie_mark_target_as_cc(${TARGET_NAME})

target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${IE_MAIN_TEMPLATE_PLUGIN_SOURCE_DIR}/include")

# link common Inference Engine libraries
target_link_libraries(${TARGET_NAME} PRIVATE
    IE::inference_engine
    IE::inference_engine_transformations
    ${NGRAPH_LIBRARIES})

# Link inference backend library to plugin. Here we use ngraph interpreter_backend as example
target_link_libraries(${TARGET_NAME} PRIVATE
    IE::interpreter_backend)

set_target_properties(${TARGET_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ${ENABLE_LTO})

# ATTENTION: uncomment to register a plugin in the plugins.xml file
# ie_register_plugins(MAIN_TARGET ${TARGET_NAME}
#                     POSSIBLE_PLUGINS ${TARGET_NAME})
# [cmake:plugin]

ov_install_static_lib(ngraph_backend ngraph)
ov_install_static_lib(interpreter_backend ngraph)
