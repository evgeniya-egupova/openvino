// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "openvino/core/function.hpp"

#include <gtest/gtest.h>

#include <shared_node_info.hpp>
#include <test_common.hpp>

#include "openvino/core/partial_shape.hpp"
#include "openvino/opsets/opset8.hpp"

TEST(function, get_input_by_tensor_name) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto input = f->input("input");
    EXPECT_EQ(input.get_node(), arg0.get());
    EXPECT_EQ(input.get_element_type(), ov::element::f32);
    EXPECT_EQ(input.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_output_by_tensor_name) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    const std::unordered_set<std::string> out_names = {"relu_t", "identity"};
    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names(out_names);
    auto result = std::make_shared<ov::opset8::Result>(relu);
    auto f = std::make_shared<ov::Function>(result, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto output = f->output("relu_t");
    EXPECT_EQ(output.get_tensor().get_names().size(), 2);
    EXPECT_EQ(output.get_tensor().get_names(), out_names);
    EXPECT_EQ(output.get_node(), result.get());
    EXPECT_EQ(f->output("identity"), output);
    EXPECT_EQ(output.get_element_type(), ov::element::f32);
    EXPECT_EQ(output.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_input_by_tensor_index_without_name) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    auto f = std::make_shared<ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto input = f->input(0);
    EXPECT_THROW(f->input("input"), ov::Exception);
    EXPECT_EQ(input.get_node(), arg0.get());
    EXPECT_EQ(input.get_element_type(), ov::element::f32);
    EXPECT_EQ(input.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_output_by_tensor_index_without_name) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    auto result = std::make_shared<ov::opset8::Result>(relu);
    auto f = std::make_shared<ov::Function>(result, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto output = f->output(0);
    EXPECT_THROW(f->output("relu_t"), ov::Exception);
    EXPECT_EQ(output.get_tensor().get_names().size(), 0);
    EXPECT_EQ(output.get_node(), result.get());
    EXPECT_THROW(f->output("identity"), ov::Exception);
    EXPECT_EQ(output.get_element_type(), ov::element::f32);
    EXPECT_EQ(output.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_incorrect_output_by_tensor_name) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_THROW(f->output("input"), ov::Exception);
}

TEST(function, get_incorrect_input_by_tensor_name) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_THROW(f->input("relu_t"), ov::Exception);
}

TEST(function, get_input_by_index) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto input = f->input(0);
    EXPECT_EQ(input.get_node(), arg0.get());
    EXPECT_EQ(input.get_element_type(), ov::element::f32);
    EXPECT_EQ(input.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_output_by_index) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto result = std::make_shared<ov::opset8::Result>(relu);
    auto f = std::make_shared<ov::Function>(result, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto output = f->output(0);
    EXPECT_EQ(output.get_node(), result.get());
    EXPECT_EQ(output.get_element_type(), ov::element::f32);
    EXPECT_EQ(output.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_input_without_index) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto input = f->input();
    EXPECT_EQ(input.get_node(), arg0.get());
    EXPECT_EQ(input.get_element_type(), ov::element::f32);
    EXPECT_EQ(input.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_output_without_index) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto result = std::make_shared<ov::opset8::Result>(relu);
    auto f = std::make_shared<ov::Function>(result, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto output = f->output();
    EXPECT_EQ(output.get_node(), result.get());
    EXPECT_EQ(output.get_element_type(), ov::element::f32);
    EXPECT_EQ(output.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_incorrect_output_by_index) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_THROW(f->output(2), std::exception);
}

TEST(function, get_incorrect_input_by_index) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_THROW(f->input(2), std::exception);
}

TEST(function, incorrect_multiple_inputs_outputs_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1, 2, 3, 3});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input1"});

    auto arg1 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1, 2, 3, 3});
    arg1->set_friendly_name("data1");
    arg1->get_output_tensor(0).set_names({"input2", "data1"});

    auto concat = std::make_shared<ov::opset8::Concat>(ov::NodeVector{arg0, arg1}, 1);
    concat->set_friendly_name("concat");
    concat->get_output_tensor(0).set_names({"concat_t"});
    auto result1 = std::make_shared<ov::opset8::Result>(concat);

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto result2 = std::make_shared<ov::opset8::Result>(relu);
    auto f = std::make_shared<ov::Function>(ov::ResultVector{result1, result2}, ov::ParameterVector{arg0, arg1});

    f->validate_nodes_and_infer_types();

    EXPECT_THROW(f->input(), ov::Exception);
    EXPECT_THROW(f->output(), ov::Exception);
}

