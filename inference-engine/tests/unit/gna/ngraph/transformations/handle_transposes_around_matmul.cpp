// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>

#include "transformations/handle_transposes_around_matmul.hpp"

#include "common_test_utils/ngraph_test_utils.hpp"
#include <ngraph/function.hpp>
#include <ngraph/opsets/opset7.hpp>
#include <ngraph/pass/manager.hpp>
#include <transformations/init_node_info.hpp>
#include <numeric>

namespace handle_transpose_before_matmul {

std::shared_ptr<ngraph::Function> CreateTransposeMatmulFunction(const ngraph::Shape& input_shape,
    const ngraph::Shape& reshape_shape, const ngraph::Shape& matmul_shape, bool create_reshape_after_transpose) {
    auto input_params = std::make_shared<ngraph::opset7::Parameter>(ngraph::element::i64, input_shape);

    auto new_shape_const = ngraph::opset7::Constant::create(ngraph::element::i64, ngraph::Shape{reshape_shape.size()}, reshape_shape);
    auto reshape = std::make_shared<ngraph::opset7::Reshape>(input_params, new_shape_const, false);

    auto transpose_order = ngraph::opset7::Constant::create(ngraph::element::i64, ngraph::Shape{2}, {1, 0});
    auto transpose = std::make_shared<ngraph::opset7::Transpose>(reshape, transpose_order);

    std::vector<size_t> data(ngraph::shape_size(matmul_shape));
    std::iota(std::begin(data), std::end(data), 1);
    auto constant = ngraph::opset7::Constant::create(ngraph::element::i64, matmul_shape, data);
    std::shared_ptr<ngraph::opset7::MatMul> matmul;
    if (create_reshape_after_transpose) {
        auto reshape_after_transpose_const = ngraph::opset7::Constant::create(ngraph::element::i64,
            ngraph::Shape{reshape_shape.size()}, reshape_shape);
        auto reshape_after_transpose = std::make_shared<ngraph::opset7::Reshape>(transpose, reshape_after_transpose_const, false);
        matmul = std::make_shared<ngraph::opset7::MatMul>(reshape_after_transpose, constant);
    } else {
        matmul = std::make_shared<ngraph::opset7::MatMul>(transpose, constant);
    }

    auto result = std::make_shared<ngraph::opset7::Result>(matmul);
    return std::make_shared<ngraph::Function>(ngraph::ResultVector{result}, ngraph::ParameterVector{input_params});
}

std::shared_ptr<ngraph::Function> CreateMatmulFunction(const ngraph::Shape& input_shape,
    const ngraph::Shape& reshape_shape, const ngraph::Shape& matmul_shape, bool create_reshape_instead_of_transpose) {
    auto input_params = std::make_shared<ngraph::opset7::Parameter>(ngraph::element::i64, input_shape);

    std::shared_ptr<ngraph::opset7::Reshape> reshape;
    auto const_shape = ngraph::opset7::Constant::create(ngraph::element::i64, ngraph::Shape{reshape_shape.size()}, reshape_shape);
    if (create_reshape_instead_of_transpose) {
        auto new_reshape = std::make_shared<ngraph::opset7::Reshape>(input_params, const_shape, false);
        auto new_shape_after_transpose = ngraph::opset7::Constant::create(ngraph::element::i64,
            ngraph::Shape{reshape_shape.size()}, {reshape_shape[1], reshape_shape[0]});
        reshape = std::make_shared<ngraph::opset7::Reshape>(new_reshape, new_shape_after_transpose, false);
    } else {
        reshape = std::make_shared<ngraph::opset7::Reshape>(input_params, const_shape, false);
    }

    std::vector<size_t> data(ngraph::shape_size(matmul_shape));
    std::iota(std::begin(data), std::end(data), 1);
    auto constant = ngraph::opset7::Constant::create(ngraph::element::i64, matmul_shape, data);
    auto matmul = std::make_shared<ngraph::opset7::MatMul>(reshape, constant);

    auto result = std::make_shared<ngraph::opset7::Result>(matmul);
    return std::make_shared<ngraph::Function>(ngraph::ResultVector{result}, ngraph::ParameterVector{input_params});
}

} // namespace handle_transpose_before_matmul

