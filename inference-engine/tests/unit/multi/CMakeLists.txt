# Copyright (C) 2018-2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

set(TARGET_NAME ieMultiPluginUnitTests)

set(CI_BUILD_NUMBER "unittest")
addVersionDefines(${IE_MAIN_SOURCE_DIR}/src/multi_device/multi_device_plugin.cpp CI_BUILD_NUMBER)
add_definitions(-DMULTIUNITTEST)

addIeTargetTest(
        NAME ${TARGET_NAME}
        ROOT ${CMAKE_CURRENT_SOURCE_DIR}
        ADDITIONAL_SOURCE_DIRS ${IE_MAIN_SOURCE_DIR}/src/multi_device ${IE_MAIN_SOURCE_DIR}/src/multi_device/utils
        INCLUDES
            ${IE_MAIN_SOURCE_DIR}/src/multi_device ${CMAKE_CURRENT_SOURCE_DIR}
        LINK_LIBRARIES
            inference_engine_lp_transformations
            ngraphFunctions
            inference_engine
            ngraph
            inference_engine_transformations
            unitTestUtils
        ADD_CPPLINT
        DEPENDENCIES
            template_extension
            mock_engine
            ngraphFunctions
        LABELS
            Multi
            Auto
)

set_ie_threading_interface_for(${TARGET_NAME})

set_target_properties(${TARGET_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ${ENABLE_LTO})