TEST(function, multiple_inputs_outputs_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1, 3, 3, 3});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input1"});

    auto arg1 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1, 2, 3, 3});
    arg1->set_friendly_name("data1");
    arg1->get_output_tensor(0).set_names({"input2", "data1"});

    auto concat = std::make_shared<ov::opset8::Concat>(ov::NodeVector{arg0, arg1}, 1);
    concat->set_friendly_name("concat");
    concat->get_output_tensor(0).set_names({"concat_t"});
    auto result1 = std::make_shared<ov::opset8::Result>(concat);

    auto shape_of = std::make_shared<ov::opset8::ShapeOf>(concat);
    shape_of->set_friendly_name("shape_of");
    shape_of->get_output_tensor(0).set_names({"shape_of_t", "identity"});
    auto result2 = std::make_shared<ov::opset8::Result>(shape_of);
    auto f = std::make_shared<ov::Function>(ov::ResultVector{result1, result2}, ov::ParameterVector{arg0, arg1});

    f->validate_nodes_and_infer_types();

    auto input1 = f->input(0);
    auto input2 = f->input("data1");

    EXPECT_NE(input1, input2);
    EXPECT_EQ(input1, f->input("input1"));
    EXPECT_EQ(input2, f->input("input2"));
    EXPECT_EQ(input2, f->input(1));
    EXPECT_EQ(input1.get_node(), arg0.get());
    EXPECT_EQ(input2.get_node_shared_ptr(), arg1);

    auto output1 = f->output(0);
    auto output2 = f->output("shape_of_t");

    EXPECT_NE(output1, output2);
    EXPECT_EQ(output1, f->output("concat_t"));
    EXPECT_EQ(output2, f->output("identity"));
    EXPECT_EQ(output2, f->output(1));
    EXPECT_EQ(arg0.get(), f->input(0).get_node());
    EXPECT_EQ(arg1.get(), f->input(1).get_node());
    EXPECT_EQ(result1.get(), f->output(0).get_node());
    EXPECT_EQ(result2.get(), f->output(1).get_node());
    EXPECT_EQ(output1, result1);
    EXPECT_EQ(output2, result2);
    EXPECT_EQ(f->inputs().size(), 2);
    EXPECT_EQ(f->outputs().size(), 2);
}

TEST(function, get_input_by_tensor_name_from_const) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<const ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto input = f->input("input");
    EXPECT_EQ(input.get_node(), arg0.get());
    EXPECT_EQ(input.get_element_type(), ov::element::f32);
    EXPECT_EQ(input.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_output_by_tensor_name_from_const_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    const std::unordered_set<std::string> out_names = {"relu_t", "identity"};
    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names(out_names);
    auto result = std::make_shared<ov::opset8::Result>(relu);
    auto f = std::make_shared<const ov::Function>(result, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto output = f->output("relu_t");
    EXPECT_EQ(output.get_tensor().get_names().size(), 2);
    EXPECT_EQ(output.get_tensor().get_names(), out_names);
    EXPECT_EQ(output.get_node(), result.get());
    EXPECT_EQ(f->output("identity"), output);
    EXPECT_EQ(output.get_element_type(), ov::element::f32);
    EXPECT_EQ(output.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_incorrect_output_by_tensor_name_from_const_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<const ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_THROW(f->output("input"), ov::Exception);
}

TEST(function, get_incorrect_input_by_tensor_name_from_const_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<const ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_THROW(f->input("relu_t"), ov::Exception);
}

TEST(function, get_input_by_index_from_const_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<const ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto input = f->input(0);
    EXPECT_EQ(input.get_node(), arg0.get());
    EXPECT_EQ(input.get_element_type(), ov::element::f32);
    EXPECT_EQ(input.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_output_by_index_from_const_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto result = std::make_shared<ov::opset8::Result>(relu);
    auto f = std::make_shared<const ov::Function>(result, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto output = f->output(0);
    EXPECT_EQ(output.get_node(), result.get());
    EXPECT_EQ(output.get_element_type(), ov::element::f32);
    EXPECT_EQ(output.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_input_without_index_from_const_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<const ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto input = f->input();
    EXPECT_EQ(input.get_node(), arg0.get());
    EXPECT_EQ(input.get_element_type(), ov::element::f32);
    EXPECT_EQ(input.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_output_without_index_from_const_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto result = std::make_shared<ov::opset8::Result>(relu);
    auto f = std::make_shared<const ov::Function>(result, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    auto output = f->output();
    EXPECT_EQ(output.get_node(), result.get());
    EXPECT_EQ(output.get_element_type(), ov::element::f32);
    EXPECT_EQ(output.get_partial_shape(), ov::PartialShape{1});
}

TEST(function, get_incorrect_output_by_index_from_const_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<const ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_THROW(f->output(2), std::exception);
}

TEST(function, get_incorrect_input_by_index_from_const_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto f = std::make_shared<const ov::Function>(relu, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_THROW(f->input(2), std::exception);
}

TEST(function, incorrect_multiple_inputs_outputs_function_from_const_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1, 2, 3, 3});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input1"});

    auto arg1 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1, 2, 3, 3});
    arg1->set_friendly_name("data1");
    arg1->get_output_tensor(0).set_names({"input2", "data1"});

    auto concat = std::make_shared<ov::opset8::Concat>(ov::NodeVector{arg0, arg1}, 1);
    concat->set_friendly_name("concat");
    concat->get_output_tensor(0).set_names({"concat_t"});
    auto result1 = std::make_shared<ov::opset8::Result>(concat);

    auto relu = std::make_shared<ov::opset8::Relu>(arg0);
    relu->set_friendly_name("relu");
    relu->get_output_tensor(0).set_names({"relu_t", "identity"});
    auto result2 = std::make_shared<ov::opset8::Result>(relu);
    auto f = std::make_shared<const ov::Function>(ov::ResultVector{result1, result2}, ov::ParameterVector{arg0, arg1});

    f->validate_nodes_and_infer_types();

    EXPECT_THROW(f->input(), ov::Exception);
    EXPECT_THROW(f->output(), ov::Exception);
}

TEST(function, multiple_inputs_outputs_function_from_const_function) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1, 3, 3, 3});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input1"});

    auto arg1 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1, 2, 3, 3});
    arg1->set_friendly_name("data1");
    arg1->get_output_tensor(0).set_names({"input2", "data1"});

    auto concat = std::make_shared<ov::opset8::Concat>(ov::NodeVector{arg0, arg1}, 1);
    concat->set_friendly_name("concat");
    concat->get_output_tensor(0).set_names({"concat_t"});
    auto result1 = std::make_shared<ov::opset8::Result>(concat);

    auto shape_of = std::make_shared<ov::opset8::ShapeOf>(concat);
    shape_of->set_friendly_name("shape_of");
    shape_of->get_output_tensor(0).set_names({"shape_of_t", "identity"});
    auto result2 = std::make_shared<ov::opset8::Result>(shape_of);
    auto f = std::make_shared<const ov::Function>(ov::ResultVector{result1, result2}, ov::ParameterVector{arg0, arg1});

    f->validate_nodes_and_infer_types();

    auto input1 = f->input(0);
    auto input2 = f->input("data1");

    EXPECT_NE(input1, input2);
    EXPECT_EQ(input1, f->input("input1"));
    EXPECT_EQ(input2, f->input("input2"));
    EXPECT_EQ(input2, f->input(1));
    EXPECT_EQ(input1.get_node(), arg0.get());
    EXPECT_EQ(input2.get_node_shared_ptr(), arg1);

    auto output1 = f->output(0);
    auto output2 = f->output("shape_of_t");

    EXPECT_NE(output1, output2);
    EXPECT_EQ(output1, f->output("concat_t"));
    EXPECT_EQ(output2, f->output("identity"));
    EXPECT_EQ(arg0.get(), f->input(0).get_node());
    EXPECT_EQ(arg1.get(), f->input(1).get_node());
    EXPECT_EQ(result1.get(), f->output(0).get_node());
    EXPECT_EQ(result2.get(), f->output(1).get_node());
    EXPECT_EQ(output2, f->output(1));
    EXPECT_EQ(output1.get_node(), result1.get());
    EXPECT_EQ(output2.get_node(), result2.get());
    EXPECT_EQ(f->inputs().size(), 2);
    EXPECT_EQ(f->outputs().size(), 2);
}