namespace handle_transpose_after_matmul {

std::shared_ptr<ngraph::Function> CreateMatmulTransposeFunction(
    const ngraph::Shape& input_shape,
    const ngraph::Shape& matmul_shape,
    const ngraph::Shape& reshape_shape,
    bool create_reshape_before_transpose,
    bool enable_last_reshape,
    bool enable_add,
    bool matmul_on_left_side,
    bool enable_fq) {
    auto input_params = std::make_shared<ngraph::opset7::Parameter>(ngraph::element::i64, input_shape);

    std::vector<size_t> data(ngraph::shape_size(matmul_shape));
    std::iota(std::begin(data), std::end(data), 1);
    auto matmul_constant = ngraph::opset7::Constant::create(ngraph::element::i64, matmul_shape, data);
    std::shared_ptr<ngraph::Node> node = std::make_shared<ngraph::opset7::MatMul>(input_params, matmul_constant);
    const auto matmul_output_shape = node->get_output_shape(0);
    if (enable_add) {
        auto add_const = ngraph::opset7::Constant::create(ngraph::element::i64, matmul_output_shape, {1});
        if (matmul_on_left_side) {
            node = std::make_shared<ngraph::opset7::Add>(add_const, node);
        } else {
            node = std::make_shared<ngraph::opset7::Add>(node, add_const);
        }
    }

    if (enable_fq) {
        node = std::make_shared<ngraph::opset7::FakeQuantize>(
            node,
            ngraph::opset7::Constant::create(ngraph::element::f32, {1}, {-0.1}),
            ngraph::opset7::Constant::create(ngraph::element::f32, {1}, {0.1}),
            ngraph::opset7::Constant::create(ngraph::element::f32, {1}, {-0.1}),
            ngraph::opset7::Constant::create(ngraph::element::f32, {1}, {0.1}),
            255);
    }

    if (create_reshape_before_transpose) {
        auto matmul_output_shape = node->get_output_shape(0);
        std::swap(matmul_output_shape[0], matmul_output_shape[1]);
        auto reshape_before_transpose_const = ngraph::opset7::Constant::create(ngraph::element::i64,
            ngraph::Shape{matmul_output_shape.size()}, matmul_output_shape);
        node = std::make_shared<ngraph::opset7::Reshape>(node, reshape_before_transpose_const, false);
    }

    auto transpose_order = ngraph::opset7::Constant::create(ngraph::element::i64, ngraph::Shape{2}, {1, 0});
    node = std::make_shared<ngraph::opset7::Transpose>(node, transpose_order);

    if (enable_last_reshape) {
        auto shape_const = ngraph::opset7::Constant::create(ngraph::element::i64, ngraph::Shape{reshape_shape.size()}, reshape_shape);
        node = std::make_shared<ngraph::opset7::Reshape>(node, shape_const, false);
    }

    auto result = std::make_shared<ngraph::opset7::Result>(node);
    return std::make_shared<ngraph::Function>(ngraph::ResultVector{result}, ngraph::ParameterVector{input_params});
}

std::shared_ptr<ngraph::Function> CreateMatmulFunction(
    const ngraph::Shape& input_shape,
    const ngraph::Shape& matmul_shape,
    const ngraph::Shape& reshape_shape,
    bool create_reshape_instead_of_transpose,
    bool enable_last_reshape,
    bool enable_add,
    bool matmul_on_left_side,
    bool enable_fq) {
    auto input_params = std::make_shared<ngraph::opset7::Parameter>(ngraph::element::i64, input_shape);

    std::vector<size_t> data(ngraph::shape_size(matmul_shape));
    std::iota(std::begin(data), std::end(data), 1);
    auto matmul_constant = ngraph::opset7::Constant::create(ngraph::element::i64, matmul_shape, data);
    std::shared_ptr<ngraph::Node> node = std::make_shared<ngraph::opset7::MatMul>(input_params, matmul_constant);
    const auto matmul_output_shape = node->get_output_shape(0);
    if (enable_add) {
        auto add_const = ngraph::opset7::Constant::create(ngraph::element::i64, matmul_output_shape, {1});
        if (matmul_on_left_side) {
            node = std::make_shared<ngraph::opset7::Add>(add_const, node);
        } else {
            node = std::make_shared<ngraph::opset7::Add>(node, add_const);
        }
    }

    if (enable_fq) {
        node = std::make_shared<ngraph::opset7::FakeQuantize>(
            node,
            ngraph::opset7::Constant::create(ngraph::element::f32, {1}, {-0.1}),
            ngraph::opset7::Constant::create(ngraph::element::f32, {1}, {0.1}),
            ngraph::opset7::Constant::create(ngraph::element::f32, {1}, {-0.1}),
            ngraph::opset7::Constant::create(ngraph::element::f32, {1}, {0.1}),
            255);
    }

    std::shared_ptr<ngraph::Node> reshape;
    auto shape_const = ngraph::opset7::Constant::create(ngraph::element::i64, ngraph::Shape{reshape_shape.size()}, reshape_shape);
    if (create_reshape_instead_of_transpose) {
        auto reshape_instead_of_transpose_const = ngraph::opset7::Constant::create(ngraph::element::i64,
            ngraph::Shape{matmul_output_shape.size()}, {matmul_output_shape[1], matmul_output_shape[0]});
        auto reshape_instead_of_transpose = std::make_shared<ngraph::opset7::Reshape>(node, reshape_instead_of_transpose_const, false);
        reshape = reshape_instead_of_transpose;
        if (enable_last_reshape) {
            reshape = std::make_shared<ngraph::opset7::Reshape>(reshape_instead_of_transpose, shape_const, false);
        }
    } else {
        reshape = node;
        if (enable_last_reshape) {
            reshape = std::make_shared<ngraph::opset7::Reshape>(node, shape_const, false);
        }
    }

    auto result = std::make_shared<ngraph::opset7::Result>(reshape);
    return std::make_shared<ngraph::Function>(ngraph::ResultVector{result}, ngraph::ParameterVector{input_params});
}

} // namespace handle_transpose_after_matmul

