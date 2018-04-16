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
#ifndef SYCLDNN_TEST_TYPES_TYPE_PAIR_H_
#define SYCLDNN_TEST_TYPES_TYPE_PAIR_H_

namespace sycldnn {
namespace types {
template <typename First, typename Second>
struct TypePair {
  using FirstType = First;
  using SecondType = Second;
};
}  // namespace types
}  // namespace sycldnn
#endif  // SYCLDNN_TEST_TYPES_TYPE_PAIR_H_