TEST(function_reshape, ReshapedDynamicShapeLayout) {
    std::shared_ptr<ov::Function> ngraph;
    {
        ov::PartialShape shape({-1, 3, 22, 22});
        ov::element::Type type(ov::element::Type_t::f32);
        auto param = std::make_shared<ov::op::v0::Parameter>(type, shape);
        param->get_output_tensor(0).set_names({"tensor", "tensor2"});
        auto relu = std::make_shared<ov::op::v0::Relu>(param);

        ov::ParameterVector params = {param};
        ngraph = std::make_shared<ov::Function>(relu, params);
    }

    EXPECT_TRUE(ngraph->input().get_partial_shape().is_dynamic());

    std::map<std::string, ov::PartialShape> new_shape;
    new_shape["tensor"] = ov::Shape{1, 3, 22, 22};
    EXPECT_NO_THROW(ngraph->reshape(new_shape));

    EXPECT_FALSE(ngraph->input().get_partial_shape().is_dynamic());
    EXPECT_FALSE(ngraph->get_parameters().front()->get_partial_shape().is_dynamic());
}

TEST(function_reshape, ReshapeBatchReLU) {
    std::shared_ptr<ov::Function> ngraph;
    {
        ov::PartialShape shape({1, 3, 22, 22});
        ov::element::Type type(ov::element::Type_t::f32);
        auto param = std::make_shared<ov::op::v0::Parameter>(type, shape);
        param->get_output_tensor(0).set_names({"tensor", "tensor2"});
        auto relu = std::make_shared<ov::op::v0::Relu>(param);
        auto result = std::make_shared<ov::op::v0::Result>(relu);

        ov::ParameterVector params = {param};
        ov::ResultVector results = {result};

        ngraph = std::make_shared<ov::Function>(results, params);
    }

    EXPECT_EQ(ngraph->get_parameters()[0]->get_shape(), ov::Shape({1, 3, 22, 22}));
    EXPECT_EQ(ngraph->get_results()[0]->get_shape(), ov::Shape({1, 3, 22, 22}));

    {
        std::map<std::string, ov::PartialShape> new_shape;
        new_shape["tensor2"] = ov::PartialShape{2, 3, 22, 22};
        EXPECT_NO_THROW(ngraph->reshape(new_shape));
    }

    EXPECT_EQ(ngraph->get_parameters()[0]->get_shape(), ov::Shape({2, 3, 22, 22}));
    EXPECT_EQ(ngraph->get_results()[0]->get_shape(), ov::Shape({2, 3, 22, 22}));
}

TEST(function_reshape, ReshapeSpatialReLU) {
    std::shared_ptr<ov::Function> ngraph;
    {
        ov::PartialShape shape({1, 3, 22, 22});
        ov::element::Type type(ov::element::Type_t::f32);
        auto param = std::make_shared<ov::op::v0::Parameter>(type, shape);
        param->get_output_tensor(0).set_names({"tensor"});
        auto relu = std::make_shared<ov::op::v0::Relu>(param);
        auto result = std::make_shared<ov::op::v0::Result>(relu);

        ov::ParameterVector params = {param};
        ov::ResultVector results = {result};

        ngraph = std::make_shared<ov::Function>(results, params);
    }

    EXPECT_EQ(ngraph->get_parameters()[0]->get_shape(), ov::Shape({1, 3, 22, 22}));
    EXPECT_EQ(ngraph->get_results()[0]->get_shape(), ov::Shape({1, 3, 22, 22}));

    {
        std::map<std::string, ov::PartialShape> new_shape;
        new_shape["tensor"] = ov::PartialShape{1, 3, 25, 25};
        EXPECT_NO_THROW(ngraph->reshape(new_shape));
    }

    EXPECT_EQ(ngraph->get_parameters()[0]->get_shape(), ov::Shape({1, 3, 25, 25}));
    EXPECT_EQ(ngraph->get_results()[0]->get_shape(), ov::Shape({1, 3, 25, 25}));
}

