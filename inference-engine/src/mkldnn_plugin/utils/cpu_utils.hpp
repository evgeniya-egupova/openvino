// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

namespace MKLDNNPlugin {

/**
* @brief Returns normalized by size dims where missing dimensions are filled with units from the beginning
* Example: dims = {2, 3, 5}; ndims = 5; result = {1, 1, 2, 3, 5}
* @param dims
* shape to normalize
* @param ndims
* rank of resulting shape
* @return normalized vector
*/
inline std::vector<size_t> getNormalizedDimsBySize(const InferenceEngine::SizeVector &dims, size_t ndims) {
    if (dims.size() >= ndims)
        return dims;

    std::vector<size_t> normalizedDims = dims;
    for (size_t i = 0; i < (ndims - dims.size()); i++) {
        normalizedDims.insert(normalizedDims.begin(), 1);
    }
    return normalizedDims;
}

/**
* @brief Checked that secondInputDims unidirectional broadcastable per tensor or per channel to firstInputDims
* @param firstInputDims
* shape on which should be broadcastable
* @param secondInputDims
* shape which should be broadcastable
* @param weakComparison
* flag which specify how we compare C dims if value is undefined (weak or strong)
* @return true if broadcastable, false otherwise.
*/
inline bool isPerTensorOrPerChannelBroadcastable(const InferenceEngine::SizeVector &firstInputDims, const InferenceEngine::SizeVector& secondInputDims,
                                                 bool weakComparison = false) {
    bool (*dimsEqual)(size_t, size_t) = weakComparison ? static_cast<bool (*)(size_t, size_t)>(dimsEqualWeak) :
                                                         static_cast<bool (*)(size_t, size_t)>(dimsEqualStrong);
    if (secondInputDims.size() > firstInputDims.size())
        return false;
    if (std::accumulate(secondInputDims.begin(), secondInputDims.end(), 1, std::multiplies<size_t>()) == 1)
        return true;

    std::vector<size_t> normalizedSecondInputDims = getNormalizedDimsBySize(secondInputDims, firstInputDims.size());
    for (size_t i = 0; i < normalizedSecondInputDims.size(); i++) {
        if ((i == 1 && !dimsEqual(normalizedSecondInputDims[i], firstInputDims[1])) || (i != 1 && normalizedSecondInputDims[i] != 1))
            return false;
    }
    return true;
}

inline bool isEmptyTensorDesc(const InferenceEngine::TensorDesc &td) {
    const auto dims = td.getDims();
    return std::any_of(dims.begin(), dims.end(), [](size_t dim) { return dim == 0; } );
}

/**
* @brief Return precision to which given precision must be converted to be supported in plug-in
* @param precision
* precision for convert
* @return plug-in supported precision or UNSPECIFIED if precision unsupported
*/
inline InferenceEngine::Precision normalizeToSupportedPrecision(InferenceEngine::Precision precision) {
    switch (precision) {
        case InferenceEngine::Precision::U8:
        case InferenceEngine::Precision::I8:
        case InferenceEngine::Precision::I32:
        case InferenceEngine::Precision::BF16:
        case InferenceEngine::Precision::FP32: {
            break;
        }
        case InferenceEngine::Precision::FP64: {
            precision = InferenceEngine::Precision::FP32;
            break;
        }
        case InferenceEngine::Precision::BOOL: {
            precision = InferenceEngine::Precision::U8;
            break;
        }
        case InferenceEngine::Precision::U16:
        case InferenceEngine::Precision::I16:
        case InferenceEngine::Precision::U32:
        case InferenceEngine::Precision::I64:
        case InferenceEngine::Precision::U64: {
            precision = InferenceEngine::Precision::I32;
            break;
        }
        case InferenceEngine::Precision::FP16: {
            precision = InferenceEngine::Precision::FP32;
            break;
        }
        default: {
            precision = InferenceEngine::Precision::UNSPECIFIED;
        }
    }
    return precision;
}

/**
* @brief Return aligned buffer by targetSize.
* If buffer has size 1, values are broadcasted with targetSize size.
* If aligned buffer size > targetSize, other values filled by zero.
* @param targetSize
* target size buffer
* @param buffer
* buffer to be aligned
* @param align
* alignment for targetSize
* @return aligned buffer
*/
inline std::vector<float> makeAlignedBuffer(size_t targetSize, const std::vector<float> &buffer, int align = -1) {
    if (buffer.empty()) {
        IE_THROW() << "Can't align buffer, becuase buffer is empty";
    }

    auto alignedBuffer = buffer;
    if (align == -1) {
        align = targetSize;
    }
    const size_t bufferSizeAligned = rnd_up(targetSize, align);

    alignedBuffer.resize(bufferSizeAligned, 0);
    if (buffer.size() == 1) {
        std::fill(alignedBuffer.begin() + 1, alignedBuffer.begin() + targetSize, buffer[0]);
    }
    return alignedBuffer;
}

}  // namespace MKLDNNPlugin
