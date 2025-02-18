// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <pybind11/pybind11.h>

namespace py = pybind11;

void regclass_frontend_FrontEnd(py::module m);
void regclass_frontend_Extension(py::module m);
void regclass_frontend_TelemetryExtension(py::module m);