TEST(function_reshape, ReshapeSpatialReLUWithoutReplaceParameter) {
    std::shared_ptr<ov::Function> ngraph;
    {
        ov::PartialShape shape({1, 3, 22, 22});
        ov::element::Type type(ov::element::Type_t::f32);
        auto param = std::make_shared<ov::op::v0::Parameter>(type, shape);
        auto relu = std::make_shared<ov::op::v0::Relu>(param);
        auto result = std::make_shared<ov::op::v0::Result>(relu);

        ov::ParameterVector params = {param};
        ov::ResultVector results = {result};

        ngraph = std::make_shared<ov::Function>(results, params);
    }

    EXPECT_EQ(ngraph->get_parameters()[0]->get_shape(), ov::Shape({1, 3, 22, 22}));
    EXPECT_EQ(ngraph->get_results()[0]->get_shape(), ov::Shape({1, 3, 22, 22}));

    {
        ngraph->get_parameters()[0]->set_partial_shape({1, 3, 25, 25});
        ngraph->validate_nodes_and_infer_types();
    }

    EXPECT_EQ(ngraph->input().get_partial_shape(), ov::Shape({1, 3, 25, 25}));
    EXPECT_EQ(ngraph->output().get_partial_shape(), ov::Shape({1, 3, 25, 25}));
}

TEST(function_reshape, ReshapeSpatialReLUStaticToDynamic) {
    const ov::PartialShape refShape{1, 3, ov::Dimension::dynamic(), 25};
    std::shared_ptr<ov::Function> ngraph;
    {
        ov::PartialShape shape({1, 3, 22, 22});
        ov::element::Type type(ov::element::Type_t::f32);
        auto param = std::make_shared<ov::op::v0::Parameter>(type, shape);
        param->get_output_tensor(0).set_names({"tensor"});
        auto relu = std::make_shared<ov::op::v0::Relu>(param);
        auto result = std::make_shared<ov::op::v0::Result>(relu);

        ov::ParameterVector params = {param};
        ov::ResultVector results = {result};

        ngraph = std::make_shared<ov::Function>(results, params);
    }

    EXPECT_EQ(ngraph->get_parameters()[0]->get_shape(), ov::Shape({1, 3, 22, 22}));
    EXPECT_EQ(ngraph->get_results()[0]->get_shape(), ov::Shape({1, 3, 22, 22}));

    {
        std::map<std::string, ov::PartialShape> new_shape;
        new_shape["tensor"] = refShape;
        EXPECT_NO_THROW(ngraph->reshape(new_shape));
    }

    EXPECT_TRUE(ngraph->input(0).get_partial_shape().is_dynamic());
    EXPECT_TRUE(ngraph->output(0).get_partial_shape().is_dynamic());
    EXPECT_EQ(ngraph->input(0).get_partial_shape(), refShape);
    EXPECT_EQ(ngraph->output(0).get_partial_shape(), refShape);
}

TEST(function_reshape, ReshapeSpatialReLUStaticToFullyDynamic) {
    const ov::PartialShape refShape = ov::PartialShape::dynamic();
    std::shared_ptr<ov::Function> ngraph;
    {
        ov::PartialShape shape({1, 3, 22, 22});
        ov::element::Type type(ov::element::Type_t::f32);
        auto param = std::make_shared<ov::op::v0::Parameter>(type, shape);
        param->get_output_tensor(0).set_names({"tensor"});
        auto relu = std::make_shared<ov::op::v0::Relu>(param);
        auto result = std::make_shared<ov::op::v0::Result>(relu);

        ov::ParameterVector params = {param};
        ov::ResultVector results = {result};

        ngraph = std::make_shared<ov::Function>(results, params);
    }

    EXPECT_EQ(ngraph->get_parameters()[0]->get_shape(), ov::Shape({1, 3, 22, 22}));
    EXPECT_EQ(ngraph->get_results()[0]->get_shape(), ov::Shape({1, 3, 22, 22}));

    {
        std::map<std::string, ov::PartialShape> new_shape;
        new_shape["tensor"] = refShape;
        EXPECT_NO_THROW(ngraph->reshape(new_shape));
    }

    EXPECT_TRUE(ngraph->input().get_partial_shape().is_dynamic());
    EXPECT_TRUE(ngraph->output().get_partial_shape().is_dynamic());
    EXPECT_EQ(ngraph->input().get_partial_shape(), refShape);
    EXPECT_EQ(ngraph->output().get_partial_shape(), refShape);
}

