// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "test_utils/cpu_test_utils.hpp"
#include "functional_test_utils/ov_tensor_utils.hpp"

#include "shared_test_classes/base/ov_subgraph.hpp"
#include "ngraph_functions/builders.hpp"
#include "ngraph_functions/utils/ngraph_helpers.hpp"

using namespace InferenceEngine;
using namespace CPUTestUtils;
using namespace ov::test;

namespace CPULayerTestsDefinitions {
using ROIAlignShapes = std::vector<InputShape>;

using ROIAlignSpecificParams =  std::tuple<
        int,                                                 // bin's column count
        int,                                                 // bin's row count
        float,                                               // scale for given region considering actual input size
        int,                                                 // pooling ratio
        std::string,                                         // pooling mode
        ROIAlignShapes
>;

using ROIAlignLayerTestParams = std::tuple<
        ROIAlignSpecificParams,
        ElementType,                    // Net precision
        LayerTestsUtils::TargetDevice   // Device name
>;

using ROIAlignLayerCPUTestParamsSet = std::tuple<
        CPULayerTestsDefinitions::ROIAlignLayerTestParams,
        CPUSpecificParams>;

class ROIAlignLayerCPUTest : public testing::WithParamInterface<ROIAlignLayerCPUTestParamsSet>,
                             public SubgraphBaseTest, public CPUTestsBase {
public:
    static std::string getTestCaseName(testing::TestParamInfo<ROIAlignLayerCPUTestParamsSet> obj) {
        CPULayerTestsDefinitions::ROIAlignLayerTestParams basicParamsSet;
        CPUSpecificParams cpuParams;
        std::tie(basicParamsSet, cpuParams) = obj.param;
        std::string td;
        ElementType netPrecision;
        ROIAlignSpecificParams roiPar;
        std::tie(roiPar, netPrecision, td) = basicParamsSet;

        int pooledH;
        int pooledW;
        float spatialScale;
        int samplingRatio;
        std::string mode;
        ROIAlignShapes inputShapes;
        std::tie(pooledH, pooledW, spatialScale, samplingRatio, mode, inputShapes) = roiPar;
        std::ostringstream result;

        result << netPrecision << "_IS=";
        for (const auto& shape : inputShapes) {
            result << CommonTestUtils::partialShape2str({ shape.first }) << "_";
        }
        result << "TS=";
        for (const auto& shape : inputShapes) {
            result << "(";
            for (const auto& targetShape : shape.second) {
                result << CommonTestUtils::vec2str(targetShape) << "_";
            }
            result << ")_";
        }

        result << "pooledH=" << pooledH << "_";
        result << "pooledW=" << pooledW << "_";
        result << "spatialScale=" << spatialScale << "_";
        result << "samplingRatio=" << samplingRatio << "_";
        result << mode << "_";
        result << CPUTestsBase::getTestCaseName(cpuParams);

        return result.str();
    }
protected:
    void generate_inputs(const std::vector<ngraph::Shape>& targetInputStaticShapes) override {
        inputs.clear();
        const auto& funcInputs = function->inputs();

        ov::runtime::Tensor data_tensor;
        const auto& dataPrecision = funcInputs[0].get_element_type();
        const auto& dataShape = targetInputStaticShapes.front();
        data_tensor = ov::test::utils::create_and_fill_tensor(dataPrecision, dataShape, 10, 0, 1000);

        const auto& coordsET = funcInputs[1].get_element_type();
        auto coordsTensor = ov::runtime::Tensor{ coordsET, targetInputStaticShapes[1] };
        if (coordsET == ElementType::f32) {
            auto coordsTensorData = static_cast<float*>(coordsTensor.data());
            for (size_t i = 0; i < coordsTensor.get_size(); i += 4) {
                coordsTensorData[i] = 1.f;
                coordsTensorData[i] = 1.f;
                coordsTensorData[i] = 19.f;
                coordsTensorData[i] = 19.f;
            }
        } else if (coordsET == ElementType::bf16) {
            auto coordsTensorData = static_cast<std::int16_t*>(coordsTensor.data());
            for (size_t i = 0; i < coordsTensor.get_size(); i += 4) {
                coordsTensorData[i] = static_cast<std::int16_t>(ngraph::bfloat16(1.f).to_bits());
                coordsTensorData[i] = static_cast<std::int16_t>(ngraph::bfloat16(1.f).to_bits());
                coordsTensorData[i] = static_cast<std::int16_t>(ngraph::bfloat16(19.f).to_bits());
                coordsTensorData[i] = static_cast<std::int16_t>(ngraph::bfloat16(19.f).to_bits());
            }
        } else {
            IE_THROW() << "roi align. Unsupported precision: " << coordsET;
        }

        auto roisIdxTensor = ov::runtime::Tensor{ funcInputs[2].get_element_type(), targetInputStaticShapes[2] };
        auto roisIdxTensorData = static_cast<std::int32_t*>(roisIdxTensor.data());
        if (roisIdxTensor.get_size() == 1) {
            roisIdxTensorData[0] = 1;
        } else if (roisIdxTensor.get_size() == 2) {
            roisIdxTensorData[0] = 0;
            roisIdxTensorData[1] = 1;
        } else {
            IE_THROW() << "Unexpected roiIdx size: " << roisIdxTensor.get_size();
        }

        inputs.insert({ funcInputs[0].get_node_shared_ptr(), data_tensor });
        inputs.insert({ funcInputs[1].get_node_shared_ptr(), coordsTensor });
        inputs.insert({ funcInputs[2].get_node_shared_ptr(), roisIdxTensor });
    }

    void SetUp() override {
        CPULayerTestsDefinitions::ROIAlignLayerTestParams basicParamsSet;
        CPUSpecificParams cpuParams;
        std::tie(basicParamsSet, cpuParams) = this->GetParam();
        std::tie(inFmts, outFmts, priority, selectedType) = cpuParams;

        CPULayerTestsDefinitions::ROIAlignSpecificParams roiAlignParams;
        ElementType inputPrecision;
        std::tie(roiAlignParams, inputPrecision, targetDevice) = basicParamsSet;

        int pooledH;
        int pooledW;
        float spatialScale;
        int samplingRatio;
        std::string mode;
        ROIAlignShapes inputShapes;
        std::tie(pooledH, pooledW, spatialScale, samplingRatio, mode, inputShapes) = roiAlignParams;

        init_input_shapes(inputShapes);

        auto float_params = ngraph::builder::makeDynamicParams(inputPrecision, { inputDynamicShapes[0], inputDynamicShapes[1] });
        auto int_params = ngraph::builder::makeDynamicParams(ngraph::element::i32, { inputDynamicShapes[2] });

        auto roialign = std::make_shared<ngraph::opset3::ROIAlign>(float_params[0], float_params[1], int_params[0], pooledH, pooledW,
                                                                   samplingRatio, spatialScale, mode);

        selectedType = makeSelectedTypeStr("unknown", inputPrecision);
        if (inputPrecision == ElementType::bf16) {
            rel_threshold = 1e-2;
        }

        ngraph::ParameterVector params{ float_params[0], float_params[1], int_params[0] };
        function = makeNgraphFunction(inputPrecision, params, roialign, "ROIAlign");
    }
};

TEST_P(ROIAlignLayerCPUTest, CompareWithRefs) {
    SKIP_IF_CURRENT_TEST_IS_DISABLED()
    run();
    CheckPluginRelatedResults(executableNetwork, "ROIAlign");
}

namespace {

/* CPU PARAMS */
std::vector<CPUSpecificParams> filterCPUInfoForDevice() {
    std::vector<CPUSpecificParams> resCPUParams;
    resCPUParams.push_back(CPUSpecificParams{{nchw, nc, x}, {nchw}, {}, {}});
    resCPUParams.push_back(CPUSpecificParams{{nhwc, nc, x}, {nhwc}, {}, {}});
    if (with_cpu_x86_avx512f()) {
        resCPUParams.push_back(CPUSpecificParams{{nChw16c, nc, x}, {nChw16c}, {}, {}});
    } else if (with_cpu_x86_avx2() || with_cpu_x86_sse42()) {
        resCPUParams.push_back(CPUSpecificParams{{nChw8c, nc, x}, {nChw8c}, {}, {}});
    }
    return resCPUParams;
}

const std::vector<ElementType> netPrecisions = {
        ElementType::f32,
        ElementType::bf16,
};

const std::vector<int> spatialBinXVector = { 2 };

const std::vector<int> spatialBinYVector = { 2 };

const std::vector<float> spatialScaleVector = { 1.0f };

const std::vector<int> poolingRatioVector = { 7 };

const std::vector<std::string> modeVector = {
        "avg",
        "max"
};

const std::vector<ROIAlignShapes> inputShapeVector = {
    ROIAlignShapes{{{}, {{ 2, 18, 20, 20 }}}, {{}, {{2, 4}}}, {{}, {{2}}}},
    ROIAlignShapes{{{}, {{ 2, 4, 20, 20 }}}, {{}, {{2, 4}}}, {{}, {{2}}}},
    ROIAlignShapes{{{}, {{ 2, 4, 20, 40 }}}, {{}, {{2, 4}}}, {{}, {{2}}}},
    ROIAlignShapes{{{}, {{ 10, 1, 20, 20 }}}, {{}, {{2, 4}}}, {{}, {{2}}}},
    ROIAlignShapes{{{}, {{ 2, 18, 20, 20 }}}, {{}, {{1, 4}}}, {{}, {{1}}}},
    ROIAlignShapes{{{}, {{ 2, 4, 20, 20 }}}, {{}, {{1, 4}}}, {{}, {{1}}}},
    ROIAlignShapes{{{}, {{ 2, 4, 20, 40 }}}, {{}, {{1, 4}}}, {{}, {{1}}}},
    ROIAlignShapes{{{}, {{ 10, 1, 20, 20 }}}, {{}, {{1, 4}}}, {{}, {{1}}}},
    ROIAlignShapes{
        {{-1, -1, -1, -1}, {{ 10, 1, 20, 20 }, { 2, 4, 20, 20 }, { 2, 18, 20, 20 }}},
        {{-1, 4}, {{1, 4}, {2, 4}, {1, 4}}},
        {{-1}, {{1}, {2}, {1}}}
    },
    ROIAlignShapes{
        {{{2, 10}, { 1, 5 }, -1, -1}, {{ 2, 1, 20, 20 }, { 10, 5, 30, 20 }, { 4, 4, 40, 40 }}},
        {{-1, 4}, {{2, 4}, {2, 4}, {1, 4}}},
        {{-1}, {{2}, {2}, {1}}}
    },
    ROIAlignShapes{
        {{{2, 10}, {1, 18}, {10, 30}, {15, 25}}, {{ 10, 1, 10, 15 }, { 2, 4, 20, 20 }, { 7, 18, 30, 25 }}},
        {{{1, 2}, 4}, {{1, 4}, {2, 4}, {1, 4}}},
        {{{1, 2}}, {{1}, {2}, {1}}}
    },
};

const auto roiAlignParams = ::testing::Combine(
        ::testing::ValuesIn(spatialBinXVector),       // bin's column count
        ::testing::ValuesIn(spatialBinYVector),       // bin's row count
        ::testing::ValuesIn(spatialScaleVector),      // scale for given region considering actual input size
        ::testing::ValuesIn(poolingRatioVector),      // pooling ratio for bin
        ::testing::ValuesIn(modeVector),              // pooling mode
        ::testing::ValuesIn(inputShapeVector)         // feature map shape
);

INSTANTIATE_TEST_SUITE_P(smoke_ROIAlignLayoutTest, ROIAlignLayerCPUTest,
        ::testing::Combine(
                ::testing::Combine(
                        roiAlignParams,
                        ::testing::ValuesIn(netPrecisions),
                        ::testing::Values(CommonTestUtils::DEVICE_CPU)),
                ::testing::ValuesIn(filterCPUInfoForDevice())),
                ROIAlignLayerCPUTest::getTestCaseName);
} // namespace
} // namespace CPULayerTestsDefinitions