namespace {

void RunTest(const std::shared_ptr<ngraph::Function>& func, const std::shared_ptr<ngraph::Function>& reference_func) {
    {
        ngraph::pass::Manager m;
        m.register_pass<ngraph::pass::InitNodeInfo>();
        m.register_pass<GNAPluginNS::HandleTransposesAroundMatMul>();
        m.run_passes(func);
        ASSERT_NO_THROW(check_rt_info(func));
    }

    const FunctionsComparator func_comparator = FunctionsComparator::with_default().enable(FunctionsComparator::ATTRIBUTES);
    const FunctionsComparator::Result result = func_comparator(func, reference_func);
    ASSERT_TRUE(result.valid);
}

} // namespace

TEST(TransformationTests, InsertTransposeBeforeMatmulTest) {
    RunTest(
        handle_transpose_before_matmul::CreateMatmulFunction({2, 8}, {8, 2}, {2, 1}, false),
        handle_transpose_before_matmul::CreateTransposeMatmulFunction({2, 8}, {8, 2}, {2, 1}, true));
    RunTest(
        handle_transpose_before_matmul::CreateMatmulFunction({1, 16}, {8, 2}, {2, 1}, false),
        handle_transpose_before_matmul::CreateTransposeMatmulFunction({1, 16}, {8, 2}, {2, 1}, true));
    RunTest(
        handle_transpose_before_matmul::CreateMatmulFunction({1, 2, 8}, {8, 2}, {2, 1}, false),
        handle_transpose_before_matmul::CreateTransposeMatmulFunction({1, 2, 8}, {8, 2}, {2, 1}, true));
}

TEST(TransformationTests, InsertTransposeBeforeMatmulTestReshapeInOutEq) {
    RunTest(
        handle_transpose_before_matmul::CreateMatmulFunction({2, 9}, {9, 2}, {2, 1}, false),
        handle_transpose_before_matmul::CreateMatmulFunction({2, 9}, {9, 2}, {2, 1}, false));
    RunTest(
        handle_transpose_before_matmul::CreateMatmulFunction({9, 2}, {9, 2}, {2, 1}, false),
        handle_transpose_before_matmul::CreateMatmulFunction({9, 2}, {9, 2}, {2, 1}, false));
}