TEST(function_reshape, ReshapeSpatialReLUDynamicToDynamic) {
    const ov::PartialShape refShape{1, 3, ov::Dimension::dynamic(), 25};
    std::shared_ptr<ov::Function> ngraph;
    {
        ov::PartialShape shape({1, 3, 22, ov::Dimension::dynamic()});
        ov::element::Type type(ov::element::Type_t::f32);
        auto param = std::make_shared<ov::op::v0::Parameter>(type, shape);
        param->get_output_tensor(0).set_names({"tensor"});
        auto relu = std::make_shared<ov::op::v0::Relu>(param);
        auto result = std::make_shared<ov::op::v0::Result>(relu);

        ov::ParameterVector params = {param};
        ov::ResultVector results = {result};

        ngraph = std::make_shared<ov::Function>(results, params);
    }

    EXPECT_EQ(ngraph->input().get_partial_shape(), ov::PartialShape({1, 3, 22, ov::Dimension::dynamic()}));
    EXPECT_EQ(ngraph->output().get_partial_shape(), ov::PartialShape({1, 3, 22, ov::Dimension::dynamic()}));

    {
        std::map<std::string, ov::PartialShape> new_shape;
        new_shape["tensor"] = refShape;
        EXPECT_NO_THROW(ngraph->reshape(new_shape));
    }

    EXPECT_TRUE(ngraph->input().get_partial_shape().is_dynamic());
    EXPECT_TRUE(ngraph->output().get_partial_shape().is_dynamic());
    EXPECT_EQ(ngraph->input().get_partial_shape(), refShape);
    EXPECT_EQ(ngraph->output().get_partial_shape(), refShape);
}

TEST(function_reshape, TestInvalidReshape) {
    std::shared_ptr<ov::Function> f;
    {
        auto input = std::make_shared<ov::op::v0::Parameter>(ov::element::f32, ov::Shape{1, 1000, 4});
        input->get_output_tensor(0).set_names({"tensor"});
        auto shape = ov::op::v0::Constant::create(ov::element::i64, {2}, {1, 4000});
        auto reshape = std::make_shared<ov::op::v1::Reshape>(input, shape, true);
        f = std::make_shared<ov::Function>(ov::OutputVector{reshape}, ov::ParameterVector{input});
    }

    EXPECT_ANY_THROW(f->reshape({{"tensor", ov::Shape({4})}}));

    auto param = f->get_parameters().front();
    EXPECT_EQ(param->get_output_shape(0), ov::Shape({1, 1000, 4}));

    EXPECT_NO_THROW(f->reshape({{"tensor", ov::Shape({1, 1000, 4})}}));
}

TEST(function_reshape, TestReshapeWithInvalidTensorName) {
    std::shared_ptr<ov::Function> f;
    {
        auto input = std::make_shared<ov::op::v0::Parameter>(ov::element::f32, ov::Shape{1, 1000, 4});
        input->set_friendly_name("param");
        input->get_output_tensor(0).set_names({"tensor"});
        auto shape = ov::op::v0::Constant::create(ov::element::i64, {2}, {1, 4000});
        auto reshape = std::make_shared<ov::op::v1::Reshape>(input, shape, true);
        f = std::make_shared<ov::Function>(ov::OutputVector{reshape}, ov::ParameterVector{input});
    }

    // both operation names and tensor names are specified
    EXPECT_ANY_THROW(f->reshape({{"param", ov::Shape({4, 4, 4})}, {"tensor", ov::Shape({4, 4, 4})}}));

    // operation name does not work
    EXPECT_ANY_THROW(f->reshape({{"param", ov::Shape({4, 4, 4})}}));
}

TEST(function_reshape, TestReshapeWithInvalidShapesForTheSameTensor) {
    std::shared_ptr<ov::Function> f;
    {
        auto input = std::make_shared<ov::op::v0::Parameter>(ov::element::f32, ov::Shape{1, 1000, 4});
        input->set_friendly_name("param");
        input->get_output_tensor(0).set_names({"tensor1", "tensor2"});
        auto shape = ov::op::v0::Constant::create(ov::element::i64, {2}, {1, 4000});
        auto reshape = std::make_shared<ov::op::v1::Reshape>(input, shape, true);
        f = std::make_shared<ov::Function>(ov::OutputVector{reshape}, ov::ParameterVector{input});
    }

    // both tensor names are specified, but have different shapes
    EXPECT_ANY_THROW(f->reshape({{"tensor1", ov::Shape({2, 500, 4})}, {"tensor2", ov::Shape({4, 250, 4})}}));
}

TEST(function_reshape, ReshapeBatchReLUByPort) {
    std::shared_ptr<ov::Function> ngraph;
    ov::Output<ov::Node> port;
    {
        ov::PartialShape shape({1, 3, 22, 22});
        ov::element::Type type(ov::element::Type_t::f32);
        auto param = std::make_shared<ov::op::v0::Parameter>(type, shape);
        param->get_output_tensor(0).set_names({"tensor", "tensor2"});
        port = param->output(0);
        auto relu = std::make_shared<ov::op::v0::Relu>(param);
        auto result = std::make_shared<ov::op::v0::Result>(relu);

        ov::ParameterVector params = {param};
        ov::ResultVector results = {result};

        ngraph = std::make_shared<ov::Function>(results, params);
    }

    EXPECT_EQ(ngraph->get_parameters()[0]->get_shape(), ov::Shape({1, 3, 22, 22}));
    EXPECT_EQ(ngraph->get_results()[0]->get_shape(), ov::Shape({1, 3, 22, 22}));

    {
        std::map<ov::Output<ov::Node>, ov::PartialShape> new_shape;
        new_shape[port] = ov::PartialShape{2, 3, 22, 22};
        EXPECT_NO_THROW(ngraph->reshape(new_shape));
    }

    EXPECT_EQ(ngraph->get_parameters()[0]->get_shape(), ov::Shape({2, 3, 22, 22}));
    EXPECT_EQ(ngraph->get_results()[0]->get_shape(), ov::Shape({2, 3, 22, 22}));
}

