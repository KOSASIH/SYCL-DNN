/*
 * Copyright 2018 Codeplay Software Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use these files except in compliance with the License.
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
#include "arm_fixture.h"
#include "vgg_param_set.h"

#define VGG_BENCHMARK(N, C, W, H, F)                           \
  CONVOLUTION_BENCHMARK(                                       \
      ARM_Forward_##N##_##C##_##W##_##H##_##Flt##_##S##_##Ftr, \
      ParameterSet<N, C, W, H, F>, sycldnn::conv2d::conv_type::Forward)

#define VGG_PARAMS(channels, width, height, features) \
  VGG_BENCHMARK(1, channels, width, height, features);
#include "bench/conv2d/vgg_params.def"
#undef VGG_PARAMS

#define VGG_PARAMS(channels, width, height, features) \
  VGG_BENCHMARK(32, channels, width, height, features);
#include "bench/conv2d/vgg_params.def"
#undef VGG_PARAMS

#define VGG_PARAMS(channels, width, height, features) \
  VGG_BENCHMARK(64, channels, width, height, features);
#include "bench/conv2d/vgg_params.def"
#undef VGG_PARAMS