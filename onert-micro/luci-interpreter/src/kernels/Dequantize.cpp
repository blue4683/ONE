/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DIS_QUANT

#include "Builders.h"
#include "kernels/Utils.h"
#include "SISOKernel.h"

#include "PALDequantize.h"

namespace luci_interpreter
{

void configure_kernel_CircleDequantize(const circle::Operator *cur_op,
                                       BaseRuntimeGraph *runtime_graph)
{
  kernels::SISOKernel kernel(cur_op, runtime_graph);

  LUCI_INTERPRETER_CHECK(Tensor::num_elements(kernel.input()) ==
                         Tensor::num_elements(kernel.output()));
  LUCI_INTERPRETER_CHECK(Tensor::num_dims(kernel.input()) == Tensor::num_dims(kernel.output()));
  LUCI_INTERPRETER_CHECK(!Tensor::scales(kernel.input()).empty());
  LUCI_INTERPRETER_CHECK(!Tensor::zero_points(kernel.input()).empty());
}

void execute_kernel_CircleDequantize(const circle::Operator *cur_op,
                                     BaseRuntimeGraph *runtime_graph)
{
  kernels::SISOKernel kernel(cur_op, runtime_graph);

  const auto *input_data = runtime_graph->getDataByTensor(kernel.input());
  assert(input_data);

  auto *output_data = runtime_graph->getDataByTensor(kernel.output());
  assert(output_data);

  const int flat_size = kernels::getTensorRuntimeShape(kernel.input(), runtime_graph).flatSize();

  switch (Tensor::element_type(kernel.output()))
  {
#ifndef DIS_FLOAT
    case DataType::FLOAT32:
    {
      luci_interpreter_pal::QuantizationParams params{};
      params.zero_point = Tensor::zero_point(kernel.input());
      params.scale = Tensor::scale(kernel.input());
      switch (Tensor::element_type(kernel.input()))
      {
        case DataType::S8:
          luci_interpreter_pal::Dequantize(params, flat_size,
                                           kernels::getTensorData<int8_t>(input_data),
                                           kernels::getTensorData<float>(output_data));
          break;
        case DataType::U8:
          luci_interpreter_pal::Dequantize(params, flat_size,
                                           kernels::getTensorData<uint8_t>(input_data),
                                           kernels::getTensorData<float>(output_data));
          break;
        case DataType::S16:
          luci_interpreter_pal::Dequantize(params, flat_size,
                                           kernels::getTensorData<int16_t>(input_data),
                                           kernels::getTensorData<float>(output_data));
          break;
        default:
          assert(false && "Unsupported type");
      }
      break;
    }
#endif // DIS_FLOAT
    default:
      assert(false && "Unsupported type");
  }
}

} // namespace luci_interpreter

#endif // DIS_QUANT