TEST(function, add_output_tensor_name) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    relu1->set_friendly_name("relu1");
    relu1->get_output_tensor(0).set_names({"relu_t1"});

    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    relu2->set_friendly_name("relu2");
    relu2->get_output_tensor(0).set_names({"relu_t2"});
    auto f = std::make_shared<ov::Function>(relu2, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_EQ(f->get_results().size(), 1);

    ov::Output<ov::Node> out1, out2;
    EXPECT_NO_THROW(out1 = f->add_output("relu_t1"));
    EXPECT_EQ(f->get_results().size(), 2);
    EXPECT_NO_THROW(out2 = f->add_output("relu_t1"));
    EXPECT_EQ(f->get_results().size(), 2);
    EXPECT_EQ(f->get_results()[1]->input_value(0).get_node(), relu1.get());
    EXPECT_EQ(out1, out2);
    EXPECT_EQ(out1.get_node(), f->get_results()[1].get());
}

TEST(function, add_output_op_name) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    relu1->set_friendly_name("relu1");
    relu1->get_output_tensor(0).set_names({"relu_t1"});

    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    relu2->set_friendly_name("relu2");
    relu2->get_output_tensor(0).set_names({"relu_t2"});
    auto f = std::make_shared<ov::Function>(relu2, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_EQ(f->get_results().size(), 1);

    EXPECT_NO_THROW(f->add_output("relu1", 0));
    EXPECT_EQ(f->get_results().size(), 2);
    EXPECT_NO_THROW(f->add_output("relu_t1"));
    EXPECT_EQ(f->get_results().size(), 2);
    EXPECT_NO_THROW(f->add_output("relu2", 0));
    EXPECT_EQ(f->get_results().size(), 2);
    EXPECT_EQ(f->get_results()[1]->input_value(0).get_node(), relu1.get());
}

TEST(function, add_output_port) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    relu1->set_friendly_name("relu1");
    relu1->get_output_tensor(0).set_names({"relu_t1"});

    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    relu2->set_friendly_name("relu2");
    relu2->get_output_tensor(0).set_names({"relu_t2"});
    auto f = std::make_shared<ov::Function>(relu2, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_EQ(f->get_results().size(), 1);

    ov::Output<ov::Node> out;
    EXPECT_NO_THROW(out = f->add_output(relu1->output(0)));
    EXPECT_EQ(out.get_node(), f->get_results()[1].get());
    EXPECT_EQ(f->get_results().size(), 2);
    EXPECT_EQ(f->get_results()[1]->input_value(0).get_node(), relu1.get());
}

TEST(function, add_output_incorrect_tensor_name) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    relu1->set_friendly_name("relu1");
    relu1->get_output_tensor(0).set_names({"relu_t1"});

    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    relu2->set_friendly_name("relu2");
    relu2->get_output_tensor(0).set_names({"relu_t2"});
    auto f = std::make_shared<ov::Function>(relu2, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_EQ(f->get_results().size(), 1);

    EXPECT_THROW(f->add_output("relu"), ov::Exception);
    EXPECT_EQ(f->get_results().size(), 1);
}

TEST(function, add_output_op_incorrect_name) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    relu1->set_friendly_name("relu1");
    relu1->get_output_tensor(0).set_names({"relu_t1"});

    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    relu2->set_friendly_name("relu2");
    relu2->get_output_tensor(0).set_names({"relu_t2"});
    auto f = std::make_shared<ov::Function>(relu2, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_EQ(f->get_results().size(), 1);

    EXPECT_THROW(f->add_output("relu_t1", 0), ov::Exception);
    EXPECT_EQ(f->get_results().size(), 1);
}

TEST(function, add_output_op_name_incorrect_idx) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    relu1->set_friendly_name("relu1");
    relu1->get_output_tensor(0).set_names({"relu_t1"});

    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    relu2->set_friendly_name("relu2");
    relu2->get_output_tensor(0).set_names({"relu_t2"});
    auto f = std::make_shared<ov::Function>(relu2, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_EQ(f->get_results().size(), 1);

    EXPECT_THROW(f->add_output("relu1", 10), ov::Exception);
    EXPECT_EQ(f->get_results().size(), 1);
}

TEST(function, add_output_port_to_result) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    arg0->set_friendly_name("data");
    arg0->get_output_tensor(0).set_names({"input"});

    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    relu1->set_friendly_name("relu1");
    relu1->get_output_tensor(0).set_names({"relu_t1"});

    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    relu2->set_friendly_name("relu2");
    relu2->get_output_tensor(0).set_names({"relu_t2"});
    auto result = std::make_shared<ov::opset8::Result>(relu2);
    auto f = std::make_shared<ov::Function>(ov::ResultVector{result}, ov::ParameterVector{arg0});
    f->validate_nodes_and_infer_types();

    EXPECT_EQ(f->get_results().size(), 1);

    ov::Output<ov::Node> out;
    EXPECT_NO_THROW(out = f->add_output(result->output(0)));
    EXPECT_EQ(f->get_results().size(), 1);
    EXPECT_EQ(out, result->output(0));
}

namespace {
bool all_ops_have_same_info(const std::shared_ptr<ov::Function>& f) {
    auto shared_info = ov::FunctionAccessor(f).get_shared_info();
    for (auto&& op : f->get_ordered_ops()) {
        if (std::set<std::shared_ptr<ov::SharedRTInfo>>({shared_info}) != ov::NodeAccessor(op).get_shared_info()) {
            return false;
        }
    }
    return true;
}
}  // namespace

