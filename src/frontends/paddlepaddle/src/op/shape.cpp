// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <node_context.hpp>

#include "openvino/opsets/opset6.hpp"

namespace ov {
namespace frontend {
namespace pdpd {
namespace op {
NamedOutputs shape(const NodeContext& node) {
    auto data = node.get_ng_input("Input");
    auto shape_node = std::make_shared<ov::opset6::ShapeOf>(data, element::i32);
    return node.default_single_output_mapping({shape_node}, {"Out"});
}

}  // namespace op
}  // namespace pdpd
}  // namespace frontend
}  // namespace ov
