/*
 * Copyright 2018 Codeplay Software Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SYCLDNN_SRC_CONV2D_WINOGRAD_QUEUE_FILTER_TRANSFORM_IMPL_H_
#define SYCLDNN_SRC_CONV2D_WINOGRAD_QUEUE_FILTER_TRANSFORM_IMPL_H_

#include "sycldnn/mem_object.h"

#include "src/conv2d/winograd/queue_filter_transform.h"

#include "src/conv2d/winograd/kernels/extract_filter_transform.h"

namespace sycldnn {
namespace conv2d {
namespace internal {
namespace winograd {

namespace {

/** Round up a value to the nearest multiple of 4. */
auto round_up = [](int val) {
  constexpr int pow_two_multiple = 4;
  return helpers::round_up_to_nearest_multiple(val, pow_two_multiple);
};

/** Get the number of threads required for a given convolution. */
template <typename ConvType>
inline cl::sycl::range<1> get_thread_range(Conv2DParams const& params,
                                           TileInfo const& /*unused*/) {
  size_t n_threads = round_up(params.channels * params.features);
  return cl::sycl::range<1>{n_threads};
}

/**
 * Get the number of threads required for a given filter backprop convolution.
 */
template <>
inline cl::sycl::range<1> get_thread_range<conv_type::FilterBackprop>(
    Conv2DParams const& params, TileInfo const& tile_info) {
  size_t n_threads = round_up(params.batch * tile_info.rows * tile_info.cols *
                              params.features);
  return cl::sycl::range<1>{n_threads};
}

}  // namespace

template <typename T, typename Index, typename ConvType, int M, int N, int R,
          int S>
SNNStatus queue_filter_transform(BaseMemObject<T const>& filter_mem,
                                 BaseMemObject<T>& transform_mem,
                                 Conv2DParams const& params,
                                 TileInfo const& tile_info,
                                 cl::sycl::queue& queue) {
  using Functor = ExtractFilterTiles<T, Index, M, N, R, S, ConvType>;

  auto event = queue.submit([&](cl::sycl::handler& cgh) {
    auto filter = filter_mem.read_accessor(cgh);
    auto transform = transform_mem.write_accessor(cgh);
    auto range = get_thread_range<ConvType>(params, tile_info);
    Functor conv{params, tile_info, filter, transform};

    cgh.parallel_for(range, conv);
  });
  return SNNStatus{event, StatusCode::OK};
}

}  // namespace winograd
}  // namespace internal
}  // namespace conv2d
}  // namespace sycldnn

#endif  // SYCLDNN_SRC_CONV2D_WINOGRAD_QUEUE_FILTER_TRANSFORM_IMPL_H_