TEST(function, topological_sort_caching_basic) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    auto result = std::make_shared<ov::opset8::Result>(relu2);
    auto f = std::make_shared<ov::Function>(ov::ResultVector{result}, ov::ParameterVector{arg0});

    auto shared_info = ov::FunctionAccessor(f).get_shared_info();
    // Check that after function creation which call get_ordered_ops
    // cache is set to true value
    ASSERT_TRUE(shared_info->get_use_topological_cache());

    // Check that nodes contains the same shared info after function creation
    ASSERT_EQ(ov::NodeAccessor(arg0).get_shared_info().size(), 1);
    ASSERT_TRUE(ov::NodeAccessor(arg0).get_shared_info().count(shared_info));

    ASSERT_EQ(ov::NodeAccessor(relu1).get_shared_info().size(), 1);
    ASSERT_TRUE(ov::NodeAccessor(relu1).get_shared_info().count(shared_info));

    ASSERT_EQ(ov::NodeAccessor(relu2).get_shared_info().size(), 1);
    ASSERT_TRUE(ov::NodeAccessor(relu2).get_shared_info().count(shared_info));

    ASSERT_EQ(ov::NodeAccessor(result).get_shared_info().size(), 1);
    ASSERT_TRUE(ov::NodeAccessor(result).get_shared_info().count(shared_info));

    ASSERT_EQ(f->get_ordered_ops().size(), 4);
}

TEST(function, topological_sort_caching_replace_node) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    auto result = std::make_shared<ov::opset8::Result>(relu2);
    auto f = std::make_shared<ov::Function>(ov::ResultVector{result}, ov::ParameterVector{arg0});

    auto shared_info = ov::FunctionAccessor(f).get_shared_info();
    ASSERT_TRUE(shared_info->get_use_topological_cache());

    auto new_relu = std::make_shared<ov::opset8::Relu>(relu1);
    ov::replace_node(relu2, new_relu);

    // Function has changed so cache must be updated
    ASSERT_FALSE(shared_info->get_use_topological_cache());

    // Before get_ordered_ops, new_node shouldn't have shared_info, but after
    // it will be set to the function shared_info and cache will be used.
    ASSERT_FALSE(ov::NodeAccessor(new_relu).get_shared_info().count(shared_info));
    ASSERT_EQ(f->get_ordered_ops().size(), 4);
    ASSERT_TRUE(ov::NodeAccessor(new_relu).get_shared_info().count(shared_info));
    ASSERT_TRUE(shared_info->get_use_topological_cache());
    ASSERT_TRUE(all_ops_have_same_info(f));
}

TEST(function, topological_sort_caching_replace_source_output) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    auto result = std::make_shared<ov::opset8::Result>(relu2);
    auto f = std::make_shared<ov::Function>(ov::ResultVector{result}, ov::ParameterVector{arg0});

    auto shared_info = ov::FunctionAccessor(f).get_shared_info();
    ASSERT_TRUE(shared_info->get_use_topological_cache());

    relu2->input(0).replace_source_output(relu1);

    // Function has changed so cache must be updated
    ASSERT_FALSE(shared_info->get_use_topological_cache());

    ASSERT_EQ(f->get_ordered_ops().size(), 4);
    ASSERT_TRUE(shared_info->get_use_topological_cache());
    ASSERT_TRUE(all_ops_have_same_info(f));
}

TEST(function, topological_sort_caching_dangling_node) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    auto result = std::make_shared<ov::opset8::Result>(relu2);
    auto f = std::make_shared<ov::Function>(ov::ResultVector{result}, ov::ParameterVector{arg0});

    auto shared_info = ov::FunctionAccessor(f).get_shared_info();
    ASSERT_TRUE(shared_info->get_use_topological_cache());

    auto new_relu = std::make_shared<ov::opset8::Relu>(relu1);

    // Function has not changed so cache mustn't be updated
    ASSERT_TRUE(shared_info->get_use_topological_cache());
    // Dangling node is not in Function
    ASSERT_EQ(f->get_ordered_ops().size(), 4);
}

TEST(function, topological_sort_caching_replace_output) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    auto result = std::make_shared<ov::opset8::Result>(relu2);
    auto f = std::make_shared<ov::Function>(ov::ResultVector{result}, ov::ParameterVector{arg0});

    auto shared_info = ov::FunctionAccessor(f).get_shared_info();
    ASSERT_TRUE(shared_info->get_use_topological_cache());

    auto new_relu = std::make_shared<ov::opset8::Relu>(relu1);
    relu2->output(0).replace(new_relu);

    // Function has changed so cache must be updated
    ASSERT_FALSE(shared_info->get_use_topological_cache());
    ASSERT_EQ(f->get_ordered_ops().size(), 4);
    ASSERT_TRUE(shared_info->get_use_topological_cache());
    ASSERT_TRUE(all_ops_have_same_info(f));
}

TEST(function, topological_sort_caching_set_argument) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    auto result = std::make_shared<ov::opset8::Result>(relu2);
    auto f = std::make_shared<ov::Function>(ov::ResultVector{result}, ov::ParameterVector{arg0});

    auto shared_info = ov::FunctionAccessor(f).get_shared_info();
    ASSERT_TRUE(shared_info->get_use_topological_cache());

    relu2->set_argument(0, arg0);

    // Function has changed so cache must be updated
    ASSERT_FALSE(shared_info->get_use_topological_cache());
    ASSERT_EQ(f->get_ordered_ops().size(), 3);
    ASSERT_TRUE(shared_info->get_use_topological_cache());
    ASSERT_TRUE(all_ops_have_same_info(f));
}

