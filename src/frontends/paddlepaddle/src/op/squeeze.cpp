// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <node_context.hpp>

#include "openvino/opsets/opset6.hpp"

namespace ov {
namespace frontend {
namespace pdpd {
namespace op {
NamedOutputs squeeze(const NodeContext& node) {
    auto data = node.get_ng_input("X");
    std::vector<int32_t> axes;
    if (node.has_attribute<std::vector<int32_t>>("axes")) {
        axes = node.get_attribute<std::vector<int32_t>>("axes");
    }

    std::shared_ptr<Node> out;
    if (!axes.empty()) {
        auto axesNode = ov::opset6::Constant::create(ov::element::i32, {axes.size()}, axes);
        out = std::make_shared<ov::opset6::Squeeze>(data, axesNode);
    } else {
        out = std::make_shared<ov::opset6::Squeeze>(data);
    }
    return node.default_single_output_mapping(out, {"Out"});
}
}  // namespace op
}  // namespace pdpd
}  // namespace frontend
}  // namespace ov
