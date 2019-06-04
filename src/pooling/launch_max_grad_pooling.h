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

#ifndef SYCLDNN_SRC_POOLING_LAUNCH_MAX_GRAD_POOLING_H_
#define SYCLDNN_SRC_POOLING_LAUNCH_MAX_GRAD_POOLING_H_

#include "sycldnn/pooling/params.h"
#include "sycldnn/pooling/sizes.h"

#include "sycldnn/internal/pooling/launch_internal.h"

#include "src/pooling/can_fastdiv.h"
#include "src/pooling/can_vectorize.h"
#include "src/pooling/kernels.h"

namespace sycldnn {
namespace pooling {
namespace internal {

template <typename T, typename Index, template <typename U> class PoolType,
          typename Direction, int VectorWidth, bool UseFastDiv>
SNNStatus queue_pooling(BaseMemObject<T const>& input_mem,
                        BaseMemObject<T const>& output_mem,
                        BaseMemObject<T const>& input_backprop_mem,
                        BaseMemObject<T>& output_backprop_mem,
                        const PoolingParams& pp, size_t threads,
                        cl::sycl::queue& queue) {
  auto event = queue.submit([&](cl::sycl::handler& cgh) {
    auto input_data = input_mem.read_accessor(cgh);
    auto output_data = output_mem.read_accessor(cgh);
    auto input_backprop = input_backprop_mem.read_accessor(cgh);
    auto output_backprop = output_backprop_mem.write_accessor(cgh);

    PoolingOp<T, Index, PoolType, Direction, VectorWidth, UseFastDiv> pool{
        input_data, output_data, input_backprop, output_backprop, pp};

    cgh.parallel_for(cl::sycl::range<1>{threads}, pool);
  });

  return {event, StatusCode::OK};
}

template <typename T, typename Index, template <typename> class PoolType,
          typename Direction, int VectorWidth>
SNNStatus launch_with_vector_size(BaseMemObject<T const>& inp_data,
                                  BaseMemObject<T const>& outp_data,
                                  BaseMemObject<T const>& inp_backprop,
                                  BaseMemObject<T>& outp_backprop,
                                  const PoolingParams& pp, size_t threads,
                                  cl::sycl::queue& queue) {
  threads /= VectorWidth;
  if (can_use_fastdiv<Direction>(pp, VectorWidth)) {
    return queue_pooling<T, Index, PoolType, Direction, VectorWidth, true>(
        inp_data, outp_data, inp_backprop, outp_backprop, pp, threads, queue);
  } else {
    return queue_pooling<T, Index, PoolType, Direction, VectorWidth, false>(
        inp_data, outp_data, inp_backprop, outp_backprop, pp, threads, queue);
  }
}

template <typename T, typename Index, template <typename> class PoolType,
          typename Direction>
SNNStatus launch_with_index(BaseMemObject<T const>& inp_data,
                            BaseMemObject<T const>& outp_data,
                            BaseMemObject<T const>& inp_backprop,
                            BaseMemObject<T>& outp_backprop,
                            const PoolingParams& pp, size_t threads,
                            cl::sycl::queue& queue) {
  if (can_vectorize<Direction, PoolType>(pp, 4)) {
    return launch_with_vector_size<T, Index, PoolType, Direction, 4>(
        inp_data, outp_data, inp_backprop, outp_backprop, pp, threads, queue);
  } else if (can_vectorize<Direction, PoolType>(pp, 2)) {
    return launch_with_vector_size<T, Index, PoolType, Direction, 2>(
        inp_data, outp_data, inp_backprop, outp_backprop, pp, threads, queue);
  } else {
    return launch_with_vector_size<T, Index, PoolType, Direction, 1>(
        inp_data, outp_data, inp_backprop, outp_backprop, pp, threads, queue);
  }
}

template <typename T, template <typename> class PoolType, typename Direction,
          EnableIfMaxGradient<T, PoolType, Direction>>
SNNStatus launch_pooling(BaseMemObject<T const>& inp_data,
                         BaseMemObject<T const>& outp_data,
                         BaseMemObject<T const>& inp_backprop,
                         BaseMemObject<T>& outp_backprop,
                         const PoolingParams& pp, cl::sycl::queue& queue) {
  auto sizes = get_sizes<Direction>(pp);
  size_t threads = sizes.output_size;
  if (threads > std::numeric_limits<int32_t>::max()) {
#ifdef SNN_USE_INT64
    return launch_with_index<T, int64_t, PoolType, Direction>(
        inp_data, outp_data, inp_backprop, outp_backprop, pp, threads, queue);
#else
    SNNStatus index_too_large;
    index_too_large.status = StatusCode::IndexExceeded;
    return index_too_large;
#endif  // SNN_USE_INT64
  } else {
    return launch_with_index<T, int32_t, PoolType, Direction>(
        inp_data, outp_data, inp_backprop, outp_backprop, pp, threads, queue);
  }
}

}  // namespace internal
}  // namespace pooling
}  // namespace sycldnn

#endif  // SYCLDNN_SRC_POOLING_LAUNCH_MAX_GRAD_POOLING_H_