TEST(function, topological_sort_caching_set_arguments) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    auto result = std::make_shared<ov::opset8::Result>(relu2);
    auto f = std::make_shared<ov::Function>(ov::ResultVector{result}, ov::ParameterVector{arg0});

    auto shared_info = ov::FunctionAccessor(f).get_shared_info();
    ASSERT_TRUE(shared_info->get_use_topological_cache());

    relu2->set_arguments({arg0->output(0)});

    // Function has changed so cache must be updated
    ASSERT_FALSE(shared_info->get_use_topological_cache());
    ASSERT_EQ(f->get_ordered_ops().size(), 3);
    ASSERT_TRUE(shared_info->get_use_topological_cache());
    ASSERT_TRUE(all_ops_have_same_info(f));
}

TEST(function, topological_sort_caching_add_cf) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    auto result = std::make_shared<ov::opset8::Result>(relu2);
    auto f = std::make_shared<ov::Function>(ov::ResultVector{result}, ov::ParameterVector{arg0});

    auto shared_info = ov::FunctionAccessor(f).get_shared_info();
    ASSERT_TRUE(shared_info->get_use_topological_cache());

    relu2->add_control_dependency(arg0);

    // Function has changed so cache must be updated
    ASSERT_FALSE(shared_info->get_use_topological_cache());
    ASSERT_EQ(f->get_ordered_ops().size(), 4);
    ASSERT_TRUE(shared_info->get_use_topological_cache());
    ASSERT_TRUE(all_ops_have_same_info(f));
}

TEST(function, topological_sort_caching_result_parameter_sink) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    auto relu1 = std::make_shared<ov::opset8::Relu>(arg0);
    auto relu2 = std::make_shared<ov::opset8::Relu>(relu1);
    auto result = std::make_shared<ov::opset8::Result>(relu2);
    auto f = std::make_shared<ov::Function>(ov::ResultVector{result}, ov::ParameterVector{arg0});

    auto shared_info = ov::FunctionAccessor(f).get_shared_info();
    ASSERT_TRUE(shared_info->get_use_topological_cache());

    auto check_caching_status = [=](int64_t expected_number_of_ops) {
        ASSERT_FALSE(shared_info->get_use_topological_cache());
        ASSERT_EQ(f->get_ordered_ops().size(), expected_number_of_ops);
        ASSERT_TRUE(shared_info->get_use_topological_cache());
        ASSERT_TRUE(all_ops_have_same_info(f));
    };

    auto result2 = std::make_shared<ov::opset8::Result>(relu2);
    f->add_results({result2});
    check_caching_status(5);

    f->remove_result(result2);
    check_caching_status(4);

    auto arg1 = std::make_shared<ov::opset8::Parameter>();
    f->add_parameters({arg1});
    check_caching_status(5);

    f->remove_parameter(arg1);
    check_caching_status(4);

    auto assign = std::make_shared<ov::opset8::Assign>();
    f->add_sinks({assign});
    check_caching_status(5);

    f->remove_sink(assign);
    check_caching_status(4);
}

TEST(function, topological_sort_caching_multiple_components) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    auto relu0 = std::make_shared<ov::opset8::Relu>(arg0);
    auto result0 = std::make_shared<ov::opset8::Result>(relu0);

    auto arg1 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    auto relu1 = std::make_shared<ov::opset8::Relu>(arg1);
    auto result1 = std::make_shared<ov::opset8::Result>(relu1);

    auto f = std::make_shared<ov::Function>(ov::ResultVector{result0, result1}, ov::ParameterVector{arg0, arg1});

    auto shared_info = ov::FunctionAccessor(f).get_shared_info();
    ASSERT_TRUE(shared_info->get_use_topological_cache());
    ASSERT_TRUE(all_ops_have_same_info(f));
}

TEST(function, topological_sort_caching_shared_nodes) {
    auto arg0 = std::make_shared<ov::opset8::Parameter>(ov::element::f32, ov::PartialShape{1});
    auto relu0 = std::make_shared<ov::opset8::Relu>(arg0);
    auto result0 = std::make_shared<ov::opset8::Result>(relu0);

    auto f1 = std::make_shared<ov::Function>(ov::ResultVector{result0}, ov::ParameterVector{arg0});
    auto f2 = std::make_shared<ov::Function>(ov::ResultVector{result0}, ov::ParameterVector{arg0});

    auto f1_shared_info = ov::FunctionAccessor(f1).get_shared_info();
    auto f2_shared_info = ov::FunctionAccessor(f2).get_shared_info();

    for (auto&& node : f1->get_ordered_ops()) {
        auto node_info = ov::NodeAccessor(node).get_shared_info();
        // As two Functions owns the same node so node will have two shared_info objects
        ASSERT_EQ(node_info.size(), 2);
        ASSERT_TRUE(node_info.count(f1_shared_info));
        ASSERT_TRUE(node_info.count(f2_shared_info));
    }

    relu0->add_control_dependency(arg0);  // simply drop cache
    ASSERT_FALSE(f1_shared_info->get_use_topological_cache());
    ASSERT_FALSE(f2_shared_info->get_use_topological_cache());
}