TEST(TransformationTests, RemoveTransposeBeforeMatmulTest) {
    RunTest(
        handle_transpose_before_matmul::CreateTransposeMatmulFunction({1, 8}, {2, 4}, {2, 1}, false),
        handle_transpose_before_matmul::CreateMatmulFunction({1, 8}, {2, 4}, {2, 1}, true));
}

TEST(TransformationTests, RemoveTransposeBeforeMatmulTestReshapeInOutEq) {
    RunTest(
        handle_transpose_before_matmul::CreateTransposeMatmulFunction({2, 8}, {2, 8}, {2, 5}, false),
        handle_transpose_before_matmul::CreateTransposeMatmulFunction({2, 8}, {2, 8}, {2, 5}, false));
}

TEST(TransformationTests, InsertTransposeAfterMatmulTest) {
    for (auto enable_add : { true, false}) {
        for (auto matmul_on_left_side : { true, false}) {
            for (auto enable_fq : { true, false}) {
                RunTest(
                    handle_transpose_after_matmul::CreateMatmulFunction(
                        {4, 1}, {1, 8}, {2, 16}, false, true, enable_add, matmul_on_left_side, enable_fq),
                    handle_transpose_after_matmul::CreateMatmulTransposeFunction(
                        {4, 1}, {1, 8}, {2, 16}, true, true, enable_add, matmul_on_left_side, enable_fq));
                RunTest(
                    handle_transpose_after_matmul::CreateMatmulFunction(
                        {1, 256}, {256, 256}, {8, 32}, false, true, enable_add, matmul_on_left_side, enable_fq),
                    handle_transpose_after_matmul::CreateMatmulFunction(
                        {1, 256}, {256, 256}, {8, 32}, false, true, enable_add, matmul_on_left_side, enable_fq));
            }
        }
    }
}

TEST(TransformationTests, RemoveTransposeAfterMatmulTest) {
    for (auto enable_add : { true, false }) {
        for (auto matmul_on_left_side : { true, false }) {
            for (auto enable_fq : { true, false }) {
                RunTest(
                    handle_transpose_after_matmul::CreateMatmulTransposeFunction(
                        {4, 1}, {1, 8}, {2, 16}, false, true, enable_add, matmul_on_left_side, enable_fq),
                    handle_transpose_after_matmul::CreateMatmulFunction(
                        {4, 1}, {1, 8}, {2, 16}, true, true, enable_add, matmul_on_left_side, enable_fq));
            }
        }
    }
}

TEST(TransformationTests, RemoveTransposeAfterMatmulTestReshapeInOutEq) {
    for (auto enable_add : { true, false }) {
        for (auto matmul_on_left_side : { true, false }) {
            for (auto enable_fq : { true, false }) {
                RunTest(
                    handle_transpose_after_matmul::CreateMatmulTransposeFunction(
                        {4, 1}, {1, 8}, {8, 4}, false, true, enable_add, matmul_on_left_side, enable_fq),
                    handle_transpose_after_matmul::CreateMatmulTransposeFunction(
                        {4, 1}, {1, 8}, {8, 4}, false, true, enable_add, matmul_on_left_side, enable_fq));
            }
        }
    }
}

TEST(TransformationTests, InsertTransposeAfterMatmulTestReshapeInOutEq) {
    for (auto enable_last_reshape : { true, false }) {
        for (auto enable_add : { true, false }) {
            for (auto matmul_on_left_side : { true, false }) {
                for (auto enable_fq : { true, false }) {
                    RunTest(
                        handle_transpose_after_matmul::CreateMatmulFunction(
                            {4, 1}, {1, 8}, {4, 8}, false, enable_last_reshape, enable_add, matmul_on_left_side, enable_fq),
                        handle_transpose_after_matmul::CreateMatmulFunction(
                            {4, 1}, {1, 8}, {4, 8}, false, enable_last_reshape, enable_add, matmul_on_left_side, enable_fq));
                }
            }
        }
    }
}
