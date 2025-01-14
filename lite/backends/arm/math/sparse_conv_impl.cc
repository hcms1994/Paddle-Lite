// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lite/backends/arm/math/sparse_conv_impl.h"
#include <arm_neon.h>
#include <vector>
#include "lite/core/parallel_defines.h"

namespace paddle {
namespace lite {
namespace arm {
namespace math {

#ifdef __aarch64__

#define SPARSE_F32_F32_W48_V8_KERNEL        \
  "dup     v20.4s,  %w[vbias]\n"            \
  "dup     v21.4s,  v20.s[0]\n"             \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "dup     v22.4s,  v20.s[0]\n"             \
  "dup     v23.4s,  v20.s[0]\n"             \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "dup     v24.4s,  v20.s[0]\n"             \
  "dup     v25.4s,  v20.s[0]\n"             \
  "dup     v26.4s,  v20.s[0]\n"             \
  "prfm  pldl1keep, [%[b_ptr], #192]\n"     \
  "dup     v27.4s,  v20.s[0]\n"             \
  "dup     v28.4s,  v20.s[0]\n"             \
  "dup     v29.4s,  v20.s[0]\n"             \
  "dup     v30.4s,  v20.s[0]\n"             \
  "dup     v31.4s,  v20.s[0]\n"             \
  "cbz    %w[k],    1f\n"                   \
  "cbz    %w[n],    3f\n"                   \
  "0:\n"                                    \
  "ldr   q0, [%[a_ptr]], #16\n"             \
  "ldr   q1, [%[widx_dmap]],   #16\n"       \
  "mov   w1, v1.s[0]\n"                     \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "sxtw  x1,  w1\n"                         \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "subs    %w[n],   %w[n],   #1\n"          \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "fmla    v20.4s,  v2.4s,  v0.s[0]\n"      \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "fmla    v21.4s,  v3.4s,  v0.s[0]\n"      \
  "ldp   q10, q11, [%[b_ptr], #128]\n"      \
  "fmla    v22.4s,  v4.4s,  v0.s[0]\n"      \
  "ldp   q12, q13, [%[b_ptr], #160]\n"      \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr], #192]\n"     \
  "fmla    v23.4s,  v5.4s,  v0.s[0]\n"      \
  "fmla    v24.4s,  v6.4s,  v0.s[0]\n"      \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "fmla    v25.4s,  v7.4s,  v0.s[0]\n"      \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "fmla    v26.4s,  v8.4s,  v0.s[0]\n"      \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "fmla    v27.4s,  v9.4s,  v0.s[0]\n"      \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "fmla    v28.4s,  v10.4s,  v0.s[0]\n"     \
  "fmla    v29.4s,  v11.4s,  v0.s[0]\n"     \
  "ldp   q10, q11, [%[b_ptr], #128]\n"      \
  "fmla    v30.4s,  v12.4s,  v0.s[0]\n"     \
  "fmla    v31.4s,  v13.4s,  v0.s[0]\n"     \
  "ldp   q12, q13, [%[b_ptr], #160]\n"      \
  "mov   w1, v1.s[1]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr], #192]\n"     \
  "fmla    v20.4s,  v2.4s,  v0.s[1]\n"      \
  "fmla    v21.4s,  v3.4s,  v0.s[1]\n"      \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "fmla    v22.4s,  v4.4s,  v0.s[1]\n"      \
  "fmla    v23.4s,  v5.4s,  v0.s[1]\n"      \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "fmla    v24.4s,  v6.4s,  v0.s[1]\n"      \
  "fmla    v25.4s,  v7.4s,  v0.s[1]\n"      \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "fmla    v26.4s,  v8.4s,  v0.s[1]\n"      \
  "fmla    v27.4s,  v9.4s,  v0.s[1]\n"      \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "fmla    v28.4s,  v10.4s,  v0.s[1]\n"     \
  "fmla    v29.4s,  v11.4s,  v0.s[1]\n"     \
  "ldp   q10, q11, [%[b_ptr], #128]\n"      \
  "fmla    v30.4s,  v12.4s,  v0.s[1]\n"     \
  "fmla    v31.4s,  v13.4s,  v0.s[1]\n"     \
  "ldp   q12, q13, [%[b_ptr], #160]\n"      \
  "mov   w1, v1.s[2]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr], #192]\n"     \
  "fmla    v20.4s,  v2.4s,  v0.s[2]\n"      \
  "fmla    v21.4s,  v3.4s,  v0.s[2]\n"      \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "fmla    v22.4s,  v4.4s,  v0.s[2]\n"      \
  "fmla    v23.4s,  v5.4s,  v0.s[2]\n"      \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "fmla    v24.4s,  v6.4s,  v0.s[2]\n"      \
  "fmla    v25.4s,  v7.4s,  v0.s[2]\n"      \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "fmla    v26.4s,  v8.4s,  v0.s[2]\n"      \
  "fmla    v27.4s,  v9.4s,  v0.s[2]\n"      \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "fmla    v28.4s,  v10.4s,  v0.s[2]\n"     \
  "fmla    v29.4s,  v11.4s,  v0.s[2]\n"     \
  "ldp   q10, q11, [%[b_ptr], #128]\n"      \
  "fmla    v30.4s,  v12.4s,  v0.s[2]\n"     \
  "fmla    v31.4s,  v13.4s,  v0.s[2]\n"     \
  "ldp   q12, q13, [%[b_ptr], #160]\n"      \
  "mov   w1, v1.s[3]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "fmla    v20.4s,  v2.4s,  v0.s[3]\n"      \
  "fmla    v21.4s,  v3.4s,  v0.s[3]\n"      \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "fmla    v22.4s,  v4.4s,  v0.s[3]\n"      \
  "fmla    v23.4s,  v5.4s,  v0.s[3]\n"      \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "fmla    v24.4s,  v6.4s,  v0.s[3]\n"      \
  "fmla    v25.4s,  v7.4s,  v0.s[3]\n"      \
  "fmla    v26.4s,  v8.4s,  v0.s[3]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #192]\n"     \
  "fmla    v27.4s,  v9.4s,  v0.s[3]\n"      \
  "fmla    v28.4s,  v10.4s,  v0.s[3]\n"     \
  "fmla    v29.4s,  v11.4s,  v0.s[3]\n"     \
  "fmla    v30.4s,  v12.4s,  v0.s[3]\n"     \
  "fmla    v31.4s,  v13.4s,  v0.s[3]\n"     \
  "bne     0b\n"                            \
  "3:\n"                                    \
  "cbz    %w[m],    1f\n"                   \
  "ldr   q0, [%[a_ptr]], #16\n"             \
  "ldr   q1, [%[widx_dmap]],   #16\n"       \
  "mov   w1, v1.s[0]\n"                     \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "sxtw  x1,  w1\n"                         \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "subs  %w[m],   %w[m],   #1\n"            \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "fmla    v20.4s,  v2.4s,  v0.s[0]\n"      \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "fmla    v21.4s,  v3.4s,  v0.s[0]\n"      \
  "ldp   q10, q11, [%[b_ptr], #128]\n"      \
  "fmla    v22.4s,  v4.4s,  v0.s[0]\n"      \
  "ldp   q12, q13, [%[b_ptr], #160]\n"      \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "fmla    v23.4s,  v5.4s,  v0.s[0]\n"      \
  "fmla    v24.4s,  v6.4s,  v0.s[0]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #192]\n"     \
  "fmla    v25.4s,  v7.4s,  v0.s[0]\n"      \
  "fmla    v26.4s,  v8.4s,  v0.s[0]\n"      \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "fmla    v27.4s,  v9.4s,  v0.s[0]\n"      \
  "fmla    v28.4s,  v10.4s,  v0.s[0]\n"     \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "fmla    v29.4s,  v11.4s,  v0.s[0]\n"     \
  "fmla    v30.4s,  v12.4s,  v0.s[0]\n"     \
  "fmla    v31.4s,  v13.4s,  v0.s[0]\n"     \
  "beq     1f\n"                            \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "mov   w1, v1.s[1]\n"                     \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "sxtw  x1,  w1\n"                         \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "subs  %w[m],   %w[m],   #1\n"            \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "fmla    v20.4s,  v2.4s,  v0.s[1]\n"      \
  "ldp   q10, q11, [%[b_ptr], #128]\n"      \
  "fmla    v21.4s,  v3.4s,  v0.s[1]\n"      \
  "ldp   q12, q13, [%[b_ptr], #160]\n"      \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "fmla    v22.4s,  v4.4s,  v0.s[1]\n"      \
  "fmla    v23.4s,  v5.4s,  v0.s[1]\n"      \
  "fmla    v24.4s,  v6.4s,  v0.s[1]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #192]\n"     \
  "fmla    v25.4s,  v7.4s,  v0.s[1]\n"      \
  "fmla    v26.4s,  v8.4s,  v0.s[1]\n"      \
  "fmla    v27.4s,  v9.4s,  v0.s[1]\n"      \
  "fmla    v28.4s,  v10.4s,  v0.s[1]\n"     \
  "fmla    v29.4s,  v11.4s,  v0.s[1]\n"     \
  "fmla    v30.4s,  v12.4s,  v0.s[1]\n"     \
  "fmla    v31.4s,  v13.4s,  v0.s[1]\n"     \
  "beq     1f\n"                            \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "mov   w1, v1.s[2]\n"                     \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "sxtw  x1,  w1\n"                         \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "fmla    v20.4s,  v2.4s,  v0.s[2]\n"      \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "fmla    v21.4s,  v3.4s,  v0.s[2]\n"      \
  "ldp   q10, q11, [%[b_ptr], #128]\n"      \
  "fmla    v22.4s,  v4.4s,  v0.s[2]\n"      \
  "ldp   q12, q13, [%[b_ptr], #160]\n"      \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "fmla    v23.4s,  v5.4s,  v0.s[2]\n"      \
  "fmla    v24.4s,  v6.4s,  v0.s[2]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #192]\n"     \
  "fmla    v25.4s,  v7.4s,  v0.s[2]\n"      \
  "fmla    v26.4s,  v8.4s,  v0.s[2]\n"      \
  "fmla    v27.4s,  v9.4s,  v0.s[2]\n"      \
  "fmla    v28.4s,  v10.4s,  v0.s[2]\n"     \
  "fmla    v29.4s,  v11.4s,  v0.s[2]\n"     \
  "fmla    v30.4s,  v12.4s,  v0.s[2]\n"     \
  "fmla    v31.4s,  v13.4s,  v0.s[2]\n"     \
  "1:\n"

#define SPARSE_F32_F32_W32_V8_KERNEL        \
  "dup     v21.4s,  %w[vbias]\n"            \
  "dup     v22.4s,  v21.s[0]\n"             \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "dup     v23.4s,  v21.s[0]\n"             \
  "dup     v24.4s,  v21.s[0]\n"             \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "dup     v25.4s,  v21.s[0]\n"             \
  "dup     v26.4s,  v21.s[0]\n"             \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "dup     v27.4s,  v21.s[0]\n"             \
  "dup     v28.4s,  v21.s[0]\n"             \
  "cbz    %w[k],    1f\n"                   \
  "cbz    %w[n],    3f\n"                   \
  "0:\n"                                    \
  "ldr   q0, [%[a_ptr]], #16\n"             \
  "ldr   q1, [%[widx_dmap]],   #16\n"       \
  "mov   w1, v1.s[0]\n"                     \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "sxtw  x1,  w1\n"                         \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "subs    %w[n],   %w[n],   #1\n"          \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "fmla    v21.4s,  v2.4s,  v0.s[0]\n"      \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v22.4s,  v3.4s,  v0.s[0]\n"      \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "fmla    v23.4s,  v4.4s,  v0.s[0]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[0]\n"      \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "fmla    v25.4s,  v6.4s,  v0.s[0]\n"      \
  "fmla    v26.4s,  v7.4s,  v0.s[0]\n"      \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "fmla    v27.4s,  v8.4s,  v0.s[0]\n"      \
  "fmla    v28.4s,  v9.4s,  v0.s[0]\n"      \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "mov   w1, v1.s[1]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[1]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[1]\n"      \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "fmla    v23.4s,  v4.4s,  v0.s[1]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[1]\n"      \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "fmla    v25.4s,  v6.4s,  v0.s[1]\n"      \
  "fmla    v26.4s,  v7.4s,  v0.s[1]\n"      \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "fmla    v27.4s,  v8.4s,  v0.s[1]\n"      \
  "fmla    v28.4s,  v9.4s,  v0.s[1]\n"      \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "mov   w1, v1.s[2]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[2]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[2]\n"      \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "fmla    v23.4s,  v4.4s,  v0.s[2]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[2]\n"      \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "fmla    v25.4s,  v6.4s,  v0.s[2]\n"      \
  "fmla    v26.4s,  v7.4s,  v0.s[2]\n"      \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "fmla    v27.4s,  v8.4s,  v0.s[2]\n"      \
  "fmla    v28.4s,  v9.4s,  v0.s[2]\n"      \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "mov   w1, v1.s[3]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[3]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[3]\n"      \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "fmla    v23.4s,  v4.4s,  v0.s[3]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[3]\n"      \
  "fmla    v25.4s,  v6.4s,  v0.s[3]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v26.4s,  v7.4s,  v0.s[3]\n"      \
  "fmla    v27.4s,  v8.4s,  v0.s[3]\n"      \
  "fmla    v28.4s,  v9.4s,  v0.s[3]\n"      \
  "bne     0b\n"                            \
  "3:\n"                                    \
  "cbz    %w[m],    1f\n"                   \
  "ldr   q0, [%[a_ptr]], #16\n"             \
  "ldr   q1, [%[widx_dmap]],   #16\n"       \
  "mov   w1, v1.s[0]\n"                     \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "sxtw  x1,  w1\n"                         \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "subs  %w[m],   %w[m],   #1\n"            \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "fmla    v21.4s,  v2.4s,  v0.s[0]\n"      \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "fmla    v22.4s,  v3.4s,  v0.s[0]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v23.4s,  v4.4s,  v0.s[0]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[0]\n"      \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "fmla    v25.4s,  v6.4s,  v0.s[0]\n"      \
  "fmla    v26.4s,  v7.4s,  v0.s[0]\n"      \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "fmla    v27.4s,  v8.4s,  v0.s[0]\n"      \
  "fmla    v28.4s,  v9.4s,  v0.s[0]\n"      \
  "beq     1f\n"                            \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "mov   w1, v1.s[1]\n"                     \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "sxtw  x1,  w1\n"                         \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "subs  %w[m],   %w[m],   #1\n"            \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "fmla    v21.4s,  v2.4s,  v0.s[1]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[1]\n"      \
  "fmla    v23.4s,  v4.4s,  v0.s[1]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[1]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v25.4s,  v6.4s,  v0.s[1]\n"      \
  "fmla    v26.4s,  v7.4s,  v0.s[1]\n"      \
  "fmla    v27.4s,  v8.4s,  v0.s[1]\n"      \
  "fmla    v28.4s,  v9.4s,  v0.s[1]\n"      \
  "beq     1f\n"                            \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "mov   w1, v1.s[2]\n"                     \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "sxtw  x1,  w1\n"                         \
  "ldp   q6, q7, [%[b_ptr], #64]\n"         \
  "fmla    v21.4s,  v2.4s,  v0.s[2]\n"      \
  "ldp   q8, q9, [%[b_ptr], #96]\n"         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "fmla    v22.4s,  v3.4s,  v0.s[2]\n"      \
  "fmla    v23.4s,  v4.4s,  v0.s[2]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[2]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v25.4s,  v6.4s,  v0.s[2]\n"      \
  "fmla    v26.4s,  v7.4s,  v0.s[2]\n"      \
  "fmla    v27.4s,  v8.4s,  v0.s[2]\n"      \
  "fmla    v28.4s,  v9.4s,  v0.s[2]\n"      \
  "1:\n"

#define SPARSE_F32_F32_W16_V8_KERNEL        \
  "dup     v21.4s,  %w[vbias]\n"            \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "dup     v22.4s,  v21.s[0]\n"             \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "dup     v23.4s,  v21.s[0]\n"             \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "dup     v24.4s,  v21.s[0]\n"             \
  "cbz    %w[k],    1f\n"                   \
  "cbz    %w[n],    3f\n"                   \
  "0:\n"                                    \
  "ldr   q0, [%[a_ptr]], #16\n"             \
  "ldr   q1, [%[widx_dmap]],   #16\n"       \
  "mov   w1, v1.s[0]\n"                     \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "sxtw  x1,  w1\n"                         \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "subs    %w[n],   %w[n],   #1\n"          \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[0]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[0]\n"      \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "fmla    v23.4s,  v4.4s,  v0.s[0]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[0]\n"      \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "mov   w1, v1.s[1]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[1]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[1]\n"      \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "fmla    v23.4s,  v4.4s,  v0.s[1]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[1]\n"      \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "mov   w1, v1.s[2]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[2]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[2]\n"      \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "fmla    v23.4s,  v4.4s,  v0.s[2]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[2]\n"      \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "mov   w1, v1.s[3]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[3]\n"      \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "fmla    v22.4s,  v3.4s,  v0.s[3]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v23.4s,  v4.4s,  v0.s[3]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[3]\n"      \
  "bne     0b\n"                            \
  "3:\n"                                    \
  "cbz    %w[m],    1f\n"                   \
  "ldr   q0, [%[a_ptr]], #16\n"             \
  "ldr   q1, [%[widx_dmap]],   #16\n"       \
  "mov   w1, v1.s[0]\n"                     \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "sxtw  x1,  w1\n"                         \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "subs  %w[m],   %w[m],   #1\n"            \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "fmla    v21.4s,  v2.4s,  v0.s[0]\n"      \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "fmla    v22.4s,  v3.4s,  v0.s[0]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v23.4s,  v4.4s,  v0.s[0]\n"      \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "fmla    v24.4s,  v5.4s,  v0.s[0]\n"      \
  "beq     1f\n"                            \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "mov   w1, v1.s[1]\n"                     \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "subs  %w[m],   %w[m],   #1\n"            \
  "fmla    v21.4s,  v2.4s,  v0.s[1]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[1]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v23.4s,  v4.4s,  v0.s[1]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[1]\n"      \
  "beq     1f\n"                            \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "mov   w1, v1.s[2]\n"                     \
  "ldp   q4, q5, [%[b_ptr], #32]\n"         \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "fmla    v21.4s,  v2.4s,  v0.s[2]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[2]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v23.4s,  v4.4s,  v0.s[2]\n"      \
  "fmla    v24.4s,  v5.4s,  v0.s[2]\n"      \
  "1:\n"

#define SPARSE_F32_F32_W8_V8_KERNEL         \
  "dup     v21.4s,  %w[vbias]\n"            \
  "dup     v22.4s,  v21.s[0]\n"             \
  "cbz    %w[k],    1f\n"                   \
  "cbz    %w[n],    3f\n"                   \
  "0:\n"                                    \
  "ldr   q0, [%[a_ptr]], #16\n"             \
  "ldr   q1, [%[widx_dmap]],   #16\n"       \
  "mov   w1, v1.s[0]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "subs    %w[n],   %w[n],   #1\n"          \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[0]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[0]\n"      \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "mov   w1, v1.s[1]\n"                     \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[1]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[1]\n"      \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "mov   w1, v1.s[2]\n"                     \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[2]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[2]\n"      \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "mov   w1, v1.s[3]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[3]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[3]\n"      \
  "bne     0b\n"                            \
  "3:\n"                                    \
  "cbz    %w[m],    1f\n"                   \
  "ldr   q0, [%[a_ptr]], #16\n"             \
  "ldr   q1, [%[widx_dmap]],   #16\n"       \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "subs  %w[m],   %w[m],   #1\n"            \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "mov   w1, v1.s[0]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "fmla    v21.4s,  v2.4s,  v0.s[0]\n"      \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v22.4s,  v3.4s,  v0.s[0]\n"      \
  "beq     1f\n"                            \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "mov   w1, v1.s[1]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "subs  %w[m],   %w[m],   #1\n"            \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[1]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[1]\n"      \
  "beq     1f\n"                            \
  "ldp   q2, q3, [%[b_ptr]]\n"              \
  "mov   w1, v1.s[2]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[2]\n"      \
  "fmla    v22.4s,  v3.4s,  v0.s[2]\n"      \
  "1:\n"

#define SPARSE_F32_F32_W4_V8_KERNEL         \
  "dup     v21.4s,  %w[vbias]\n"            \
  "cbz    %w[k],    1f\n"                   \
  "cbz    %w[n],    3f\n"                   \
  "0:\n"                                    \
  "ldr   q0, [%[a_ptr]], #16\n"             \
  "ldr   q1, [%[widx_dmap]],   #16\n"       \
  "mov   w1, v1.s[0]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "ldr   q2, [%[b_ptr]]\n"                  \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr]]\n"           \
  "subs    %w[n],   %w[n],   #1\n"          \
  "fmla    v21.4s,  v2.4s,  v0.s[0]\n"      \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "ldr   q2, [%[b_ptr]]\n"                  \
  "mov   w1, v1.s[1]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr]]\n"           \
  "fmla    v21.4s,  v2.4s,  v0.s[1]\n"      \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "ldr   q2, [%[b_ptr]]\n"                  \
  "mov   w1, v1.s[2]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr]]\n"           \
  "fmla    v21.4s,  v2.4s,  v0.s[2]\n"      \
  "ldr   q2, [%[b_ptr]]\n"                  \
  "mov   w1, v1.s[3]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "fmla    v21.4s,  v2.4s,  v0.s[3]\n"      \
  "bne     0b\n"                            \
  "3:\n"                                    \
  "cbz    %w[m],    1f\n"                   \
  "ldr   q0, [%[a_ptr]], #16\n"             \
  "ldr   q1, [%[widx_dmap]],   #16\n"       \
  "ldr   q2, [%[b_ptr]]\n"                  \
  "subs  %w[m],   %w[m],   #1\n"            \
  "mov   w1, v1.s[0]\n"                     \
  "prfm  pldl1keep, [%[widx_dmap], #128]\n" \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[a_ptr], #128]\n"     \
  "fmla    v21.4s,  v2.4s,  v0.s[0]\n"      \
  "beq     1f\n"                            \
  "ldr   q2, [%[b_ptr]]\n"                  \
  "mov   w1, v1.s[1]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr]]\n"           \
  "subs  %w[m],   %w[m],   #1\n"            \
  "fmla    v21.4s,  v2.4s,  v0.s[1]\n"      \
  "beq     1f\n"                            \
  "ldr   q2, [%[b_ptr]]\n"                  \
  "mov   w1, v1.s[2]\n"                     \
  "sxtw  x1,  w1\n"                         \
  "add   %[b_ptr],  %[b_ptr], x1\n"         \
  "prfm  pldl1keep, [%[b_ptr]]\n"           \
  "fmla    v21.4s,  v2.4s,  v0.s[2]\n"      \
  "1:\n"

#define SPARSE_F32_F32_W1_8_V8_KERNEL   \
  "dup     v0.4s,  %w[vbias]\n"         \
  "cbz    %w[k],    1f\n"               \
  "cbz    %w[n],    3f\n"               \
  "0:\n"                                \
  "ldr     s1, [%[a_ptr]], #4   \n"     \
  "ldr     s9, [%[b_ptr]]  \n"          \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ldr     s2, [%[a_ptr]], #4   \n"     \
  "ldr     s10, [%[b_ptr]]  \n"         \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ldr     s3, [%[a_ptr]], #4   \n"     \
  "ldr     s11, [%[b_ptr]]  \n"         \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ldr     s4, [%[a_ptr]], #4   \n"     \
  "ldr     s12, [%[b_ptr]]  \n"         \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ldr     s5, [%[a_ptr]], #4   \n"     \
  "ldr     s13, [%[b_ptr]]  \n"         \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ldr     s6, [%[a_ptr]], #4   \n"     \
  "ldr     s14, [%[b_ptr]]  \n"         \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ldr     s7, [%[a_ptr]], #4   \n"     \
  "ldr     s15, [%[b_ptr]]  \n"         \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ldr     s8, [%[a_ptr]], #4   \n"     \
  "ldr     s16, [%[b_ptr]]  \n"         \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "fmadd   s0, s1, s9, s0   \n"         \
  "fmadd   s0, s2, s10, s0   \n"        \
  "fmadd   s0, s3, s11, s0   \n"        \
  "fmadd   s0, s4, s12, s0   \n"        \
  "subs    %w[n],   %w[n],   #1\n"      \
  "fmadd   s0, s5, s13, s0   \n"        \
  "fmadd   s0, s6, s14, s0   \n"        \
  "fmadd   s0, s7, s15, s0   \n"        \
  "fmadd   s0, s8, s16, s0   \n"        \
  "bne     0b\n"                        \
  "3:\n"                                \
  "cbz    %w[m],    1f\n"               \
  "2:\n"                                \
  "ldr     s1, [%[a_ptr]], #4   \n"     \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "ldr     s5, [%[b_ptr]]  \n"          \
  "subs  %w[m],   %w[m],   #1\n"        \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "fmadd   s0, s1, s5, s0   \n"         \
  "bne     2b\n"                        \
  "1:\n"

#define SPARSE_F32_F32_W48_V8_RELU   \
  /* do relu */                      \
  "cmp    %w[vflag_act],    #0\n"    \
  "beq   9f                     \n"  \
  "cmp    %w[vflag_act],    #1\n"    \
  "bne   10f                     \n" \
  "movi   v0.4s, #0\n"               \
  "fmax   v20.4s, v20.4s, v0.4s\n"   \
  "fmax   v21.4s, v21.4s, v0.4s\n"   \
  "fmax   v22.4s, v22.4s, v0.4s\n"   \
  "fmax   v23.4s, v23.4s, v0.4s\n"   \
  "fmax   v24.4s, v24.4s, v0.4s\n"   \
  "fmax   v25.4s, v25.4s, v0.4s\n"   \
  "fmax   v26.4s, v26.4s, v0.4s\n"   \
  "fmax   v27.4s, v27.4s, v0.4s\n"   \
  "fmax   v28.4s, v28.4s, v0.4s\n"   \
  "fmax   v29.4s, v29.4s, v0.4s\n"   \
  "fmax   v30.4s, v30.4s, v0.4s\n"   \
  "fmax   v31.4s, v31.4s, v0.4s\n"   \
  "b      9f                    \n"

#define SPARSE_F32_F32_W32_V8_RELU   \
  /* do relu */                      \
  "cmp    %w[vflag_act],    #0\n"    \
  "beq   9f                     \n"  \
  "cmp    %w[vflag_act],    #1\n"    \
  "bne   10f                     \n" \
  "movi   v30.4s, #0\n"              \
  "fmax   v21.4s, v21.4s, v30.4s\n"  \
  "fmax   v22.4s, v22.4s, v30.4s\n"  \
  "fmax   v23.4s, v23.4s, v30.4s\n"  \
  "fmax   v24.4s, v24.4s, v30.4s\n"  \
  "fmax   v25.4s, v25.4s, v30.4s\n"  \
  "fmax   v26.4s, v26.4s, v30.4s\n"  \
  "fmax   v27.4s, v27.4s, v30.4s\n"  \
  "fmax   v28.4s, v28.4s, v30.4s\n"  \
  "b      9f                    \n"

#define SPARSE_F32_F32_W16_V8_RELU   \
  /* do relu */                      \
  "cmp    %w[vflag_act],    #0\n"    \
  "beq   9f                     \n"  \
  "cmp    %w[vflag_act],    #1\n"    \
  "bne   10f                     \n" \
  "movi   v9.4s, #0\n"               \
  "fmax   v21.4s, v21.4s, v9.4s\n"   \
  "fmax   v22.4s, v22.4s, v9.4s\n"   \
  "fmax   v23.4s, v23.4s, v9.4s\n"   \
  "fmax   v24.4s, v24.4s, v9.4s\n"   \
  "b      9f                    \n"

#define SPARSE_F32_F32_W8_V8_RELU    \
  /* do relu */                      \
  "cmp    %w[vflag_act],    #0\n"    \
  "beq   9f                     \n"  \
  "cmp    %w[vflag_act],    #1\n"    \
  "bne   10f                     \n" \
  "movi   v9.4s, #0\n"               \
  "fmax   v21.4s, v21.4s, v9.4s\n"   \
  "fmax   v22.4s, v22.4s, v9.4s\n"   \
  "b      9f                    \n"

#define SPARSE_F32_F32_W4_V8_RELU    \
  /* do relu */                      \
  "cmp    %w[vflag_act],    #0\n"    \
  "beq   9f                     \n"  \
  "cmp    %w[vflag_act],    #1\n"    \
  "bne   10f                     \n" \
  "movi   v9.4s, #0\n"               \
  "fmax   v21.4s, v21.4s, v9.4s\n"   \
  "b      9f                    \n"

#define SPARSE_F32_F32_W1_V8_RELU \
  /* do relu */                   \
  "cmp    %w[vflag_act],  #0\n"   \
  "beq    9f                \n"   \
  "cmp    %w[vflag_act],  #1\n"   \
  "bne    10f               \n"   \
  "movi   v9.4s, #0\n"            \
  "fmax   s0,  s0,  s9\n"         \
  "b      9f                \n"

#define SPARSE_F32_F32_W48_V8_RELU6   \
  /* do relu6 */                      \
  "10: \n"                            \
  "cmp   %w[vflag_act],  #2       \n" \
  "bne   11f                     \n"  \
  "movi   v0.4s, #0\n"                \
  "dup    v1.4s,  %w[valpha]\n"       \
  "fmax   v20.4s, v20.4s, v0.4s\n"    \
  "fmax   v21.4s, v21.4s, v0.4s\n"    \
  "fmax   v22.4s, v22.4s, v0.4s\n"    \
  "fmax   v23.4s, v23.4s, v0.4s\n"    \
  "fmax   v24.4s, v24.4s, v0.4s\n"    \
  "fmax   v25.4s, v25.4s, v0.4s\n"    \
  "fmax   v26.4s, v26.4s, v0.4s\n"    \
  "fmax   v27.4s, v27.4s, v0.4s\n"    \
  "fmax   v28.4s, v28.4s, v0.4s\n"    \
  "fmax   v29.4s, v29.4s, v0.4s\n"    \
  "fmax   v30.4s, v30.4s, v0.4s\n"    \
  "fmax   v31.4s, v31.4s, v0.4s\n"    \
  "fmin   v20.4s, v20.4s, v1.4s\n"    \
  "fmin   v21.4s, v21.4s, v1.4s\n"    \
  "fmin   v22.4s, v22.4s, v1.4s\n"    \
  "fmin   v23.4s, v23.4s, v1.4s\n"    \
  "fmin   v24.4s, v24.4s, v1.4s\n"    \
  "fmin   v25.4s, v25.4s, v1.4s\n"    \
  "fmin   v26.4s, v26.4s, v1.4s\n"    \
  "fmin   v27.4s, v27.4s, v1.4s\n"    \
  "fmin   v28.4s, v28.4s, v1.4s\n"    \
  "fmin   v29.4s, v29.4s, v1.4s\n"    \
  "fmin   v30.4s, v30.4s, v1.4s\n"    \
  "fmin   v31.4s, v31.4s, v1.4s\n"    \
  "b      9f                    \n"

#define SPARSE_F32_F32_W32_V8_RELU6   \
  /* do relu6 */                      \
  "10: \n"                            \
  "cmp   %w[vflag_act],  #2       \n" \
  "bne   11f                     \n"  \
  "movi   v0.4s, #0\n"                \
  "dup    v1.4s,  %w[valpha]\n"       \
  "fmax   v21.4s, v21.4s, v0.4s\n"    \
  "fmax   v22.4s, v22.4s, v0.4s\n"    \
  "fmax   v23.4s, v23.4s, v0.4s\n"    \
  "fmax   v24.4s, v24.4s, v0.4s\n"    \
  "fmax   v25.4s, v25.4s, v0.4s\n"    \
  "fmax   v26.4s, v26.4s, v0.4s\n"    \
  "fmax   v27.4s, v27.4s, v0.4s\n"    \
  "fmax   v28.4s, v28.4s, v0.4s\n"    \
  "fmin   v21.4s, v21.4s, v1.4s\n"    \
  "fmin   v22.4s, v22.4s, v1.4s\n"    \
  "fmin   v23.4s, v23.4s, v1.4s\n"    \
  "fmin   v24.4s, v24.4s, v1.4s\n"    \
  "fmin   v25.4s, v25.4s, v1.4s\n"    \
  "fmin   v26.4s, v26.4s, v1.4s\n"    \
  "fmin   v27.4s, v27.4s, v1.4s\n"    \
  "fmin   v28.4s, v28.4s, v1.4s\n"    \
  "b      9f                    \n"

#define SPARSE_F32_F32_W16_V8_RELU6   \
  /* do relu6 */                      \
  "10: \n"                            \
  "cmp   %w[vflag_act],  #2       \n" \
  "bne   11f                     \n"  \
  "movi   v0.4s, #0\n"                \
  "dup    v1.4s,  %w[valpha]\n"       \
  "fmax   v21.4s, v21.4s, v0.4s\n"    \
  "fmax   v22.4s, v22.4s, v0.4s\n"    \
  "fmax   v23.4s, v23.4s, v0.4s\n"    \
  "fmax   v24.4s, v24.4s, v0.4s\n"    \
  "fmin   v21.4s, v21.4s, v1.4s\n"    \
  "fmin   v22.4s, v22.4s, v1.4s\n"    \
  "fmin   v23.4s, v23.4s, v1.4s\n"    \
  "fmin   v24.4s, v24.4s, v1.4s\n"    \
  "b      9f                    \n"

#define SPARSE_F32_F32_W8_V8_RELU6    \
  /* do relu6 */                      \
  "10: \n"                            \
  "cmp   %w[vflag_act],  #2       \n" \
  "bne   11f                     \n"  \
  "movi   v0.4s, #0\n"                \
  "dup    v1.4s,  %w[valpha]\n"       \
  "fmax   v21.4s, v21.4s, v0.4s\n"    \
  "fmax   v22.4s, v22.4s, v0.4s\n"    \
  "fmin   v21.4s, v21.4s, v1.4s\n"    \
  "fmin   v22.4s, v22.4s, v1.4s\n"    \
  "b      9f                    \n"

#define SPARSE_F32_F32_W4_V8_RELU6    \
  /* do relu6 */                      \
  "10: \n"                            \
  "cmp   %w[vflag_act],  #2       \n" \
  "bne   11f                     \n"  \
  "movi   v0.4s, #0\n"                \
  "dup    v1.4s,  %w[valpha]\n"       \
  "fmax   v21.4s, v21.4s, v0.4s\n"    \
  "fmin   v21.4s, v21.4s, v1.4s\n"    \
  "b      9f                    \n"

#define SPARSE_F32_F32_W1_V8_RELU6    \
  /* do relu6 */                      \
  "10: \n"                            \
  "cmp   %w[vflag_act],  #2       \n" \
  "bne   11f                     \n"  \
  "movi   v9.4s, #0\n"                \
  "dup    v1.4s,  %w[valpha]\n"       \
  "fmax   s0,  s0,  s9\n"             \
  "fmin   s0,  s0,  s1\n"             \
  "b      9f                    \n"

#define SPARSE_F32_F32_W48_V8_LEAKY_RELU                            \
  /* do relu */                                                     \
  "11: \n"                                                          \
  "cmp    %w[vflag_act],  #3       \n"                              \
  "bne    12f                     \n"                               \
  "movi   v0.4s, #0\n"                      /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"             /* leakey relu alpha */ \
  "fcmge  v2.4s,    v20.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v3.4s,    v20.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v4.4s,    v21.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v5.4s,    v21.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v6.4s,    v22.4s,   v0.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v7.4s,    v22.4s,   v1.4s   \n"   /* vmulq_f32 */         \
  "fcmge  v8.4s,    v23.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v9.4s,    v23.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v10.4s,   v24.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v11.4s,   v24.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v12.4s,   v25.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v13.4s,   v25.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "bif    v20.16b,  v3.16b,   v2.16b  \n"   /* choose*/             \
  "bif    v21.16b,  v5.16b,   v4.16b  \n"   /* choose*/             \
  "bif    v22.16b,  v7.16b,   v6.16b  \n"   /* choose*/             \
  "bif    v23.16b,  v9.16b,   v8.16b  \n"   /* choose*/             \
  "bif    v24.16b,  v11.16b,   v10.16b  \n" /* choose*/             \
  "bif    v25.16b,  v13.16b,   v12.16b  \n" /* choose*/             \
  "fcmge  v2.4s,    v26.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v3.4s,    v26.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v4.4s,    v27.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v5.4s,    v27.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v6.4s,    v28.4s,   v0.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v7.4s,    v28.4s,   v1.4s   \n"   /* vmulq_f32 */         \
  "fcmge  v8.4s,    v29.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v9.4s,    v29.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v10.4s,   v30.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v11.4s,   v30.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v12.4s,   v31.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v13.4s,   v31.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "bif    v26.16b,  v3.16b,   v2.16b  \n"   /* choose*/             \
  "bif    v27.16b,  v5.16b,   v4.16b  \n"   /* choose*/             \
  "bif    v28.16b,  v7.16b,   v6.16b  \n"   /* choose*/             \
  "bif    v29.16b,  v9.16b,   v8.16b  \n"   /* choose*/             \
  "bif    v30.16b,  v11.16b,   v10.16b  \n" /* choose*/             \
  "bif    v31.16b,  v13.16b,   v12.16b  \n" /* choose*/             \
  "b      9f                    \n"

#define SPARSE_F32_F32_W32_V8_LEAKY_RELU                           \
  /* do relu */                                                    \
  "11: \n"                                                         \
  "cmp    %w[vflag_act],  #3       \n"                             \
  "bne    12f                     \n"                              \
  "movi   v0.4s, #0\n"                     /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"            /* leakey relu alpha */ \
  "fcmge  v2.4s,    v21.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v21.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v4.4s,    v22.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v5.4s,    v22.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v6.4s,    v23.4s,   v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v7.4s,    v23.4s,   v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v8.4s,    v24.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v9.4s,    v24.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v21.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "bif    v22.16b,   v5.16b,   v4.16b  \n" /* choose*/             \
  "bif    v23.16b,  v7.16b,   v6.16b  \n"  /* choose*/             \
  "bif    v24.16b,  v9.16b,   v8.16b  \n"  /* choose*/             \
  "fcmge  v2.4s,    v25.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v25.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v4.4s,    v26.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v5.4s,    v26.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v6.4s,    v27.4s,   v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v7.4s,    v27.4s,   v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v8.4s,    v28.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v9.4s,    v28.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v25.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "bif    v26.16b,   v5.16b,   v4.16b  \n" /* choose*/             \
  "bif    v27.16b,  v7.16b,   v6.16b  \n"  /* choose*/             \
  "bif    v28.16b,  v9.16b,   v8.16b  \n"  /* choose*/             \
  "b      9f                    \n"

#define SPARSE_F32_F32_W16_V8_LEAKY_RELU                           \
  /* do relu */                                                    \
  "11: \n"                                                         \
  "cmp    %w[vflag_act],  #3       \n"                             \
  "bne    12f                     \n"                              \
  "movi   v0.4s, #0\n"                     /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"            /* leakey relu alpha */ \
  "fcmge  v2.4s,    v21.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v21.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v4.4s,    v22.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v5.4s,    v22.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v6.4s,    v23.4s,   v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v7.4s,    v23.4s,   v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v8.4s,    v24.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v9.4s,    v24.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v21.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "bif    v22.16b,   v5.16b,   v4.16b  \n" /* choose*/             \
  "bif    v23.16b,  v7.16b,   v6.16b  \n"  /* choose*/             \
  "bif    v24.16b,  v9.16b,   v8.16b  \n"  /* choose*/             \
  "b      9f                    \n"

#define SPARSE_F32_F32_W8_V8_LEAKY_RELU                            \
  /* do relu */                                                    \
  "11: \n"                                                         \
  "cmp    %w[vflag_act],  #3       \n"                             \
  "bne    12f                     \n"                              \
  "movi   v0.4s, #0\n"                     /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"            /* leakey relu alpha */ \
  "fcmge  v2.4s,    v21.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v21.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v4.4s,    v22.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v5.4s,    v22.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v21.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "bif    v22.16b,   v5.16b,   v4.16b  \n" /* choose*/             \
  "b      9f                    \n"

#define SPARSE_F32_F32_W4_V8_LEAKY_RELU                            \
  /* do relu */                                                    \
  "11: \n"                                                         \
  "cmp    %w[vflag_act],  #3       \n"                             \
  "bne    12f                     \n"                              \
  "movi   v0.4s, #0\n"                     /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"            /* leakey relu alpha */ \
  "fcmge  v2.4s,    v21.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v21.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v21.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "b      9f                    \n"

#define SPARSE_F32_F32_W1_V8_LEAKY_RELU                           \
  /* do relu */                                                   \
  "11: \n"                                                        \
  "cmp    %w[vflag_act],  #3       \n"                            \
  "bne    12f                     \n"                             \
  "movi   v9.4s, #0\n"                    /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"           /* leakey relu alpha */ \
  "fcmge  s2,    s0,    s9   \n"          /* vcgeq_f32 */         \
  "fmul   s3,    s0,    s1   \n"          /* vmulq_f32 */         \
  "bif    v0.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "b      9f                    \n"

#define SPARSE_F32_F32_W48_V8_HARD_SWISH                         \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v4.4s,  v20.4s, v1.4s        \n"                       \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v22.4s, v1.4s        \n"                       \
  "fadd   v10.4s,  v23.4s, v1.4s        \n"                      \
  "fadd   v12.4s,  v24.4s, v1.4s        \n"                      \
  "fadd   v14.4s,  v25.4s, v1.4s        \n"                      \
  "fadd   v16.4s,  v26.4s, v1.4s        \n"                      \
  "fadd   v18.4s,  v27.4s, v1.4s        \n"                      \
  "fmul   v5.4s,   v20.4s, v2.4s        \n"                      \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v22.4s, v2.4s        \n"                      \
  "fmul   v11.4s,  v23.4s, v2.4s        \n"                      \
  "fmul   v13.4s,  v24.4s, v2.4s        \n"                      \
  "fmul   v15.4s,  v25.4s, v2.4s        \n"                      \
  "fmul   v17.4s,  v26.4s, v2.4s        \n"                      \
  "fmul   v19.4s,  v27.4s, v2.4s        \n"                      \
  "fmax   v4.4s,  v4.4s, v0.4s        \n"                        \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v0.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v0.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v0.4s       \n"                        \
  "fmax   v16.4s, v16.4s, v0.4s       \n"                        \
  "fmax   v18.4s, v18.4s, v0.4s       \n"                        \
  "fmin   v4.4s,  v4.4s, v3.4s        \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmin   v10.4s, v10.4s, v3.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v3.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v3.4s       \n"                        \
  "fmin   v16.4s, v16.4s, v3.4s       \n"                        \
  "fmin   v18.4s, v18.4s, v3.4s       \n"                        \
  "fmul   v20.4s,  v5.4s,  v4.4s        \n"                      \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v22.4s,  v9.4s,  v8.4s        \n"                      \
  "fmul   v23.4s,  v11.4s, v10.4s        \n"                     \
  "fmul   v24.4s,  v13.4s, v12.4s        \n"                     \
  "fmul   v25.4s,  v15.4s, v14.4s        \n"                     \
  "fmul   v26.4s,  v17.4s, v16.4s        \n"                     \
  "fmul   v27.4s,  v19.4s, v18.4s        \n"                     \
  "fadd   v4.4s,  v28.4s, v1.4s        \n"                       \
  "fadd   v6.4s,  v29.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v30.4s, v1.4s        \n"                       \
  "fadd   v10.4s,  v31.4s, v1.4s        \n"                      \
  "fmul   v5.4s,   v28.4s, v2.4s        \n"                      \
  "fmul   v7.4s,   v29.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v30.4s, v2.4s        \n"                      \
  "fmul   v11.4s,  v31.4s, v2.4s        \n"                      \
  "fmax   v4.4s,  v4.4s, v0.4s        \n"                        \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v0.4s       \n"                        \
  "fmin   v4.4s,  v4.4s, v3.4s        \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmin   v10.4s, v10.4s, v3.4s       \n"                        \
  "fmul   v28.4s,  v5.4s,  v4.4s        \n"                      \
  "fmul   v29.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v30.4s,  v9.4s,  v8.4s        \n"                      \
  "fmul   v31.4s,  v11.4s, v10.4s        \n"                     \
  "9:\n"

#define SPARSE_F32_F32_W32_V8_HARD_SWISH                         \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v22.4s, v1.4s        \n"                       \
  "fadd   v10.4s,  v23.4s, v1.4s        \n"                      \
  "fadd   v12.4s,  v24.4s, v1.4s        \n"                      \
  "fadd   v14.4s,  v25.4s, v1.4s        \n"                      \
  "fadd   v16.4s,  v26.4s, v1.4s        \n"                      \
  "fadd   v18.4s,  v27.4s, v1.4s        \n"                      \
  "fadd   v30.4s,  v28.4s, v1.4s        \n"                      \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v22.4s, v2.4s        \n"                      \
  "fmul   v11.4s,  v23.4s, v2.4s        \n"                      \
  "fmul   v13.4s,  v24.4s, v2.4s        \n"                      \
  "fmul   v15.4s,  v25.4s, v2.4s        \n"                      \
  "fmul   v17.4s,  v26.4s, v2.4s        \n"                      \
  "fmul   v19.4s,  v27.4s, v2.4s        \n"                      \
  "fmul   v31.4s,  v28.4s, v2.4s        \n"                      \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v0.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v0.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v0.4s       \n"                        \
  "fmax   v16.4s, v16.4s, v0.4s       \n"                        \
  "fmax   v18.4s, v18.4s, v0.4s       \n"                        \
  "fmax   v30.4s, v30.4s, v0.4s       \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmin   v10.4s, v10.4s, v3.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v3.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v3.4s       \n"                        \
  "fmin   v16.4s, v16.4s, v3.4s       \n"                        \
  "fmin   v18.4s, v18.4s, v3.4s       \n"                        \
  "fmin   v30.4s, v30.4s, v3.4s       \n"                        \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v22.4s,  v9.4s,  v8.4s        \n"                      \
  "fmul   v23.4s,  v11.4s, v10.4s        \n"                     \
  "fmul   v24.4s,  v13.4s, v12.4s        \n"                     \
  "fmul   v25.4s,  v15.4s, v14.4s        \n"                     \
  "fmul   v26.4s,  v17.4s, v16.4s        \n"                     \
  "fmul   v27.4s,  v19.4s, v18.4s        \n"                     \
  "fmul   v28.4s,  v31.4s, v30.4s        \n"                     \
  "9:\n"

#define SPARSE_F32_F32_W16_V8_HARD_SWISH                         \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v22.4s, v1.4s        \n"                       \
  "fadd   v10.4s,  v23.4s, v1.4s        \n"                      \
  "fadd   v12.4s,  v24.4s, v1.4s        \n"                      \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v22.4s, v2.4s        \n"                      \
  "fmul   v11.4s,  v23.4s, v2.4s        \n"                      \
  "fmul   v13.4s,  v24.4s, v2.4s        \n"                      \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v0.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v0.4s       \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmin   v10.4s, v10.4s, v3.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v3.4s       \n"                        \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v22.4s,  v9.4s,  v8.4s        \n"                      \
  "fmul   v23.4s,  v11.4s, v10.4s        \n"                     \
  "fmul   v24.4s,  v13.4s, v12.4s        \n"                     \
  "9:\n"

#define SPARSE_F32_F32_W8_V8_HARD_SWISH                          \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v22.4s, v1.4s        \n"                       \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v22.4s, v2.4s        \n"                      \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v22.4s,  v9.4s,  v8.4s        \n"                      \
  "9:\n"

#define SPARSE_F32_F32_W4_V8_HARD_SWISH                          \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "9:\n"

#define SPARSE_F32_F32_W1_V8_HARD_SWISH                          \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v9.4s,    #0                \n"    /* for hardswish */ \
  "ldr    s1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    s2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    s3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   s6,  s0,  s1        \n"                                \
  "fmul   s7,  s0,  s2        \n"                                \
  "fmax   s6,  s6,  s9        \n"                                \
  "fmin   s6,  s6,  s3        \n"                                \
  "fmul   s0,  s7,  s6        \n"                                \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx48, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx48, and the required data is
 * MxKxKx48.
 */
#define SPARSE_F32_F32_W48_V8_OUT       \
  SPARSE_F32_F32_W48_V8_KERNEL          \
  SPARSE_F32_F32_W48_V8_RELU            \
  SPARSE_F32_F32_W48_V8_RELU6           \
  SPARSE_F32_F32_W48_V8_LEAKY_RELU      \
  SPARSE_F32_F32_W48_V8_HARD_SWISH      \
  /* store result */                    \
  "stp   q20, q21,  [%[c_ptr]]\n"       \
  "stp   q22, q23,  [%[c_ptr], #32]\n"  \
  "stp   q24, q25,  [%[c_ptr], #64]\n"  \
  "stp   q26, q27,  [%[c_ptr], #96]\n"  \
  "stp   q28, q29,  [%[c_ptr], #128]\n" \
  "stp   q30, q31,  [%[c_ptr], #160]\n"

/**
 * The data block size for sparse matrix calculation is Mx32, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx48, and the required data is
 * MxKxKx32.
 */
#define SPARSE_F32_F32_W32_V8_OUT      \
  SPARSE_F32_F32_W32_V8_KERNEL         \
  SPARSE_F32_F32_W32_V8_RELU           \
  SPARSE_F32_F32_W32_V8_RELU6          \
  SPARSE_F32_F32_W32_V8_LEAKY_RELU     \
  SPARSE_F32_F32_W32_V8_HARD_SWISH     \
  /* store result */                   \
  "stp   q21, q22,  [%[c_ptr]]\n"      \
  "stp   q23, q24,  [%[c_ptr], #32]\n" \
  "stp   q25, q26,  [%[c_ptr], #64]\n" \
  "stp   q27, q28,  [%[c_ptr], #96]\n"

/**
 * The data block size for sparse matrix calculation is Mx16, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx16, and the required data is
 * MxKxKx16.
 */
#define SPARSE_F32_F32_W16_V8_OUT  \
  SPARSE_F32_F32_W16_V8_KERNEL     \
  SPARSE_F32_F32_W16_V8_RELU       \
  SPARSE_F32_F32_W16_V8_RELU6      \
  SPARSE_F32_F32_W16_V8_LEAKY_RELU \
  SPARSE_F32_F32_W16_V8_HARD_SWISH \
  /* store result */               \
  "stp   q21, q22,  [%[c_ptr]]\n"  \
  "stp   q23, q24,  [%[c_ptr], #32]\n"

/**
 * The data block size for sparse matrix calculation is Mx8, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx8, and the required data is
 * MxKxKx8.
 */
#define SPARSE_F32_F32_W8_V8_OUT  \
  SPARSE_F32_F32_W8_V8_KERNEL     \
  SPARSE_F32_F32_W8_V8_RELU       \
  SPARSE_F32_F32_W8_V8_RELU6      \
  SPARSE_F32_F32_W8_V8_LEAKY_RELU \
  SPARSE_F32_F32_W8_V8_HARD_SWISH \
  /* store result */              \
  "stp   q21, q22,  [%[c_ptr]]\n"

/**
 * The data block size for sparse matrix calculation is Mx4, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx4, and the required data is
 * MxKxKx4.
 */
#define SPARSE_F32_F32_W4_V8_OUT  \
  SPARSE_F32_F32_W4_V8_KERNEL     \
  SPARSE_F32_F32_W4_V8_RELU       \
  SPARSE_F32_F32_W4_V8_RELU6      \
  SPARSE_F32_F32_W4_V8_LEAKY_RELU \
  SPARSE_F32_F32_W4_V8_HARD_SWISH \
  /* store result */              \
  "str   q21,  [%[c_ptr]]\n"

#define SPARSE_F32_F32_W1_8_V8_OUT \
  SPARSE_F32_F32_W1_8_V8_KERNEL    \
  SPARSE_F32_F32_W1_V8_RELU        \
  SPARSE_F32_F32_W1_V8_RELU6       \
  SPARSE_F32_F32_W1_V8_LEAKY_RELU  \
  SPARSE_F32_F32_W1_V8_HARD_SWISH  \
  /* store result */               \
  "str   s0,  [%[c_ptr]]\n"

/**
 * \brief Sparse calculation implementation of 1x1 convolution, both input and
 * output are f32.
 * Sparse matrix multiplication is calculated in blocks, the block size is Mx48,
 * that is,
 * the parameter matrix is MxK, and the activation matrix is Kx48; when N is
 * less than 48,
 * it is calculated in blocks of Mx32, Mx16, Mx8, and Mx4 in turn;
 * @param A sparse weight data
 * @param B dense input data
 * @param widx_dmap An array of int32_t values storing scaled [by sizeof(input
 * element)] difference
 * between input channels corresponding to successive non-zero element
 * @param nidx_nnzmap the number of non-zero kernel elements per each output
 * channel
 * @param bias
 * @param output
 * @param M
 * @param N
 * @param K
 * @param param
 * @param ctx
 */
void sparse_conv_fp32_pipelined(const float* A,
                                const float* B,
                                const int32_t* widx_dmap,
                                const uint32_t* nidx_nnzmap,
                                const float* bias,
                                float* output,
                                const int M,
                                const int K,
                                const int N,
                                const operators::SparseConvParam& param,
                                ARMContext* ctx) {
  auto act_param = param.activation_param;
  auto act_type = act_param.active_type;
  volatile float alpha = 0.f;
  float hs_param[12] = {0.f};
  int flag_act = 0x00;  // relu: 1, relu6: 2, leakey: 3, hard_swish: 4
  if (act_param.has_active) {
    if (act_type == lite_api::ActivationType::kRelu) {
      flag_act = 0x01;
    } else if (act_type == lite_api::ActivationType::kRelu6) {
      flag_act = 0x02;
      alpha = act_param.Relu_clipped_coef;
    } else if (act_type == lite_api::ActivationType::kLeakyRelu) {
      flag_act = 0x03;
      alpha = act_param.Leaky_relu_alpha;
    } else if (act_type == lite_api::ActivationType::kHardSwish) {
      flag_act = 0x04;
      for (int i = 0; i < 4; i++) {
        hs_param[i] = act_param.hard_swish_offset;
        hs_param[i + 4] = 1.0 / act_param.hard_swish_scale;
        hs_param[i + 8] = act_param.hard_swish_threshold;
      }
    }
  }
  int flag_bias = (bias != nullptr) ? 1 : 0;
  size_t mc = N * sizeof(float);
  size_t nc = M;
  size_t output_stride = N * sizeof(float);
  size_t output_decrement = output_stride * nc - 48 * sizeof(float);
  while
    SPARSE_LIKELY(mc >= 48 * sizeof(float)) {
      LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
        float* cur_output =
            reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
        const float* cur_w = A;
        uint32_t nnz = nidx_nnzmap[i];
        const float* cur_b = B;
        const int32_t* dmap = widx_dmap;
        if (i != 0) {
          int cur_rem = nidx_nnzmap[i - 1] & 3;
          if (cur_rem != 0) {
            cur_rem = 4 - cur_rem;
          }
          nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
          cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
          cur_b += ((nidx_nnzmap[i - 1] == 0)
                        ? 0
                        : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
          dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
        }
        uint32_t pair_num = nnz / 4;
        uint32_t lave_num = nnz % 4;
        float vbias = (bias != nullptr) ? bias[i] : 0.0;
        // clang-format off
            asm volatile(SPARSE_F32_F32_W48_V8_OUT  
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap)
              : [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha),
                [hs_param] "r"(hs_param)
              : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
                  "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
                  "v16", "v17", "v18", "v21", "v22", "v23", "v24", "v25", 
                  "v26", "v27", "v28", "v30", "v31", "x1", "cc", "memory");
        // clang-format on
      }
      LITE_PARALLEL_COMMON_END();
      output = reinterpret_cast<float*>((uintptr_t)output + 48 * sizeof(float));
      B += 48;
      mc -= 48 * sizeof(float);
    }

  if
    SPARSE_UNLIKELY(mc != 0) {
      output_decrement += 16 * sizeof(float);
      if (mc >= (32 * sizeof(float))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const float* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const float* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            int cur_rem = nidx_nnzmap[i - 1] & 3;
            if (cur_rem != 0) {
              cur_rem = 4 - cur_rem;
            }
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
            cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
            cur_b += ((nidx_nnzmap[i - 1] == 0)
                          ? 0
                          : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
            dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
          }
          uint32_t pair_num = nnz / 4;
          uint32_t lave_num = nnz % 4;
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          // clang-format off
            asm volatile(SPARSE_F32_F32_W32_V8_OUT  
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap)
              : [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha),
                [hs_param] "r"(hs_param)
              : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
                "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
                "v16", "v17", "v18", "v21", "v22", "v23", "v24", "v25", 
                "v26", "v27", "v28", "v30", "v31", "w1", "x1", "cc", "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 32 * sizeof(float));
        B += 32;
        mc -= 32 * sizeof(float);
      }
      output_decrement += 16 * sizeof(float);
      if (mc >= (16 * sizeof(float))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const float* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const float* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            int cur_rem = nidx_nnzmap[i - 1] & 3;
            if (cur_rem != 0) {
              cur_rem = 4 - cur_rem;
            }
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
            cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
            cur_b += ((nidx_nnzmap[i - 1] == 0)
                          ? 0
                          : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
            dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
          }
          uint32_t pair_num = nnz / 4;
          uint32_t lave_num = nnz % 4;
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          // clang-format off
            asm volatile(SPARSE_F32_F32_W16_V8_OUT  
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap)
              : [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha),
                [hs_param] "r"(hs_param)
              : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
                "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v21", "v22", "v23",
                "v24", "w1", "x1", "cc", "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 16 * sizeof(float));
        B += 16;
        mc -= 16 * sizeof(float);
      }
      output_decrement += 8 * sizeof(float);
      if (mc >= (8 * sizeof(float))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const float* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const float* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            int cur_rem = nidx_nnzmap[i - 1] & 3;
            if (cur_rem != 0) {
              cur_rem = 4 - cur_rem;
            }
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
            cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
            cur_b += ((nidx_nnzmap[i - 1] == 0)
                          ? 0
                          : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
            dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
          }
          uint32_t pair_num = nnz / 4;
          uint32_t lave_num = nnz % 4;
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          // clang-format off
            asm volatile(SPARSE_F32_F32_W8_V8_OUT  
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap)
              : [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha),
                [hs_param] "r"(hs_param)
              : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v11", "v12", "v21", 
              "v22", "w1", "x1", "cc", "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 8 * sizeof(float));
        B += 8;
        mc -= 8 * sizeof(float);
      }
      output_decrement += 4 * sizeof(float);
      if (mc >= (4 * sizeof(float))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const float* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const float* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            int cur_rem = nidx_nnzmap[i - 1] & 3;
            if (cur_rem != 0) {
              cur_rem = 4 - cur_rem;
            }
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
            cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
            cur_b += ((nidx_nnzmap[i - 1] == 0)
                          ? 0
                          : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
            dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
          }
          uint32_t pair_num = nnz / 4;
          uint32_t lave_num = nnz % 4;
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          // clang-format off
            asm volatile(SPARSE_F32_F32_W4_V8_OUT  
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap)
              : [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha),
                [hs_param] "r"(hs_param)
              : "v0", "v1", "v2", "v3", "v4", "v6", "v7", "v9", "v11", "v21", 
              "w1", "w2", "w3", "w4", "w5", "x1", "cc", "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 4 * sizeof(float));
        B += 4;
        mc -= 4 * sizeof(float);
      }

      output_decrement += 2 * sizeof(float);
      if (mc >= (2 * sizeof(float))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const float* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const float* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            int cur_rem = nidx_nnzmap[i - 1] & 3;
            if (cur_rem != 0) {
              cur_rem = 4 - cur_rem;
            }
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
            cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
            cur_b += ((nidx_nnzmap[i - 1] == 0)
                          ? 0
                          : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
            dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
          }
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float32x2_t vacc01n0 = vdup_n_f32(vbias);
          if
            SPARSE_LIKELY(nnz != 0) {
              do {
                const intptr_t diff = *dmap++;
                const float32x2_t vi01 = vld1_f32(cur_b);
                cur_b = (const float*)((uintptr_t)cur_b + (uintptr_t)diff);
                const float32x2_t vw = vld1_dup_f32(cur_w);
                cur_w += 1;
                vacc01n0 = vmla_lane_f32(vacc01n0, vi01, vw, 0);
              } while (--nnz != 0);
            }
          if (flag_act == 1) {
            float32x2_t vzero = vdup_n_f32(0);
            vacc01n0 = vmax_f32(vacc01n0, vzero);
          } else if (flag_act == 2) {
            float32x2_t vzero = vdup_n_f32(0);
            float32x2_t aph = vdup_n_f32(alpha);
            vacc01n0 = vmax_f32(vacc01n0, vzero);
            vacc01n0 = vmin_f32(vacc01n0, aph);
          } else if (flag_act == 0) {
          } else if (flag_act == 3) {
            float32x2_t vzero = vdup_n_f32(0);
            float32x2_t aph = vdup_n_f32(alpha);
            uint32x2_t vflag0123 = vcge_f32(vacc01n0, vzero);
            float32x2_t v0123 = vmul_f32(vacc01n0, aph);
            vacc01n0 = vbsl_f32(vflag0123, vacc01n0, v0123);
          } else {
            float32x2_t vzero = vdup_n_f32(0);
            float32x2_t offset = vdup_n_f32(act_param.hard_swish_offset);
            float32x2_t hs_scale = vdup_n_f32(1.0 / act_param.hard_swish_scale);
            float32x2_t thre = vdup_n_f32(act_param.hard_swish_threshold);
            float32x2_t vset0123 = vadd_f32(vacc01n0, offset);
            float32x2_t vscale0123 = vmul_f32(vacc01n0, hs_scale);
            vset0123 = vmax_f32(vset0123, vzero);
            vset0123 = vmin_f32(vset0123, thre);
            vacc01n0 = vmul_f32(vscale0123, vset0123);
          }
          vst1_f32(cur_output, vacc01n0);
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 2 * sizeof(float));
        B += 2;
        mc -= 2 * sizeof(float);
      }

      output_decrement += 1 * sizeof(float);
      if (mc >= (1 * sizeof(float))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const float* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const float* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            int cur_rem = nidx_nnzmap[i - 1] & 3;
            if (cur_rem != 0) {
              cur_rem = 4 - cur_rem;
            }
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
            cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
            cur_b += ((nidx_nnzmap[i - 1] == 0)
                          ? 0
                          : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
            dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
          }
          uint32_t pair_num = nnz / 8;
          uint32_t lave_num = nnz % 8;
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          // clang-format off
            asm volatile(SPARSE_F32_F32_W1_8_V8_OUT  
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap)
              : [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha),
                [hs_param] "r"(hs_param)
              : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10",
                "v11", "v12", "v13", "v14", "v15", "v16", "x1", "cc", "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 1 * sizeof(float));
        B += 1;
        mc -= 1 * sizeof(float);
      }
    }
}

#define SPARSE_INT8_F32_W64_V8_KERNEL      \
  "eor v16.16b, v0.16b, v0.16b\n"          \
  "eor v17.16b, v1.16b, v1.16b\n"          \
  "eor v18.16b, v2.16b, v2.16b\n"          \
  "eor v19.16b, v3.16b, v3.16b\n"          \
  "eor v20.16b, v4.16b, v4.16b\n"          \
  "prfm  pldl1keep, [%[a_ptr], #32]\n"     \
  "eor v21.16b, v5.16b, v5.16b\n"          \
  "eor v22.16b, v6.16b, v6.16b\n"          \
  "prfm  pldl1keep, [%[widx_dmap], #32]\n" \
  "eor v23.16b, v7.16b, v7.16b\n"          \
  "eor v24.16b, v8.16b, v8.16b\n"          \
  "prfm  pldl1keep, [%[b_ptr], #64]\n"     \
  "eor v25.16b, v9.16b, v9.16b\n"          \
  "eor v26.16b, v10.16b, v10.16b\n"        \
  "eor v27.16b, v11.16b, v11.16b\n"        \
  "eor v28.16b, v12.16b, v12.16b\n"        \
  "eor v29.16b, v13.16b, v13.16b\n"        \
  "eor v30.16b, v14.16b, v14.16b\n"        \
  "eor v31.16b, v15.16b, v15.16b\n"        \
  "cbz    %w[k],    1f\n"                  \
  "0:\n"                                   \
  "ld1r  {v0.16b}, [%[a_ptr]], #1\n"       \
  "ldp   q1, q2, [%[b_ptr]]\n"             \
  "ldp   q3, q4, [%[b_ptr], #32]\n"        \
  "subs    %w[k],   %w[k],   #1\n"         \
  "smull   v5.8h,   v0.8b,   v1.8b\n"      \
  "smull2  v6.8h,   v0.16b,  v1.16b\n"     \
  "smull   v7.8h,   v0.8b,   v2.8b\n"      \
  "smull2  v8.8h,   v0.16b,  v2.16b\n"     \
  "smull   v9.8h,   v0.8b,   v3.8b\n"      \
  "smull2  v10.8h,  v0.16b,  v3.16b\n"     \
  "smull   v11.8h,  v0.8b,   v4.8b\n"      \
  "smull2  v12.8h,  v0.16b,  v4.16b\n"     \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n"    \
  "saddw   v16.4s,  v16.4s,  v5.4h\n"      \
  "saddw2  v17.4s,  v17.4s,  v5.8h\n"      \
  "saddw   v18.4s,  v18.4s,  v6.4h\n"      \
  "saddw2  v19.4s,  v19.4s,  v6.8h\n"      \
  "saddw   v20.4s,  v20.4s,  v7.4h\n"      \
  "saddw2  v21.4s,  v21.4s,  v7.8h\n"      \
  "add   %[b_ptr],  %[b_ptr], x1\n"        \
  "saddw   v22.4s,  v22.4s,  v8.4h\n"      \
  "saddw2  v23.4s,  v23.4s,  v8.8h\n"      \
  "saddw   v24.4s,  v24.4s,  v9.4h\n"      \
  "saddw2  v25.4s,  v25.4s,  v9.8h\n"      \
  "prfm  pldl1keep, [%[b_ptr], #64]\n"     \
  "saddw   v26.4s,  v26.4s,  v10.4h\n"     \
  "saddw2  v27.4s,  v27.4s,  v10.8h\n"     \
  "saddw   v28.4s,  v28.4s,  v11.4h\n"     \
  "saddw2  v29.4s,  v29.4s,  v11.8h\n"     \
  "saddw   v30.4s,  v30.4s,  v12.4h\n"     \
  "saddw2  v31.4s,  v31.4s,  v12.8h\n"     \
  "bne     0b\n"                           \
  "1:\n"                                   \
  "scvtf   v0.4s,   v16.4s\n"              \
  "scvtf   v1.4s,   v17.4s\n"              \
  "scvtf   v2.4s,   v18.4s\n"              \
  "scvtf   v3.4s,   v19.4s\n"              \
  "scvtf   v4.4s,   v20.4s\n"              \
  "scvtf   v5.4s,   v21.4s\n"              \
  "scvtf   v6.4s,   v22.4s\n"              \
  "scvtf   v7.4s,   v23.4s\n" /* scale */  \
  "scvtf   v8.4s,   v24.4s\n"              \
  "scvtf   v9.4s,   v25.4s\n"              \
  "scvtf   v10.4s,  v26.4s\n"              \
  "scvtf   v11.4s,  v27.4s\n"              \
  "scvtf   v12.4s,  v28.4s\n"              \
  "scvtf   v13.4s,  v29.4s\n"              \
  "scvtf   v14.4s,  v30.4s\n"              \
  "scvtf   v15.4s,  v31.4s\n"              \
  "dup     v21.4s,  %w[vbias]\n"           \
  "dup     v22.4s,  v21.s[0]\n"            \
  "dup     v23.4s,  v21.s[0]\n"            \
  "dup     v24.4s,  v21.s[0]\n"            \
  "dup     v25.4s,  v21.s[0]\n"            \
  "dup     v31.4s,  %w[vscale]\n"          \
  "dup     v26.4s,  v21.s[0]\n"            \
  "dup     v27.4s,  v21.s[0]\n"            \
  "dup     v28.4s,  v21.s[0]\n"            \
  "dup     v29.4s,  v21.s[0]\n"            \
  "dup     v30.4s,  v21.s[0]\n"            \
  "fmla    v21.4s,  v0.4s,  v31.s[0]\n"    \
  "fmla    v22.4s,  v1.4s,  v31.s[0]\n"    \
  "fmla    v23.4s,  v2.4s,  v31.s[0]\n"    \
  "fmla    v24.4s,  v3.4s,  v31.s[0]\n"    \
  "fmla    v25.4s,  v4.4s,  v31.s[0]\n"    \
  "fmla    v26.4s,  v5.4s,  v31.s[0]\n"    \
  "fmla    v27.4s,  v6.4s,  v31.s[0]\n"    \
  "fmla    v28.4s,  v7.4s,  v31.s[0]\n"    \
  "fmla    v29.4s,  v8.4s,  v31.s[0]\n"    \
  "fmla    v30.4s,  v9.4s,  v31.s[0]\n"    \
  "dup     v0.4s,  %w[vbias]\n"            \
  "dup     v1.4s,  v0.s[0]\n"              \
  "dup     v2.4s,  v0.s[0]\n"              \
  "dup     v3.4s,  v0.s[0]\n"              \
  "dup     v4.4s,  v0.s[0]\n"              \
  "dup     v5.4s,  v0.s[0]\n"              \
  "fmla    v0.4s,  v10.4s,  v31.s[0]\n"    \
  "fmla    v1.4s,  v11.4s,  v31.s[0]\n"    \
  "fmla    v2.4s,  v12.4s,  v31.s[0]\n"    \
  "fmla    v3.4s,  v13.4s,  v31.s[0]\n"    \
  "fmla    v4.4s,  v14.4s,  v31.s[0]\n"    \
  "fmla    v5.4s,  v15.4s,  v31.s[0]\n"

#define SPARSE_INT8_F32_W48_V8_KERNEL            \
  "eor v8.16b, v0.16b, v0.16b\n"                 \
  "eor v9.16b, v1.16b, v1.16b\n"                 \
  "eor v10.16b, v2.16b, v2.16b\n"                \
  "eor v11.16b, v3.16b, v3.16b\n"                \
  "eor v12.16b, v4.16b, v4.16b\n"                \
  "prfm  pldl1keep, [%[a_ptr], #32]\n"           \
  "eor v13.16b, v5.16b, v5.16b\n"                \
  "eor v14.16b, v6.16b, v6.16b\n"                \
  "prfm  pldl1keep, [%[widx_dmap], #32]\n"       \
  "eor v15.16b, v7.16b, v7.16b\n"                \
  "eor v16.16b, v0.16b, v0.16b\n"                \
  "prfm  pldl1keep, [%[b_ptr], #48]\n"           \
  "eor v17.16b, v1.16b, v1.16b\n"                \
  "eor v18.16b, v2.16b, v2.16b\n"                \
  "eor v19.16b, v3.16b, v3.16b\n"                \
  "dup     v20.4s,  %w[vbias]\n"                 \
  "dup     v21.4s,  v20.s[0]\n"                  \
  "dup     v22.4s,  v20.s[0]\n"                  \
  "dup     v23.4s,  v20.s[0]\n"                  \
  "dup     v24.4s,  v20.s[0]\n"                  \
  "dup     v25.4s,  v20.s[0]\n"                  \
  "dup     v26.4s,  v20.s[0]\n"                  \
  "dup     v27.4s,  v20.s[0]\n"                  \
  "dup     v28.4s,  v20.s[0]\n"                  \
  "dup     v29.4s,  v20.s[0]\n"                  \
  "dup     v30.4s,  v20.s[0]\n"                  \
  "dup     v31.4s,  v20.s[0]\n"                  \
  "cbz    %w[k],    1f\n"                        \
  "0:\n"                                         \
  "ld1r  {v0.16b}, [%[a_ptr]], #1\n"             \
  "ld1   {v1.16b, v2.16b, v3.16b}, [%[b_ptr]]\n" \
  "subs    %w[k],   %w[k],   #1\n"               \
  "ldrsw   x1, [%[widx_dmap]],   #4\n"           \
  "smull   v4.8h,   v0.8b,   v1.8b\n"            \
  "smull2  v5.8h,   v0.16b,  v1.16b\n"           \
  "add   %[b_ptr],  %[b_ptr], x1\n"              \
  "smull   v6.8h,   v0.8b,   v2.8b\n"            \
  "smull2  v7.8h,   v0.16b,  v2.16b\n"           \
  "saddw   v8.4s,  v8.4s,  v4.4h\n"              \
  "saddw2  v9.4s,  v9.4s,  v4.8h\n"              \
  "prfm  pldl1keep, [%[b_ptr], #48]\n"           \
  "saddw   v10.4s,  v10.4s,  v5.4h\n"            \
  "saddw2  v11.4s,  v11.4s,  v5.8h\n"            \
  "smull   v4.8h,   v0.8b,   v3.8b\n"            \
  "saddw   v12.4s,  v12.4s,  v6.4h\n"            \
  "saddw2  v13.4s,  v13.4s,  v6.8h\n"            \
  "saddw   v14.4s,  v14.4s,  v7.4h\n"            \
  "smull2  v5.8h,   v0.16b,  v3.16b\n"           \
  "saddw2  v15.4s,  v15.4s,  v7.8h\n"            \
  "saddw   v16.4s,  v16.4s,  v4.4h\n"            \
  "saddw2  v17.4s,  v17.4s,  v4.8h\n"            \
  "saddw   v18.4s,  v18.4s,  v5.4h\n"            \
  "saddw2  v19.4s,  v19.4s,  v5.8h\n"            \
  "bne     0b\n"                                 \
  "1:\n"                                         \
  "dup     v0.4s,  %w[vscale]\n"                 \
  "scvtf   v1.4s,  v8.4s\n"                      \
  "scvtf   v2.4s,  v9.4s\n"                      \
  "scvtf   v3.4s,  v10.4s\n"                     \
  "scvtf   v4.4s,  v11.4s\n"                     \
  "scvtf   v5.4s,  v12.4s\n"                     \
  "scvtf   v6.4s,  v13.4s\n"                     \
  "scvtf   v7.4s,  v14.4s\n" /* scale */         \
  "fmla    v20.4s,  v1.4s,  v0.s[0]\n"           \
  "fmla    v21.4s,  v2.4s,  v0.s[0]\n"           \
  "fmla    v22.4s,  v3.4s,  v0.s[0]\n"           \
  "scvtf   v1.4s,  v15.4s\n"                     \
  "scvtf   v2.4s,  v16.4s\n"                     \
  "scvtf   v3.4s,  v17.4s\n"                     \
  "fmla    v23.4s,  v4.4s,  v0.s[0]\n"           \
  "fmla    v24.4s,  v5.4s,  v0.s[0]\n"           \
  "fmla    v25.4s,  v6.4s,  v0.s[0]\n"           \
  "fmla    v26.4s,  v7.4s,  v0.s[0]\n"           \
  "scvtf   v4.4s,  v18.4s\n"                     \
  "scvtf   v5.4s,  v19.4s\n" /* scale */         \
  "fmla    v27.4s,  v1.4s,  v0.s[0]\n"           \
  "fmla    v28.4s,  v2.4s,  v0.s[0]\n"           \
  "fmla    v29.4s,  v3.4s,  v0.s[0]\n"           \
  "fmla    v30.4s,  v4.4s,  v0.s[0]\n"           \
  "fmla    v31.4s,  v5.4s,  v0.s[0]\n"

#define SPARSE_INT8_F32_W32_V8_KERNEL      \
  "eor v11.16b, v0.16b, v0.16b\n"          \
  "eor v12.16b, v1.16b, v1.16b\n"          \
  "prfm  pldl1keep, [%[a_ptr], #32]\n"     \
  "eor v13.16b, v2.16b, v2.16b\n"          \
  "eor v14.16b, v3.16b, v3.16b\n"          \
  "prfm  pldl1keep, [%[widx_dmap], #32]\n" \
  "eor v15.16b, v4.16b, v4.16b\n"          \
  "eor v16.16b, v5.16b, v5.16b\n"          \
  "prfm  pldl1keep, [%[b_ptr], #32]\n"     \
  "eor v17.16b, v6.16b, v6.16b\n"          \
  "eor v18.16b, v7.16b, v7.16b\n"          \
  "dup     v21.4s,  %w[vbias]\n"           \
  "dup     v22.4s,  v21.s[0]\n"            \
  "dup     v23.4s,  v21.s[0]\n"            \
  "dup     v24.4s,  v21.s[0]\n"            \
  "dup     v25.4s,  v21.s[0]\n"            \
  "dup     v26.4s,  v21.s[0]\n"            \
  "dup     v27.4s,  v21.s[0]\n"            \
  "dup     v28.4s,  v21.s[0]\n"            \
  "cbz    %w[k],    1f\n"                  \
  "0:\n"                                   \
  "ld1r  {v0.16b}, [%[a_ptr]], #1\n"       \
  "ldrsw   x1, [%[widx_dmap]],   #4\n"     \
  "ld1   {v1.16b, v2.16b}, [%[b_ptr]]\n"   \
  "subs    %w[k],   %w[k],   #1\n"         \
  "smull   v3.8h,   v0.8b,   v1.8b\n"      \
  "add   %[b_ptr],  %[b_ptr], x1\n"        \
  "smull2  v4.8h,   v0.16b,  v1.16b\n"     \
  "smull   v5.8h,   v0.8b,   v2.8b\n"      \
  "smull2  v6.8h,   v0.16b,  v2.16b\n"     \
  "saddw   v11.4s,  v11.4s,  v3.4h\n"      \
  "saddw   v13.4s,  v13.4s,  v4.4h\n"      \
  "prfm  pldl1keep, [%[b_ptr], #64]\n"     \
  "saddw   v15.4s,  v15.4s,  v5.4h\n"      \
  "saddw   v17.4s,  v17.4s,  v6.4h\n"      \
  "saddw2  v12.4s,  v12.4s,  v3.8h\n"      \
  "saddw2  v14.4s,  v14.4s,  v4.8h\n"      \
  "saddw2  v16.4s,  v16.4s,  v5.8h\n"      \
  "saddw2  v18.4s,  v18.4s,  v6.8h\n"      \
  "bne     0b\n"                           \
  "1:\n"                                   \
  "scvtf   v3.4s,  v11.4s\n"               \
  "scvtf   v4.4s,  v12.4s\n"               \
  "scvtf   v5.4s,  v13.4s\n"               \
  "scvtf   v6.4s,  v14.4s\n"               \
  "scvtf   v7.4s,  v15.4s\n"               \
  "scvtf   v8.4s,  v16.4s\n"               \
  "scvtf   v9.4s,  v17.4s\n"               \
  "scvtf   v10.4s, v18.4s\n" /* scale */   \
  "dup     v31.4s,  %w[vscale]\n"          \
  "fmla    v21.4s,  v3.4s,  v31.s[0]\n"    \
  "fmla    v22.4s,  v4.4s,  v31.s[0]\n"    \
  "fmla    v23.4s,  v5.4s,  v31.s[0]\n"    \
  "fmla    v24.4s,  v6.4s,  v31.s[0]\n"    \
  "fmla    v25.4s,  v7.4s,  v31.s[0]\n"    \
  "fmla    v26.4s,  v8.4s,  v31.s[0]\n"    \
  "fmla    v27.4s,  v9.4s,  v31.s[0]\n"    \
  "fmla    v28.4s,  v10.4s, v31.s[0]\n"

#define SPARSE_INT8_F32_W16_V8_KERNEL      \
  "eor v11.16b, v0.16b, v0.16b\n"          \
  "prfm  pldl1keep, [%[a_ptr], #16]\n"     \
  "eor v12.16b, v1.16b, v1.16b\n"          \
  "eor v13.16b, v2.16b, v2.16b\n"          \
  "prfm  pldl1keep, [%[b_ptr], #16]\n"     \
  "eor v14.16b, v3.16b, v3.16b\n"          \
  "dup     v21.4s,  %w[vbias]\n"           \
  "dup     v22.4s,  v21.s[0]\n"            \
  "prfm  pldl1keep, [%[widx_dmap], #16]\n" \
  "dup     v23.4s,  v21.s[0]\n"            \
  "dup     v24.4s,  v21.s[0]\n"            \
  "cbz    %w[k],    1f\n"                  \
  "0:\n"                                   \
  "ld1r  {v0.16b}, [%[a_ptr]], #1\n"       \
  "ldrsw   x1, [%[widx_dmap]],   #4\n"     \
  "ld1   {v1.16b}, [%[b_ptr]]\n"           \
  "subs    %w[k],   %w[k],   #1\n"         \
  "smull   v3.8h,   v0.8b,   v1.8b\n"      \
  "smull2  v4.8h,   v0.16b,  v1.16b\n"     \
  "add   %[b_ptr],  %[b_ptr], x1\n"        \
  "saddw   v11.4s,  v11.4s,  v3.4h\n"      \
  "saddw2  v12.4s,  v12.4s,  v3.8h\n"      \
  "prfm  pldl1keep, [%[b_ptr], #32]\n"     \
  "saddw   v13.4s,  v13.4s,  v4.4h\n"      \
  "saddw2  v14.4s,  v14.4s,  v4.8h\n"      \
  "bne     0b\n"                           \
  "1:\n"                                   \
  "scvtf   v5.4s,  v11.4s\n"               \
  "scvtf   v6.4s,  v12.4s\n"               \
  "scvtf   v7.4s,  v13.4s\n"               \
  "scvtf   v8.4s,  v14.4s\n" /* scale */   \
  "dup     v2.4s,  %w[vscale]\n"           \
  "fmla    v21.4s,  v5.4s,  v2.s[0]\n"     \
  "fmla    v22.4s,  v6.4s,  v2.s[0]\n"     \
  "fmla    v23.4s,  v7.4s,  v2.s[0]\n"     \
  "fmla    v24.4s,  v8.4s,  v2.s[0]\n"

#define SPARSE_INT8_F32_W8_V8_KERNEL       \
  "eor v11.16b, v0.16b, v0.16b\n"          \
  "prfm  pldl1keep, [%[widx_dmap], #16]\n" \
  "eor v12.16b, v1.16b, v1.16b\n"          \
  "dup     v21.4s,  %w[vbias]\n"           \
  "prfm  pldl1keep, [%[a_ptr], #16]\n"     \
  "dup     v22.4s,  v21.s[0]\n"            \
  "prfm  pldl1keep, [%[b_ptr], #16]\n"     \
  "cbz    %w[k],    1f\n"                  \
  "0:\n"                                   \
  "ld1r  {v0.8b}, [%[a_ptr]], #1\n"        \
  "ldr   w1, [%[widx_dmap]],   #4\n"       \
  "sxtw  x1,  w1\n"                        \
  "ld1   {v1.8b}, [%[b_ptr]]\n"            \
  "add   %[b_ptr],  %[b_ptr], x1\n"        \
  "smull   v3.8h,   v0.8b,   v1.8b\n"      \
  "subs    %w[k],   %w[k],   #1\n"         \
  "saddw   v11.4s,  v11.4s,  v3.4h\n"      \
  "saddw2  v12.4s,  v12.4s,  v3.8h\n"      \
  "bne     0b\n"                           \
  "1:\n"                                   \
  "scvtf   v4.4s,  v11.4s\n"               \
  "scvtf   v5.4s,  v12.4s\n" /* scale */   \
  "dup     v2.4s,   %w[vscale]\n"          \
  "fmla    v21.4s,  v4.4s,  v2.s[0]\n"     \
  "fmla    v22.4s,  v5.4s,  v2.s[0]\n"

#define SPARSE_INT8_F32_W4_V8_KERNEL       \
  "eor v11.16b, v0.16b, v0.16b\n"          \
  "prfm  pldl1keep, [%[widx_dmap], #16]\n" \
  "dup     v21.4s,  %w[vbias]\n"           \
  "prfm  pldl1keep, [%[a_ptr], #16]\n"     \
  "cbz    %w[k],    1f       \n"           \
  "cmp    %w[k],   #1        \n"           \
  "beq    4f                \n"            \
  "0:\n"                                   \
  "ld1r  {v0.8b}, [%[a_ptr]], #1\n"        \
  "sub    %w[k],   %w[k],   #1\n"          \
  "ldrsw   x1, [%[widx_dmap]],   #4\n"     \
  "ld1   {v1.8b}, [%[b_ptr]]\n"            \
  "add   %[b_ptr],  %[b_ptr], x1\n"        \
  "smull   v3.8h,   v0.8b,   v1.8b\n"      \
  "saddw   v11.4s,  v11.4s,  v3.4h\n"      \
  "cmp    %w[k],    #1          \n"        \
  "bne    0b                    \n"        \
  "4:\n"                                   \
  "ld1r  {v0.8b}, [%[a_ptr]], #1\n"        \
  "ldr   w1, [%[widx_dmap]],   #4\n"       \
  "sub    %w[k],   %w[k],   #1\n"          \
  "ldrsb   w2, [%[b_ptr]]\n"               \
  "ldrsb   w3, [%[b_ptr], #1]\n"           \
  "ldrsb   w4, [%[b_ptr], #2]\n"           \
  "ldrsb   w5, [%[b_ptr], #3]\n"           \
  "sxtw  x1,  w1\n"                        \
  "mov   v1.b[0], w2\n"                    \
  "mov   v1.b[1], w3\n"                    \
  "mov   v1.b[2], w4\n"                    \
  "mov   v1.b[3], w5\n"                    \
  "add   %[b_ptr],  %[b_ptr], x1\n"        \
  "smull   v3.8h,   v0.8b,   v1.8b\n"      \
  "saddw   v11.4s,  v11.4s,  v3.4h\n"      \
  "1:\n"                                   \
  "scvtf   v4.4s,  v11.4s\n" /* scale */   \
  "dup     v2.4s,   %w[vscale]\n"          \
  "fmla    v21.4s,  v4.4s,  v2.s[0]\n"

#define SPARSE_INT8_F32_W1_V8_KERNEL    \
  "eor v3.16b, v0.16b, v0.16b\n"        \
  "dup     v0.4s,  %w[vbias]\n"         \
  "cbz    %w[n],    2f\n"               \
  "0:\n"                                \
  "ld1   {v2.8b}, [%[a_ptr]], #8\n"     \
  "ld1   {v1.b}[0], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[1], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[2], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[3], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[4], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[5], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[6], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[7], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "smull   v4.8h,   v1.8b,   v2.8b\n"   \
  "subs    %w[n],   %w[n],   #1\n"      \
  "sadalp  v3.4s,  v4.8h    \n"         \
  "bne     0b\n"                        \
  "2:\n"                                \
  "cbz    %w[m],    1f\n"               \
  "eor v5.16b, v4.16b, v4.16b\n"        \
  "eor v6.16b, v4.16b, v4.16b\n"        \
  "3:\n"                                \
  "ld1   {v5.b}[0], [%[a_ptr]], #1\n"   \
  "ld1   {v6.b}[0], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "smull   v4.8h,   v5.8b,   v6.8b\n"   \
  "saddw   v3.4s,  v3.4s,  v4.4h\n"     \
  "subs    %w[m],   %w[m],   #1\n"      \
  "bne     3b\n"                        \
  "1:\n"                                \
  "addv    s2,      v3.4s       \n"     \
  "scvtf   v1.4s,  v2.4s\n" /* scale */ \
  "dup     v2.4s,   %w[vscale]\n"       \
  "fmadd   s0, s1, s2, s0   \n"

#define SPARSE_INT8_F32_W64_V8_RELU                   \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v6.4s, #0\n"               /* for relu */   \
  "fmax   v21.4s, v21.4s, v6.4s\n"   /* relu */       \
  "fmax   v22.4s, v22.4s, v6.4s\n"   /* relu */       \
  "fmax   v23.4s, v23.4s, v6.4s\n"   /* relu */       \
  "fmax   v24.4s, v24.4s, v6.4s\n"   /* relu */       \
  "fmax   v25.4s, v25.4s, v6.4s\n"   /* relu */       \
  "fmax   v26.4s, v26.4s, v6.4s\n"   /* relu */       \
  "fmax   v27.4s, v27.4s, v6.4s\n"   /* relu */       \
  "fmax   v28.4s, v28.4s, v6.4s\n"   /* relu */       \
  "fmax   v29.4s, v29.4s, v6.4s\n"   /* relu */       \
  "fmax   v30.4s, v30.4s, v6.4s\n"   /* relu */       \
  "fmax   v0.4s,  v0.4s,  v6.4s\n"   /* relu */       \
  "fmax   v1.4s,  v1.4s,  v6.4s\n"   /* relu */       \
  "fmax   v2.4s,  v2.4s,  v6.4s\n"   /* relu */       \
  "fmax   v3.4s,  v3.4s,  v6.4s\n"   /* relu */       \
  "fmax   v4.4s,  v4.4s,  v6.4s\n"   /* relu */       \
  "fmax   v5.4s,  v5.4s,  v6.4s\n"   /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W48_V8_RELU                   \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v0.4s, #0\n"               /* for relu */   \
  "fmax   v20.4s, v20.4s, v0.4s\n"   /* relu */       \
  "fmax   v21.4s, v21.4s, v0.4s\n"   /* relu */       \
  "fmax   v22.4s, v22.4s, v0.4s\n"   /* relu */       \
  "fmax   v23.4s, v23.4s, v0.4s\n"   /* relu */       \
  "fmax   v24.4s, v24.4s, v0.4s\n"   /* relu */       \
  "fmax   v25.4s, v25.4s, v0.4s\n"   /* relu */       \
  "fmax   v26.4s, v26.4s, v0.4s\n"   /* relu */       \
  "fmax   v27.4s, v27.4s, v0.4s\n"   /* relu */       \
  "fmax   v28.4s, v28.4s, v0.4s\n"   /* relu */       \
  "fmax   v29.4s, v29.4s, v0.4s\n"   /* relu */       \
  "fmax   v30.4s, v30.4s, v0.4s\n"   /* relu */       \
  "fmax   v31.4s, v31.4s, v0.4s\n"   /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W32_V8_RELU                   \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v30.4s, #0\n"              /* for relu */   \
  "fmax   v21.4s, v21.4s, v30.4s\n"  /* relu */       \
  "fmax   v22.4s, v22.4s, v30.4s\n"  /* relu */       \
  "fmax   v23.4s, v23.4s, v30.4s\n"  /* relu */       \
  "fmax   v24.4s, v24.4s, v30.4s\n"  /* relu */       \
  "fmax   v25.4s, v25.4s, v30.4s\n"  /* relu */       \
  "fmax   v26.4s, v26.4s, v30.4s\n"  /* relu */       \
  "fmax   v27.4s, v27.4s, v30.4s\n"  /* relu */       \
  "fmax   v28.4s, v28.4s, v30.4s\n"  /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W16_V8_RELU                   \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v9.4s, #0\n"               /* for relu */   \
  "fmax   v21.4s, v21.4s, v9.4s\n"   /* relu */       \
  "fmax   v22.4s, v22.4s, v9.4s\n"   /* relu */       \
  "fmax   v23.4s, v23.4s, v9.4s\n"   /* relu */       \
  "fmax   v24.4s, v24.4s, v9.4s\n"   /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W8_V8_RELU                    \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v9.4s, #0\n"               /* for relu */   \
  "fmax   v21.4s, v21.4s, v9.4s\n"   /* relu */       \
  "fmax   v22.4s, v22.4s, v9.4s\n"   /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W4_V8_RELU                    \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v9.4s, #0\n"               /* for relu */   \
  "fmax   v21.4s, v21.4s, v9.4s\n"   /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W1_V8_RELU                    \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v9.4s, #0\n"               /* for relu */   \
  "fmax   s0,  s0,  s9\n"                             \
  "b      9f                \n"

#define SPARSE_INT8_F32_W64_V8_RELU6                    \
  /* do relu6 */                                        \
  "10: \n"                                              \
  "cmp   %w[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n"  /* no act end */  \
  "movi   v6.4s, #0\n"                /* for relu6 */   \
  "dup    v7.4s,  %w[valpha]\n"       /* relu6 alpha */ \
  "fmax   v21.4s, v21.4s, v6.4s\n"    /* relu */        \
  "fmax   v22.4s, v22.4s, v6.4s\n"    /* relu */        \
  "fmax   v23.4s, v23.4s, v6.4s\n"    /* relu */        \
  "fmax   v24.4s, v24.4s, v6.4s\n"    /* relu */        \
  "fmax   v25.4s, v25.4s, v6.4s\n"    /* relu */        \
  "fmax   v26.4s, v26.4s, v6.4s\n"    /* relu */        \
  "fmax   v27.4s, v27.4s, v6.4s\n"    /* relu */        \
  "fmax   v28.4s, v28.4s, v6.4s\n"    /* relu */        \
  "fmax   v29.4s, v29.4s, v6.4s\n"    /* relu */        \
  "fmax   v30.4s, v30.4s, v6.4s\n"    /* relu */        \
  "fmax   v0.4s,  v0.4s,  v6.4s\n"    /* relu */        \
  "fmax   v1.4s,  v1.4s,  v6.4s\n"    /* relu */        \
  "fmax   v2.4s,  v2.4s,  v6.4s\n"    /* relu */        \
  "fmax   v3.4s,  v3.4s,  v6.4s\n"    /* relu */        \
  "fmax   v4.4s,  v4.4s,  v6.4s\n"    /* relu */        \
  "fmax   v5.4s,  v5.4s,  v6.4s\n"    /* relu */        \
  "fmin   v21.4s, v21.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v22.4s, v22.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v23.4s, v23.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v24.4s, v24.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v25.4s, v25.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v26.4s, v26.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v27.4s, v27.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v28.4s, v28.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v29.4s, v29.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v30.4s, v30.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v0.4s,  v0.4s,  v7.4s\n"    /* relu6 */       \
  "fmin   v1.4s,  v1.4s,  v7.4s\n"    /* relu6 */       \
  "fmin   v2.4s,  v2.4s,  v7.4s\n"    /* relu6 */       \
  "fmin   v3.4s,  v3.4s,  v7.4s\n"    /* relu6 */       \
  "fmin   v4.4s,  v4.4s,  v7.4s\n"    /* relu6 */       \
  "fmin   v5.4s,  v5.4s,  v7.4s\n"    /* relu6 */       \
  "b      9f                    \n"   /* relu end */

#define SPARSE_INT8_F32_W48_V8_RELU6                    \
  /* do relu6 */                                        \
  "10: \n"                                              \
  "cmp   %w[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n"  /* no act end */  \
  "movi   v0.4s, #0\n"                /* for relu6 */   \
  "dup    v1.4s,  %w[valpha]\n"       /* relu6 alpha */ \
  "fmax   v20.4s, v20.4s, v0.4s\n"    /* relu */        \
  "fmax   v21.4s, v21.4s, v0.4s\n"    /* relu */        \
  "fmax   v22.4s, v22.4s, v0.4s\n"    /* relu */        \
  "fmax   v23.4s, v23.4s, v0.4s\n"    /* relu */        \
  "fmax   v24.4s, v24.4s, v0.4s\n"    /* relu */        \
  "fmax   v25.4s, v25.4s, v0.4s\n"    /* relu */        \
  "fmax   v26.4s, v26.4s, v0.4s\n"    /* relu */        \
  "fmax   v27.4s, v27.4s, v0.4s\n"    /* relu */        \
  "fmax   v28.4s, v28.4s, v0.4s\n"    /* relu */        \
  "fmax   v29.4s, v29.4s, v0.4s\n"    /* relu */        \
  "fmax   v30.4s, v30.4s, v0.4s\n"    /* relu */        \
  "fmax   v31.4s, v31.4s, v0.4s\n"    /* relu */        \
  "fmin   v20.4s, v20.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v21.4s, v21.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v22.4s, v22.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v23.4s, v23.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v24.4s, v24.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v25.4s, v25.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v26.4s, v26.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v27.4s, v27.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v28.4s, v28.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v29.4s, v29.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v30.4s, v30.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v31.4s, v31.4s, v1.4s\n"    /* relu6 */       \
  "b      9f                    \n"   /* relu end */

#define SPARSE_INT8_F32_W32_V8_RELU6                    \
  /* do relu6 */                                        \
  "10: \n"                                              \
  "cmp   %w[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n"  /* no act end */  \
  "movi   v0.4s, #0\n"                /* for relu6 */   \
  "dup    v1.4s,  %w[valpha]\n"       /* relu6 alpha */ \
  "fmax   v21.4s, v21.4s, v0.4s\n"    /* relu */        \
  "fmax   v22.4s, v22.4s, v0.4s\n"    /* relu */        \
  "fmax   v23.4s, v23.4s, v0.4s\n"    /* relu */        \
  "fmax   v24.4s, v24.4s, v0.4s\n"    /* relu */        \
  "fmax   v25.4s, v25.4s, v0.4s\n"    /* relu */        \
  "fmax   v26.4s, v26.4s, v0.4s\n"    /* relu */        \
  "fmax   v27.4s, v27.4s, v0.4s\n"    /* relu */        \
  "fmax   v28.4s, v28.4s, v0.4s\n"    /* relu */        \
  "fmin   v21.4s, v21.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v22.4s, v22.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v23.4s, v23.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v24.4s, v24.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v25.4s, v25.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v26.4s, v26.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v27.4s, v27.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v28.4s, v28.4s, v1.4s\n"    /* relu6 */       \
  "b      9f                    \n"   /* relu end */

#define SPARSE_INT8_F32_W16_V8_RELU6                    \
  /* do relu6 */                                        \
  "10: \n"                                              \
  "cmp   %w[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n"  /* no act end */  \
  "movi   v0.4s, #0\n"                /* for relu6 */   \
  "dup    v1.4s,  %w[valpha]\n"       /* relu6 alpha */ \
  "fmax   v21.4s, v21.4s, v0.4s\n"    /* relu */        \
  "fmax   v22.4s, v22.4s, v0.4s\n"    /* relu */        \
  "fmax   v23.4s, v23.4s, v0.4s\n"    /* relu */        \
  "fmax   v24.4s, v24.4s, v0.4s\n"    /* relu */        \
  "fmin   v21.4s, v21.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v22.4s, v22.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v23.4s, v23.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v24.4s, v24.4s, v1.4s\n"    /* relu6 */       \
  "b      9f                    \n"   /* relu end */

#define SPARSE_INT8_F32_W8_V8_RELU6                     \
  /* do relu6 */                                        \
  "10: \n"                                              \
  "cmp   %w[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n"  /* no act end */  \
  "movi   v0.4s, #0\n"                /* for relu6 */   \
  "dup    v1.4s,  %w[valpha]\n"       /* relu6 alpha */ \
  "fmax   v21.4s, v21.4s, v0.4s\n"    /* relu */        \
  "fmax   v22.4s, v22.4s, v0.4s\n"    /* relu */        \
  "fmin   v21.4s, v21.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v22.4s, v22.4s, v1.4s\n"    /* relu6 */       \
  "b      9f                    \n"   /* relu end */

#define SPARSE_INT8_F32_W4_V8_RELU6                     \
  /* do relu6 */                                        \
  "10: \n"                                              \
  "cmp   %w[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n"  /* no act end */  \
  "movi   v0.4s, #0\n"                /* for relu6 */   \
  "dup    v1.4s,  %w[valpha]\n"       /* relu6 alpha */ \
  "fmax   v21.4s, v21.4s, v0.4s\n"    /* relu */        \
  "fmin   v21.4s, v21.4s, v1.4s\n"    /* relu6 */       \
  "b      9f                    \n"   /* relu end */

#define SPARSE_INT8_F32_W1_V8_RELU6   \
  /* do relu6 */                      \
  "10: \n"                            \
  "cmp   %w[vflag_act],  #2       \n" \
  "bne   11f                     \n"  \
  "movi   v9.4s, #0\n"                \
  "dup    v1.4s,  %w[valpha]\n"       \
  "fmax   s0,  s0,  s9\n"             \
  "fmin   s0,  s0,  s1\n"             \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W64_V8_LEAKY_RELU                           \
  /* do relu */                                                     \
  "11: \n"                                                          \
  "cmp   %w[vflag_act],  #3       \n"       /* check leakey relu */ \
  "bne   12f                     \n"        /* no act end */        \
  "movi   v6.4s, #0\n"                      /* for relu6 */         \
  "dup    v7.4s,  %w[valpha]\n"             /* leakey relu alpha */ \
  "fcmge  v8.4s,    v21.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v9.4s,    v21.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v10.4s,   v22.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v11.4s,   v22.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v12.4s,   v23.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v13.4s,   v23.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v14.4s,   v24.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v15.4s,   v24.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "bif    v21.16b,  v9.16b,    v8.16b  \n"  /* choose*/             \
  "bif    v22.16b,  v11.16b,   v10.16b  \n" /* choose*/             \
  "bif    v23.16b,  v13.16b,   v12.16b  \n" /* choose*/             \
  "bif    v24.16b,  v15.16b,   v14.16b  \n" /* choose*/             \
  "fcmge  v8.4s,    v25.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v9.4s,    v25.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v10.4s,   v26.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v11.4s,   v26.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v12.4s,   v27.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v13.4s,   v27.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v14.4s,   v28.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v15.4s,   v28.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "bif    v25.16b,  v9.16b,    v8.16b  \n"  /* choose*/             \
  "bif    v26.16b,  v11.16b,   v10.16b  \n" /* choose*/             \
  "bif    v27.16b,  v13.16b,   v12.16b  \n" /* choose*/             \
  "bif    v28.16b,  v15.16b,   v14.16b  \n" /* choose*/             \
  "fcmge  v8.4s,    v29.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v9.4s,    v29.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v10.4s,   v30.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v11.4s,   v30.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v12.4s,   v0.4s,     v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v13.4s,   v0.4s,     v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v14.4s,   v1.4s,     v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v15.4s,   v1.4s,     v7.4s   \n"  /* vmulq_f32 */         \
  "bif    v29.16b,  v9.16b,    v8.16b  \n"  /* choose*/             \
  "bif    v30.16b,  v11.16b,   v10.16b  \n" /* choose*/             \
  "bif    v0.16b,   v13.16b,   v12.16b  \n" /* choose*/             \
  "bif    v1.16b,   v15.16b,   v14.16b  \n" /* choose*/             \
  "fcmge  v8.4s,    v2.4s,    v6.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v9.4s,    v2.4s,    v7.4s   \n"   /* vmulq_f32 */         \
  "fcmge  v10.4s,   v3.4s,    v6.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v11.4s,   v3.4s,    v7.4s   \n"   /* vmulq_f32 */         \
  "fcmge  v12.4s,   v4.4s,    v6.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v13.4s,   v4.4s,    v7.4s   \n"   /* vmulq_f32 */         \
  "fcmge  v14.4s,   v5.4s,    v6.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v15.4s,   v5.4s,    v7.4s   \n"   /* vmulq_f32 */         \
  "bif    v2.16b,   v9.16b,   v8.16b  \n"   /* choose*/             \
  "bif    v3.16b,   v11.16b,  v10.16b  \n"  /* choose*/             \
  "bif    v4.16b,   v13.16b,  v12.16b  \n"  /* choose*/             \
  "bif    v5.16b,   v15.16b,  v14.16b  \n"  /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W48_V8_LEAKY_RELU                           \
  /* do relu */                                                     \
  "11: \n"                                                          \
  "cmp   %w[vflag_act],  #3       \n"       /* check leakey relu */ \
  "bne   12f                     \n"        /* no act end */        \
  "movi   v0.4s, #0\n"                      /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"             /* leakey relu alpha */ \
  "fcmge  v2.4s,    v20.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v3.4s,    v20.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v4.4s,    v21.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v5.4s,    v21.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v6.4s,    v22.4s,   v0.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v7.4s,    v22.4s,   v1.4s   \n"   /* vmulq_f32 */         \
  "fcmge  v8.4s,    v23.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v9.4s,    v23.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v10.4s,    v24.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v11.4s,    v24.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v12.4s,    v25.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v13.4s,    v25.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v20.16b,   v3.16b,   v2.16b  \n"  /* choose*/             \
  "bif    v21.16b,   v5.16b,   v4.16b  \n"  /* choose*/             \
  "bif    v22.16b,  v7.16b,   v6.16b  \n"   /* choose*/             \
  "bif    v23.16b,  v9.16b,   v8.16b  \n"   /* choose*/             \
  "bif    v24.16b,  v11.16b,   v10.16b  \n" /* choose*/             \
  "bif    v25.16b,  v13.16b,   v12.16b  \n" /* choose*/             \
  "fcmge  v2.4s,    v26.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v3.4s,    v26.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v4.4s,    v27.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v5.4s,    v27.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v6.4s,    v28.4s,   v0.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v7.4s,    v28.4s,   v1.4s   \n"   /* vmulq_f32 */         \
  "fcmge  v8.4s,    v29.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v9.4s,    v29.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v10.4s,    v30.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v11.4s,    v30.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v12.4s,    v31.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v13.4s,    v31.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v26.16b,   v3.16b,   v2.16b  \n"  /* choose*/             \
  "bif    v27.16b,   v5.16b,   v4.16b  \n"  /* choose*/             \
  "bif    v28.16b,  v7.16b,   v6.16b  \n"   /* choose*/             \
  "bif    v29.16b,  v9.16b,   v8.16b  \n"   /* choose*/             \
  "bif    v30.16b,  v11.16b,   v10.16b  \n" /* choose*/             \
  "bif    v31.16b,  v13.16b,   v12.16b  \n" /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W32_V8_LEAKY_RELU                          \
  /* do relu */                                                    \
  "11: \n"                                                         \
  "cmp   %w[vflag_act],  #3       \n"      /* check leakey relu */ \
  "bne   12f                     \n"       /* no act end */        \
  "movi   v0.4s, #0\n"                     /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"            /* leakey relu alpha */ \
  "fcmge  v2.4s,    v21.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v21.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v4.4s,    v22.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v5.4s,    v22.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v6.4s,    v23.4s,   v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v7.4s,    v23.4s,   v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v8.4s,    v24.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v9.4s,    v24.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v21.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "bif    v22.16b,   v5.16b,   v4.16b  \n" /* choose*/             \
  "bif    v23.16b,  v7.16b,   v6.16b  \n"  /* choose*/             \
  "bif    v24.16b,  v9.16b,   v8.16b  \n"  /* choose*/             \
  "fcmge  v2.4s,    v25.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v25.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v4.4s,    v26.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v5.4s,    v26.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v6.4s,    v27.4s,   v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v7.4s,    v27.4s,   v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v8.4s,    v28.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v9.4s,    v28.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v25.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "bif    v26.16b,   v5.16b,   v4.16b  \n" /* choose*/             \
  "bif    v27.16b,  v7.16b,   v6.16b  \n"  /* choose*/             \
  "bif    v28.16b,  v9.16b,   v8.16b  \n"  /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W16_V8_LEAKY_RELU                          \
  /* do relu */                                                    \
  "11: \n"                                                         \
  "cmp   %w[vflag_act],  #3       \n"      /* check leakey relu */ \
  "bne   12f                     \n"       /* no act end */        \
  "movi   v0.4s, #0\n"                     /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"            /* leakey relu alpha */ \
  "fcmge  v2.4s,    v21.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v21.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v4.4s,    v22.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v5.4s,    v22.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v6.4s,    v23.4s,   v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v7.4s,    v23.4s,   v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v8.4s,    v24.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v9.4s,    v24.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v21.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "bif    v22.16b,   v5.16b,   v4.16b  \n" /* choose*/             \
  "bif    v23.16b,  v7.16b,   v6.16b  \n"  /* choose*/             \
  "bif    v24.16b,  v9.16b,   v8.16b  \n"  /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W8_V8_LEAKY_RELU                           \
  /* do relu */                                                    \
  "11: \n"                                                         \
  "cmp   %w[vflag_act],  #3       \n"      /* check leakey relu */ \
  "bne   12f                     \n"       /* no act end */        \
  "movi   v0.4s, #0\n"                     /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"            /* leakey relu alpha */ \
  "fcmge  v2.4s,    v21.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v21.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v4.4s,    v22.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v5.4s,    v22.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v21.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "bif    v22.16b,   v5.16b,   v4.16b  \n" /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W4_V8_LEAKY_RELU                           \
  /* do relu */                                                    \
  "11: \n"                                                         \
  "cmp   %w[vflag_act],  #3       \n"      /* check leakey relu */ \
  "bne   12f                     \n"       /* no act end */        \
  "movi   v0.4s, #0\n"                     /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"            /* leakey relu alpha */ \
  "fcmge  v2.4s,    v21.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v21.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v21.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W1_V8_LEAKY_RELU                          \
  /* do relu */                                                   \
  "11: \n"                                                        \
  "cmp    %w[vflag_act],  #3       \n"                            \
  "bne    12f                     \n"                             \
  "movi   v9.4s, #0\n"                    /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"           /* leakey relu alpha */ \
  "fcmge  s2,    s0,    s9   \n"          /* vcgeq_f32 */         \
  "fmul   s3,    s0,    s1   \n"          /* vmulq_f32 */         \
  "bif    v0.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W64_V8_HARD_SWISH                        \
  /* do relu */                                                  \
  "12: \n"                                                       \
  "movi   v6.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q7,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q8,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q9,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v10.4s,  v21.4s, v7.4s        \n"                      \
  "fadd   v12.4s,  v22.4s, v7.4s        \n"                      \
  "fadd   v14.4s,  v23.4s, v7.4s        \n"                      \
  "fmul   v11.4s,  v21.4s, v8.4s        \n"                      \
  "fmul   v13.4s,  v22.4s, v8.4s        \n"                      \
  "fmul   v15.4s,  v23.4s, v8.4s        \n"                      \
  "fmax   v10.4s, v10.4s, v6.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v6.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v6.4s       \n"                        \
  "fmin   v10.4s, v10.4s, v9.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v9.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v9.4s       \n"                        \
  "fmul   v21.4s, v11.4s, v10.4s        \n"                      \
  "fmul   v22.4s, v13.4s, v12.4s        \n"                      \
  "fmul   v23.4s, v15.4s, v14.4s        \n"                      \
  "fadd   v10.4s,  v24.4s, v7.4s        \n"                      \
  "fadd   v12.4s,  v25.4s, v7.4s        \n"                      \
  "fadd   v14.4s,  v26.4s, v7.4s        \n"                      \
  "fmul   v11.4s,  v24.4s, v8.4s        \n"                      \
  "fmul   v13.4s,  v25.4s, v8.4s        \n"                      \
  "fmul   v15.4s,  v26.4s, v8.4s        \n"                      \
  "fmax   v10.4s, v10.4s, v6.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v6.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v6.4s       \n"                        \
  "fmin   v10.4s, v10.4s, v9.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v9.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v9.4s       \n"                        \
  "fmul   v24.4s, v11.4s, v10.4s        \n"                      \
  "fmul   v25.4s, v13.4s, v12.4s        \n"                      \
  "fmul   v26.4s, v15.4s, v14.4s        \n"                      \
  "fadd   v10.4s,  v27.4s, v7.4s        \n"                      \
  "fadd   v12.4s,  v28.4s, v7.4s        \n"                      \
  "fadd   v14.4s,  v29.4s, v7.4s        \n"                      \
  "fmul   v11.4s,  v27.4s, v8.4s        \n"                      \
  "fmul   v13.4s,  v28.4s, v8.4s        \n"                      \
  "fmul   v15.4s,  v29.4s, v8.4s        \n"                      \
  "fmax   v10.4s, v10.4s, v6.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v6.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v6.4s       \n"                        \
  "fmin   v10.4s, v10.4s, v9.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v9.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v9.4s       \n"                        \
  "fmul   v27.4s, v11.4s, v10.4s        \n"                      \
  "fmul   v28.4s, v13.4s, v12.4s        \n"                      \
  "fmul   v29.4s, v15.4s, v14.4s        \n"                      \
  "fadd   v10.4s,  v30.4s, v7.4s        \n"                      \
  "fadd   v12.4s,  v0.4s, v7.4s        \n"                       \
  "fadd   v14.4s,  v1.4s, v7.4s        \n"                       \
  "fmul   v11.4s,  v30.4s, v8.4s        \n"                      \
  "fmul   v13.4s,  v0.4s, v8.4s        \n"                       \
  "fmul   v15.4s,  v1.4s, v8.4s        \n"                       \
  "fmax   v10.4s, v10.4s, v6.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v6.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v6.4s       \n"                        \
  "fmin   v10.4s, v10.4s, v9.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v9.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v9.4s       \n"                        \
  "fmul   v30.4s, v11.4s, v10.4s        \n"                      \
  "fmul   v0.4s, v13.4s, v12.4s        \n"                       \
  "fmul   v1.4s, v15.4s, v14.4s        \n"                       \
  "fadd   v10.4s, v2.4s, v7.4s        \n"                        \
  "fadd   v12.4s, v3.4s, v7.4s        \n"                        \
  "fadd   v14.4s, v4.4s, v7.4s        \n"                        \
  "fmul   v11.4s, v2.4s, v8.4s        \n"                        \
  "fmul   v13.4s, v3.4s, v8.4s        \n"                        \
  "fmul   v15.4s, v4.4s, v8.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v6.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v6.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v6.4s       \n"                        \
  "fmin   v10.4s, v10.4s, v9.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v9.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v9.4s       \n"                        \
  "fmul   v2.4s, v11.4s, v10.4s        \n"                       \
  "fmul   v3.4s, v13.4s, v12.4s        \n"                       \
  "fmul   v4.4s, v15.4s, v14.4s        \n"                       \
  "fadd   v10.4s, v5.4s, v7.4s        \n"                        \
  "fmul   v11.4s, v5.4s, v8.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v6.4s       \n"                        \
  "fmin   v10.4s, v10.4s, v9.4s       \n"                        \
  "fmul   v5.4s, v11.4s, v10.4s        \n"                       \
  "9:\n"

#define SPARSE_INT8_F32_W48_V8_HARD_SWISH                        \
  /* do relu */                                                  \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v4.4s,  v20.4s, v1.4s        \n"                       \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v22.4s, v1.4s        \n"                       \
  "fadd   v10.4s,  v23.4s, v1.4s        \n"                      \
  "fadd   v12.4s,  v24.4s, v1.4s        \n"                      \
  "fadd   v14.4s,  v25.4s, v1.4s        \n"                      \
  "fadd   v16.4s,  v26.4s, v1.4s        \n"                      \
  "fadd   v18.4s,  v27.4s, v1.4s        \n"                      \
  "fmul   v5.4s,   v20.4s, v2.4s        \n"                      \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v22.4s, v2.4s        \n"                      \
  "fmul   v11.4s,  v23.4s, v2.4s        \n"                      \
  "fmul   v13.4s,  v24.4s, v2.4s        \n"                      \
  "fmul   v15.4s,  v25.4s, v2.4s        \n"                      \
  "fmul   v17.4s,  v26.4s, v2.4s        \n"                      \
  "fmul   v19.4s,  v27.4s, v2.4s        \n"                      \
  "fmax   v4.4s,  v4.4s, v0.4s        \n"                        \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v0.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v0.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v0.4s       \n"                        \
  "fmax   v16.4s, v16.4s, v0.4s       \n"                        \
  "fmax   v18.4s, v18.4s, v0.4s       \n"                        \
  "fmin   v4.4s,  v4.4s, v3.4s        \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmin   v10.4s, v10.4s, v3.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v3.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v3.4s       \n"                        \
  "fmin   v16.4s, v16.4s, v3.4s       \n"                        \
  "fmin   v18.4s, v18.4s, v3.4s       \n"                        \
  "fmul   v20.4s,  v5.4s,  v4.4s        \n"                      \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v22.4s,  v9.4s,  v8.4s        \n"                      \
  "fmul   v23.4s,  v11.4s, v10.4s        \n"                     \
  "fmul   v24.4s,  v13.4s, v12.4s        \n"                     \
  "fmul   v25.4s,  v15.4s, v14.4s        \n"                     \
  "fmul   v26.4s,  v17.4s, v16.4s        \n"                     \
  "fmul   v27.4s,  v19.4s, v18.4s        \n"                     \
  "fadd   v4.4s,  v28.4s, v1.4s        \n"                       \
  "fadd   v6.4s,  v29.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v30.4s, v1.4s        \n"                       \
  "fadd   v10.4s,  v31.4s, v1.4s        \n"                      \
  "fmul   v5.4s,   v28.4s, v2.4s        \n"                      \
  "fmul   v7.4s,   v29.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v30.4s, v2.4s        \n"                      \
  "fmul   v11.4s,  v31.4s, v2.4s        \n"                      \
  "fmax   v4.4s,  v4.4s, v0.4s        \n"                        \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v0.4s       \n"                        \
  "fmin   v4.4s,  v4.4s, v3.4s        \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmin   v10.4s, v10.4s, v3.4s       \n"                        \
  "fmul   v28.4s,  v5.4s,  v4.4s        \n"                      \
  "fmul   v29.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v30.4s,  v9.4s,  v8.4s        \n"                      \
  "fmul   v31.4s,  v11.4s, v10.4s        \n"                     \
  "9:\n"

#define SPARSE_INT8_F32_W32_V8_HARD_SWISH                        \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v22.4s, v1.4s        \n"                       \
  "fadd   v10.4s,  v23.4s, v1.4s        \n"                      \
  "fadd   v12.4s,  v24.4s, v1.4s        \n"                      \
  "fadd   v14.4s,  v25.4s, v1.4s        \n"                      \
  "fadd   v16.4s,  v26.4s, v1.4s        \n"                      \
  "fadd   v18.4s,  v27.4s, v1.4s        \n"                      \
  "fadd   v30.4s,  v28.4s, v1.4s        \n"                      \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v22.4s, v2.4s        \n"                      \
  "fmul   v11.4s,  v23.4s, v2.4s        \n"                      \
  "fmul   v13.4s,  v24.4s, v2.4s        \n"                      \
  "fmul   v15.4s,  v25.4s, v2.4s        \n"                      \
  "fmul   v17.4s,  v26.4s, v2.4s        \n"                      \
  "fmul   v19.4s,  v27.4s, v2.4s        \n"                      \
  "fmul   v31.4s,  v28.4s, v2.4s        \n"                      \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v0.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v0.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v0.4s       \n"                        \
  "fmax   v16.4s, v16.4s, v0.4s       \n"                        \
  "fmax   v18.4s, v18.4s, v0.4s       \n"                        \
  "fmax   v30.4s, v30.4s, v0.4s       \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmin   v10.4s, v10.4s, v3.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v3.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v3.4s       \n"                        \
  "fmin   v16.4s, v16.4s, v3.4s       \n"                        \
  "fmin   v18.4s, v18.4s, v3.4s       \n"                        \
  "fmin   v30.4s, v30.4s, v3.4s       \n"                        \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v22.4s,  v9.4s,  v8.4s        \n"                      \
  "fmul   v23.4s,  v11.4s, v10.4s        \n"                     \
  "fmul   v24.4s,  v13.4s, v12.4s        \n"                     \
  "fmul   v25.4s,  v15.4s, v14.4s        \n"                     \
  "fmul   v26.4s,  v17.4s, v16.4s        \n"                     \
  "fmul   v27.4s,  v19.4s, v18.4s        \n"                     \
  "fmul   v28.4s,  v31.4s, v30.4s        \n"                     \
  "9:\n"

#define SPARSE_INT8_F32_W16_V8_HARD_SWISH                        \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v22.4s, v1.4s        \n"                       \
  "fadd   v10.4s,  v23.4s, v1.4s        \n"                      \
  "fadd   v12.4s,  v24.4s, v1.4s        \n"                      \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v22.4s, v2.4s        \n"                      \
  "fmul   v11.4s,  v23.4s, v2.4s        \n"                      \
  "fmul   v13.4s,  v24.4s, v2.4s        \n"                      \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v0.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v0.4s       \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmin   v10.4s, v10.4s, v3.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v3.4s       \n"                        \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v22.4s,  v9.4s,  v8.4s        \n"                      \
  "fmul   v23.4s,  v11.4s, v10.4s        \n"                     \
  "fmul   v24.4s,  v13.4s, v12.4s        \n"                     \
  "9:\n"

#define SPARSE_INT8_F32_W8_V8_HARD_SWISH                         \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v22.4s, v1.4s        \n"                       \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v22.4s, v2.4s        \n"                      \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v22.4s,  v9.4s,  v8.4s        \n"                      \
  "9:\n"

#define SPARSE_INT8_F32_W4_V8_HARD_SWISH                         \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "9:\n"

#define SPARSE_INT8_F32_W1_V8_HARD_SWISH                         \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v9.4s,    #0                \n"    /* for hardswish */ \
  "ldr    s1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    s2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    s3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   s6,  s0,  s1        \n"                                \
  "fmul   s7,  s0,  s2        \n"                                \
  "fmax   s6,  s6,  s9        \n"                                \
  "fmin   s6,  s6,  s3        \n"                                \
  "fmul   s0,  s7,  s6        \n"                                \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx64, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx64, and the required data is
 * MxKxKx64.
 */
#define SPARSE_INT8_F32_W64_V8_OUT      \
  SPARSE_INT8_F32_W64_V8_KERNEL         \
  SPARSE_INT8_F32_W64_V8_RELU           \
  SPARSE_INT8_F32_W64_V8_RELU6          \
  SPARSE_INT8_F32_W64_V8_LEAKY_RELU     \
  SPARSE_INT8_F32_W64_V8_HARD_SWISH     \
  /* store result */                    \
  "stp   q21, q22,  [%[c_ptr]]\n"       \
  "stp   q23, q24,  [%[c_ptr], #32]\n"  \
  "stp   q25, q26,  [%[c_ptr], #64]\n"  \
  "stp   q27, q28,  [%[c_ptr], #96]\n"  \
  "stp   q29, q30,  [%[c_ptr], #128]\n" \
  "stp   q0,  q1,   [%[c_ptr], #160]\n" \
  "stp   q2,  q3,   [%[c_ptr], #192]\n" \
  "stp   q4,  q5,   [%[c_ptr], #224]\n"

/**
 * The data block size for sparse matrix calculation is Mx48, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx48, and the required data is
 * MxKxKx48.
 */
#define SPARSE_INT8_F32_W48_V8_OUT      \
  SPARSE_INT8_F32_W48_V8_KERNEL         \
  SPARSE_INT8_F32_W48_V8_RELU           \
  SPARSE_INT8_F32_W48_V8_RELU6          \
  SPARSE_INT8_F32_W48_V8_LEAKY_RELU     \
  SPARSE_INT8_F32_W48_V8_HARD_SWISH     \
  /* store result */                    \
  "stp   q20, q21,  [%[c_ptr]]\n"       \
  "stp   q22, q23,  [%[c_ptr], #32]\n"  \
  "stp   q24, q25,  [%[c_ptr], #64]\n"  \
  "stp   q26, q27,  [%[c_ptr], #96]\n"  \
  "stp   q28, q29,  [%[c_ptr], #128]\n" \
  "stp   q30, q31,  [%[c_ptr], #160]\n"

/**
 * The data block size for sparse matrix calculation is Mx32, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx32, and the required data is
 * MxKxKx32.
 */
#define SPARSE_INT8_F32_W32_V8_OUT     \
  SPARSE_INT8_F32_W32_V8_KERNEL        \
  SPARSE_INT8_F32_W32_V8_RELU          \
  SPARSE_INT8_F32_W32_V8_RELU6         \
  SPARSE_INT8_F32_W32_V8_LEAKY_RELU    \
  SPARSE_INT8_F32_W32_V8_HARD_SWISH    \
  /* store result */                   \
  "stp   q21, q22,  [%[c_ptr]]\n"      \
  "stp   q23, q24,  [%[c_ptr], #32]\n" \
  "stp   q25, q26,  [%[c_ptr], #64]\n" \
  "stp   q27, q28,  [%[c_ptr], #96]\n"

/**
 * The data block size for sparse matrix calculation is Mx16, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx16, and the required data is
 * MxKxKx16.
 */
#define SPARSE_INT8_F32_W16_V8_OUT  \
  SPARSE_INT8_F32_W16_V8_KERNEL     \
  SPARSE_INT8_F32_W16_V8_RELU       \
  SPARSE_INT8_F32_W16_V8_RELU6      \
  SPARSE_INT8_F32_W16_V8_LEAKY_RELU \
  SPARSE_INT8_F32_W16_V8_HARD_SWISH \
  /* store result */                \
  "stp   q21, q22,  [%[c_ptr]]\n"   \
  "stp   q23, q24,  [%[c_ptr], #32]\n"

/**
 * The data block size for sparse matrix calculation is Mx8, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx8, and the required data is
 * MxKxKx8.
 */
#define SPARSE_INT8_F32_W8_V8_OUT  \
  SPARSE_INT8_F32_W8_V8_KERNEL     \
  SPARSE_INT8_F32_W8_V8_RELU       \
  SPARSE_INT8_F32_W8_V8_RELU6      \
  SPARSE_INT8_F32_W8_V8_LEAKY_RELU \
  SPARSE_INT8_F32_W8_V8_HARD_SWISH \
  /* store result */               \
  "stp   q21, q22,  [%[c_ptr]]\n"

/**
 * The data block size for sparse matrix calculation is Mx4, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx4, and the required data is
 * MxKxKx4.
 */
#define SPARSE_INT8_F32_W4_V8_OUT  \
  SPARSE_INT8_F32_W4_V8_KERNEL     \
  SPARSE_INT8_F32_W4_V8_RELU       \
  SPARSE_INT8_F32_W4_V8_RELU6      \
  SPARSE_INT8_F32_W4_V8_LEAKY_RELU \
  SPARSE_INT8_F32_W4_V8_HARD_SWISH \
  /* store result */               \
  "str   q21,  [%[c_ptr]]\n"

#define SPARSE_INT8_F32_W1_V8_OUT  \
  SPARSE_INT8_F32_W1_V8_KERNEL     \
  SPARSE_INT8_F32_W1_V8_RELU       \
  SPARSE_INT8_F32_W1_V8_RELU6      \
  SPARSE_INT8_F32_W1_V8_LEAKY_RELU \
  SPARSE_INT8_F32_W1_V8_HARD_SWISH \
  "str   s0,  [%[c_ptr]]\n"

#define PARAM1_INT8_F32                                               \
  [a_ptr] "+r"(cur_w), [b_ptr] "+r"(cur_b), [c_ptr] "+r"(cur_output), \
      [k] "+r"(nnz), [widx_dmap] "+r"(dmap)

#define PARAM2_INT8_F32                                                \
  [vscale] "r"(vsclae), [vbias] "r"(vbias), [vflag_act] "r"(flag_act), \
      [valpha] "r"(alpha), [hs_param] "r"(hs_param)

#define ASM_PARAM_INT8_F32                                                  \
  "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", \
      "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v21", "v22", \
      "v23", "v24", "v25", "v26", "v27", "v28", "v30", "v31", "w0", "w1",   \
      "x1", "cc", "memory"

#define INIT_PTR_1x1_SPMM_INT8_F32(i)                                        \
  float* cur_output =                                                        \
      reinterpret_cast<float*>((uintptr_t)output + output_stride * i);       \
  const int8_t* cur_w = A;                                                   \
  uint32_t nnz = nidx_nnzmap[i];                                             \
  const int8_t* cur_b = B;                                                   \
  const int32_t* dmap = widx_dmap;                                           \
  if (i != 0) {                                                              \
    cur_w = A + nidx_nnzmap[i - 1];                                          \
    nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];                               \
    cur_b +=                                                                 \
        ((nidx_nnzmap[i - 1] == 0) ? 0 : widx_dmap[nidx_nnzmap[i - 1] - 1]); \
    dmap = widx_dmap + nidx_nnzmap[i - 1];                                   \
  }                                                                          \
  float vsclae = scale[i];                                                   \
  float vbias = (bias != nullptr) ? bias[i] : 0.0;

#define REDEFINE_PTR_SPMM_INT8_F32 \
  uint32_t nnz = nnz0;             \
  const int32_t* dmap = dmap0;     \
  const int8_t* cur_w = cur_w0;    \
  const int8_t* cur_b = cur_b0;    \
  float* cur_output = cur_output0;

#define MAX_CACHE_SIZE (1 << 17)

/**
 * \brief Sparse calculation implementation of 1x1 convolution, the input-output
 * type is int8-f32.
 * Sparse matrix multiplication is calculated in blocks, the block size is Mx48,
 * that is,
 * the parameter matrix is MxK, and the activation matrix is Kx48; when N is
 * less than 48,
 * it is calculated in blocks of Mx32, Mx16, Mx8, and Mx4 in turn;
 * @param A sparse weight data
 * @param B dense input data
 * @param widx_dmap An array of int32_t values storing scaled [by sizeof(input
 * element)] difference
 * between input channels corresponding to successive non-zero element
 * @param nidx_nnzmap the number of non-zero kernel elements per each output
 * channel
 * @param bias
 * @param output
 * @param M
 * @param N
 * @param K
 * @param param
 * @param ctx
 */
static void sparse_conv_int8_f32_mxn_pipelined(
    const int8_t* A,
    const int8_t* B,
    const int32_t* widx_dmap,
    const uint32_t* nidx_nnzmap,
    const float* bias,
    const float* scale,
    float* output,
    int M,
    int K,
    int N,
    const operators::SparseConvParam& param,
    ARMContext* ctx) {
  auto act_param = param.activation_param;
  auto act_type = act_param.active_type;
  volatile float alpha = 0.f;
  float hs_param[12] = {0.f};
  int flag_act = 0x00;  // relu: 1, relu6: 2, leakey: 3
  if (act_param.has_active) {
    if (act_type == lite_api::ActivationType::kRelu) {
      flag_act = 0x01;
    } else if (act_type == lite_api::ActivationType::kRelu6) {
      flag_act = 0x02;
      alpha = act_param.Relu_clipped_coef;
    } else if (act_type == lite_api::ActivationType::kLeakyRelu) {
      flag_act = 0x03;
      alpha = act_param.Leaky_relu_alpha;
    } else if (act_type == lite_api::ActivationType::kHardSwish) {
      flag_act = 0x04;
      for (int i = 0; i < 4; i++) {
        hs_param[i] = act_param.hard_swish_offset;
        hs_param[i + 4] = 1.0 / act_param.hard_swish_scale;
        hs_param[i + 8] = act_param.hard_swish_threshold;
      }
    }
  }

  int flag_bias = (bias != nullptr) ? 1 : 0;
  size_t mc = N * sizeof(int8_t);
  size_t nc = M;
  size_t output_stride = N * sizeof(float);
  float vmax[4] = {-127.0, -127.0, -127.0, -127.0};

  while
    SPARSE_LIKELY(mc >= 64 * sizeof(int8_t)) {
      LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
        INIT_PTR_1x1_SPMM_INT8_F32(i)
            // clang-format off
          asm volatile(SPARSE_INT8_F32_W64_V8_OUT
            : PARAM1_INT8_F32
            : PARAM2_INT8_F32
            : ASM_PARAM_INT8_F32);
        // clang-format on
      }
      LITE_PARALLEL_COMMON_END();
      output = reinterpret_cast<float*>((uintptr_t)output + 64 * sizeof(float));
      B += 64;
      mc -= 64 * sizeof(int8_t);
    }

  if
    SPARSE_UNLIKELY(mc != 0) {
      if (mc >= (48 * sizeof(int8_t))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          INIT_PTR_1x1_SPMM_INT8_F32(i)
              // clang-format off
            asm volatile(SPARSE_INT8_F32_W48_V8_OUT
              : PARAM1_INT8_F32
              : PARAM2_INT8_F32
              : ASM_PARAM_INT8_F32);
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 48 * sizeof(float));
        B += 48;
        mc -= 48 * sizeof(int8_t);
      }

      if (mc >= 32) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          INIT_PTR_1x1_SPMM_INT8_F32(i)
              // clang-format off
            asm volatile(SPARSE_INT8_F32_W32_V8_OUT
              : PARAM1_INT8_F32
              : PARAM2_INT8_F32
              : ASM_PARAM_INT8_F32);
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 32 * sizeof(float));
        B += 32;
        mc -= 32 * sizeof(int8_t);
      }

      if (mc >= 16) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          INIT_PTR_1x1_SPMM_INT8_F32(i)
              // clang-format off
            asm volatile(SPARSE_INT8_F32_W16_V8_OUT
              : PARAM1_INT8_F32
              : PARAM2_INT8_F32
              : ASM_PARAM_INT8_F32);
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 16 * sizeof(float));
        B += 16;
        mc -= 16 * sizeof(int8_t);
      }

      if (mc >= 8) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          INIT_PTR_1x1_SPMM_INT8_F32(i)
              // clang-format off
            asm volatile(SPARSE_INT8_F32_W8_V8_OUT
              : PARAM1_INT8_F32
              : PARAM2_INT8_F32
              : ASM_PARAM_INT8_F32);
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 8 * sizeof(float));
        B += 8;
        mc -= 8;
      }

      if ((mc >= 4)) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          INIT_PTR_1x1_SPMM_INT8_F32(i)
              // clang-format off
            asm volatile(SPARSE_INT8_F32_W4_V8_OUT
              : PARAM1_INT8_F32
              : PARAM2_INT8_F32
              : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v11", "v16", "v21", 
              "w1", "w2", "w3", "w4", "w5", "x1", "cc", "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 4 * sizeof(float));
        B += 4;
        mc -= 4 * sizeof(int8_t);
      }

      int m = mc;
      for (; m > 0; m--) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          INIT_PTR_1x1_SPMM_INT8_F32(i) uint32_t pair_num = nnz / 8;
          uint32_t lave_num = nnz % 8;
          // clang-format off
            asm volatile(SPARSE_INT8_F32_W1_V8_OUT
            : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap)
            : [vscale] "r"(vsclae),
                [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha),
                [vmax] "r"(vmax),
                [hs_param] "r"(hs_param)
            : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "x1", "cc", "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output = reinterpret_cast<float*>((uintptr_t)output + sizeof(float));
        B += 1;
        mc -= 1;
      }
    }
}
static void sparse_conv_int8_f32_nxm_pipelined(
    const int8_t* A,
    const int8_t* B,
    const int32_t* widx_dmap,
    const uint32_t* nidx_nnzmap,
    const float* bias,
    const float* scale,
    float* output,
    int M,
    int K,
    int N,
    const operators::SparseConvParam& param,
    ARMContext* ctx) {
  auto act_param = param.activation_param;
  auto act_type = act_param.active_type;
  volatile float alpha = 0.f;
  float hs_param[12] = {0.f};
  int flag_act = 0x00;  // relu: 1, relu6: 2, leakey: 3
  if (act_param.has_active) {
    if (act_type == lite_api::ActivationType::kRelu) {
      flag_act = 0x01;
    } else if (act_type == lite_api::ActivationType::kRelu6) {
      flag_act = 0x02;
      alpha = act_param.Relu_clipped_coef;
    } else if (act_type == lite_api::ActivationType::kLeakyRelu) {
      flag_act = 0x03;
      alpha = act_param.Leaky_relu_alpha;
    } else if (act_type == lite_api::ActivationType::kHardSwish) {
      flag_act = 0x04;
      for (int i = 0; i < 4; i++) {
        hs_param[i] = act_param.hard_swish_offset;
        hs_param[i + 4] = 1.0 / act_param.hard_swish_scale;
        hs_param[i + 8] = act_param.hard_swish_threshold;
      }
    }
  }

  int flag_bias = (bias != nullptr) ? 1 : 0;
  size_t mc = N * sizeof(int8_t);
  size_t nc = M;
  size_t output_stride = N * sizeof(float);
  float vmax[4] = {-127.0, -127.0, -127.0, -127.0};

  LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
    float* cur_output0 =
        reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
    const int8_t* cur_w0 = A;
    volatile uint32_t nnz0 = nidx_nnzmap[i];
    const int8_t* cur_b0 = B;
    const int32_t* dmap0 = widx_dmap;
    if (i != 0) {
      int nidx = nidx_nnzmap[i - 1];
      cur_w0 = A + nidx;
      nnz0 = nidx_nnzmap[i] - nidx;
      cur_b0 += ((nidx == 0) ? 0 : widx_dmap[nidx - 1]);
      dmap0 = widx_dmap + nidx;
    }
    volatile float vsclae = scale[i];
    volatile float vbias = (bias != nullptr) ? bias[i] : 0.0;

    int mc0 = mc;
    while
      SPARSE_LIKELY(mc0 >= 64 * sizeof(int8_t)) {
        REDEFINE_PTR_SPMM_INT8_F32
        // clang-format off
        asm volatile(SPARSE_INT8_F32_W64_V8_OUT
          : PARAM1_INT8_F32
          : PARAM2_INT8_F32
          : ASM_PARAM_INT8_F32);
        // clang-format on
        cur_output0 += 64;
        cur_b0 += 64;
        mc0 -= 64 * sizeof(int8_t);
      }

    if
      SPARSE_UNLIKELY(mc != 0) {
        if (mc0 >= 48) {
          REDEFINE_PTR_SPMM_INT8_F32
          // clang-format off
        asm volatile(SPARSE_INT8_F32_W48_V8_OUT
          : PARAM1_INT8_F32
          : PARAM2_INT8_F32
          : ASM_PARAM_INT8_F32);
          // clang-format on
          mc0 -= 48;
          cur_b0 += 48;
          cur_output0 += 48;
        }

        if (mc0 >= 32) {
          REDEFINE_PTR_SPMM_INT8_F32
          // clang-format off
        asm volatile(SPARSE_INT8_F32_W32_V8_OUT
          : PARAM1_INT8_F32
          : PARAM2_INT8_F32
          : ASM_PARAM_INT8_F32);
          // clang-format on
          mc0 -= 32;
          cur_b0 += 32;
          cur_output0 += 32;
        }

        if (mc0 >= 16) {
          REDEFINE_PTR_SPMM_INT8_F32
          // clang-format off
        asm volatile(SPARSE_INT8_F32_W16_V8_OUT
          : PARAM1_INT8_F32
          : PARAM2_INT8_F32
          : ASM_PARAM_INT8_F32);
          // clang-format on
          mc0 -= 16;
          cur_b0 += 16;
          cur_output0 += 16;
        }

        if (mc0 >= 8) {
          REDEFINE_PTR_SPMM_INT8_F32
          // clang-format off
        asm volatile(SPARSE_INT8_F32_W8_V8_OUT
          : PARAM1_INT8_F32
          : PARAM2_INT8_F32
          : ASM_PARAM_INT8_F32);
          // clang-format on
          mc0 -= 8;
          cur_b0 += 8;
          cur_output0 += 8;
        }

        if ((mc0 >= 4)) {
          REDEFINE_PTR_SPMM_INT8_F32
          // clang-format off
        asm volatile(SPARSE_INT8_F32_W4_V8_OUT
          : PARAM1_INT8_F32
          : PARAM2_INT8_F32
          : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v11", "v16", "v21", 
          "w1", "w2", "w3", "w4", "w5", "x1", "cc", "memory");
          // clang-format on
          mc0 -= 4;
          cur_b0 += 4;
          cur_output0 += 4;
        }

        int m = mc0;
        for (; m > 0; m--) {
          REDEFINE_PTR_SPMM_INT8_F32

          uint32_t pair_num = nnz / 8;
          uint32_t lave_num = nnz % 8;
          // clang-format off
        asm volatile(SPARSE_INT8_F32_W1_V8_OUT
        : [a_ptr] "+r"(cur_w),
            [b_ptr] "+r"(cur_b),
            [c_ptr] "+r"(cur_output),
            [k] "+r"(nnz),
            [n] "+r"(pair_num),
            [m] "+r"(lave_num),
            [widx_dmap] "+r"(dmap)
        : [vscale] "r"(vsclae),
            [vbias] "r"(vbias),
            [vflag_act] "r"(flag_act),
            [valpha] "r"(alpha),
            [vmax] "r"(vmax),
            [hs_param] "r"(hs_param)
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "x1", "cc", "memory");
          // clang-format on
          mc0 -= 1;
          cur_b0 += 1;
          cur_output0 += 1;
        }
      }
  }
  LITE_PARALLEL_COMMON_END();
}
void sparse_conv_int8_fp32_pipelined(const int8_t* A,
                                     const int8_t* B,
                                     const int32_t* widx_dmap,
                                     const uint32_t* nidx_nnzmap,
                                     const float* bias,
                                     const float* scale,
                                     float* output,
                                     int M,
                                     int K,
                                     int N,
                                     const operators::SparseConvParam& param,
                                     ARMContext* ctx) {
  if (M * N > MAX_CACHE_SIZE) {
    sparse_conv_int8_f32_mxn_pipelined(
        A, B, widx_dmap, nidx_nnzmap, bias, scale, output, M, K, N, param, ctx);

  } else {
    sparse_conv_int8_f32_nxm_pipelined(
        A, B, widx_dmap, nidx_nnzmap, bias, scale, output, M, K, N, param, ctx);
  }
}

#define SPARSE_INT8_INT8_W64_V8_KERNEL     \
  "eor v16.16b, v0.16b, v0.16b\n"          \
  "eor v17.16b, v1.16b, v1.16b\n"          \
  "eor v18.16b, v2.16b, v2.16b\n"          \
  "eor v19.16b, v3.16b, v3.16b\n"          \
  "eor v20.16b, v4.16b, v4.16b\n"          \
  "prfm  pldl1keep, [%[a_ptr], #32]\n"     \
  "eor v21.16b, v5.16b, v5.16b\n"          \
  "eor v22.16b, v6.16b, v6.16b\n"          \
  "prfm  pldl1keep, [%[widx_dmap], #32]\n" \
  "eor v23.16b, v7.16b, v7.16b\n"          \
  "eor v24.16b, v8.16b, v8.16b\n"          \
  "prfm  pldl1keep, [%[b_ptr], #64]\n"     \
  "eor v25.16b, v9.16b, v9.16b\n"          \
  "eor v26.16b, v10.16b, v10.16b\n"        \
  "eor v27.16b, v11.16b, v11.16b\n"        \
  "eor v28.16b, v12.16b, v12.16b\n"        \
  "eor v29.16b, v13.16b, v13.16b\n"        \
  "eor v30.16b, v14.16b, v14.16b\n"        \
  "eor v31.16b, v15.16b, v15.16b\n"        \
  "cbz    %w[k],    1f\n"                  \
  "0:\n"                                   \
  "ld1r  {v0.16b}, [%[a_ptr]], #1\n"       \
  "ldp   q1, q2, [%[b_ptr]]\n"             \
  "ldp   q3, q4, [%[b_ptr], #32]\n"        \
  "subs    %w[k],   %w[k],   #1\n"         \
  "smull   v5.8h,   v0.8b,   v1.8b\n"      \
  "smull2  v6.8h,   v0.16b,  v1.16b\n"     \
  "smull   v7.8h,   v0.8b,   v2.8b\n"      \
  "smull2  v8.8h,   v0.16b,  v2.16b\n"     \
  "smull   v9.8h,   v0.8b,   v3.8b\n"      \
  "smull2  v10.8h,  v0.16b,  v3.16b\n"     \
  "smull   v11.8h,  v0.8b,   v4.8b\n"      \
  "smull2  v12.8h,  v0.16b,  v4.16b\n"     \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n"    \
  "saddw   v16.4s,  v16.4s,  v5.4h\n"      \
  "saddw2  v17.4s,  v17.4s,  v5.8h\n"      \
  "saddw   v18.4s,  v18.4s,  v6.4h\n"      \
  "saddw2  v19.4s,  v19.4s,  v6.8h\n"      \
  "saddw   v20.4s,  v20.4s,  v7.4h\n"      \
  "saddw2  v21.4s,  v21.4s,  v7.8h\n"      \
  "add   %[b_ptr],  %[b_ptr], x1\n"        \
  "saddw   v22.4s,  v22.4s,  v8.4h\n"      \
  "saddw2  v23.4s,  v23.4s,  v8.8h\n"      \
  "saddw   v24.4s,  v24.4s,  v9.4h\n"      \
  "saddw2  v25.4s,  v25.4s,  v9.8h\n"      \
  "prfm  pldl1keep, [%[b_ptr], #64]\n"     \
  "saddw   v26.4s,  v26.4s,  v10.4h\n"     \
  "saddw2  v27.4s,  v27.4s,  v10.8h\n"     \
  "saddw   v28.4s,  v28.4s,  v11.4h\n"     \
  "saddw2  v29.4s,  v29.4s,  v11.8h\n"     \
  "saddw   v30.4s,  v30.4s,  v12.4h\n"     \
  "saddw2  v31.4s,  v31.4s,  v12.8h\n"     \
  "bne     0b\n"                           \
  "1:\n"                                   \
  "scvtf   v0.4s,   v16.4s\n"              \
  "scvtf   v1.4s,   v17.4s\n"              \
  "scvtf   v2.4s,   v18.4s\n"              \
  "scvtf   v3.4s,   v19.4s\n"              \
  "scvtf   v4.4s,   v20.4s\n"              \
  "scvtf   v5.4s,   v21.4s\n"              \
  "scvtf   v6.4s,   v22.4s\n"              \
  "scvtf   v7.4s,   v23.4s\n" /* scale */  \
  "scvtf   v8.4s,   v24.4s\n"              \
  "scvtf   v9.4s,   v25.4s\n"              \
  "scvtf   v10.4s,  v26.4s\n"              \
  "scvtf   v11.4s,  v27.4s\n"              \
  "scvtf   v12.4s,  v28.4s\n"              \
  "scvtf   v13.4s,  v29.4s\n"              \
  "scvtf   v14.4s,  v30.4s\n"              \
  "scvtf   v15.4s,  v31.4s\n"              \
  "dup     v21.4s,  %w[vbias]\n"           \
  "dup     v22.4s,  v21.s[0]\n"            \
  "dup     v23.4s,  v21.s[0]\n"            \
  "dup     v24.4s,  v21.s[0]\n"            \
  "dup     v25.4s,  v21.s[0]\n"            \
  "dup     v31.4s,  %w[vscale]\n"          \
  "dup     v26.4s,  v21.s[0]\n"            \
  "dup     v27.4s,  v21.s[0]\n"            \
  "dup     v28.4s,  v21.s[0]\n"            \
  "dup     v29.4s,  v21.s[0]\n"            \
  "dup     v30.4s,  v21.s[0]\n"            \
  "fmla    v21.4s,  v0.4s,  v31.s[0]\n"    \
  "fmla    v22.4s,  v1.4s,  v31.s[0]\n"    \
  "fmla    v23.4s,  v2.4s,  v31.s[0]\n"    \
  "fmla    v24.4s,  v3.4s,  v31.s[0]\n"    \
  "fmla    v25.4s,  v4.4s,  v31.s[0]\n"    \
  "fmla    v26.4s,  v5.4s,  v31.s[0]\n"    \
  "fmla    v27.4s,  v6.4s,  v31.s[0]\n"    \
  "fmla    v28.4s,  v7.4s,  v31.s[0]\n"    \
  "fmla    v29.4s,  v8.4s,  v31.s[0]\n"    \
  "fmla    v30.4s,  v9.4s,  v31.s[0]\n"    \
  "dup     v0.4s,  %w[vbias]\n"            \
  "dup     v1.4s,  v0.s[0]\n"              \
  "dup     v2.4s,  v0.s[0]\n"              \
  "dup     v3.4s,  v0.s[0]\n"              \
  "dup     v4.4s,  v0.s[0]\n"              \
  "dup     v5.4s,  v0.s[0]\n"              \
  "fmla    v0.4s,  v10.4s,  v31.s[0]\n"    \
  "fmla    v1.4s,  v11.4s,  v31.s[0]\n"    \
  "fmla    v2.4s,  v12.4s,  v31.s[0]\n"    \
  "fmla    v3.4s,  v13.4s,  v31.s[0]\n"    \
  "fmla    v4.4s,  v14.4s,  v31.s[0]\n"    \
  "fmla    v5.4s,  v15.4s,  v31.s[0]\n"

#define SPARSE_INT8_INT8_W48_V8_KERNEL           \
  "eor v8.16b, v0.16b, v0.16b\n"                 \
  "eor v9.16b, v1.16b, v1.16b\n"                 \
  "eor v10.16b, v2.16b, v2.16b\n"                \
  "eor v11.16b, v3.16b, v3.16b\n"                \
  "eor v12.16b, v4.16b, v4.16b\n"                \
  "prfm  pldl1keep, [%[a_ptr], #32]\n"           \
  "eor v13.16b, v5.16b, v5.16b\n"                \
  "eor v14.16b, v6.16b, v6.16b\n"                \
  "prfm  pldl1keep, [%[widx_dmap], #32]\n"       \
  "eor v15.16b, v7.16b, v7.16b\n"                \
  "eor v16.16b, v0.16b, v0.16b\n"                \
  "prfm  pldl1keep, [%[b_ptr], #48]\n"           \
  "eor v17.16b, v1.16b, v1.16b\n"                \
  "eor v18.16b, v2.16b, v2.16b\n"                \
  "eor v19.16b, v3.16b, v3.16b\n"                \
  "dup     v20.4s,  %w[vbias]\n"                 \
  "dup     v21.4s,  v20.s[0]\n"                  \
  "dup     v22.4s,  v20.s[0]\n"                  \
  "dup     v23.4s,  v20.s[0]\n"                  \
  "dup     v24.4s,  v20.s[0]\n"                  \
  "dup     v25.4s,  v20.s[0]\n"                  \
  "dup     v26.4s,  v20.s[0]\n"                  \
  "dup     v27.4s,  v20.s[0]\n"                  \
  "dup     v28.4s,  v20.s[0]\n"                  \
  "dup     v29.4s,  v20.s[0]\n"                  \
  "dup     v30.4s,  v20.s[0]\n"                  \
  "dup     v31.4s,  v20.s[0]\n"                  \
  "cbz    %w[k],    1f\n"                        \
  "0:\n"                                         \
  "ld1r  {v0.16b}, [%[a_ptr]], #1\n"             \
  "ld1   {v1.16b, v2.16b, v3.16b}, [%[b_ptr]]\n" \
  "subs    %w[k],   %w[k],   #1\n"               \
  "ldrsw   x1, [%[widx_dmap]],   #4\n"           \
  "smull   v4.8h,   v0.8b,   v1.8b\n"            \
  "smull2  v5.8h,   v0.16b,  v1.16b\n"           \
  "add   %[b_ptr],  %[b_ptr], x1\n"              \
  "smull   v6.8h,   v0.8b,   v2.8b\n"            \
  "smull2  v7.8h,   v0.16b,  v2.16b\n"           \
  "saddw   v8.4s,  v8.4s,  v4.4h\n"              \
  "saddw2  v9.4s,  v9.4s,  v4.8h\n"              \
  "prfm  pldl1keep, [%[b_ptr], #48]\n"           \
  "saddw   v10.4s,  v10.4s,  v5.4h\n"            \
  "saddw2  v11.4s,  v11.4s,  v5.8h\n"            \
  "smull   v4.8h,   v0.8b,   v3.8b\n"            \
  "saddw   v12.4s,  v12.4s,  v6.4h\n"            \
  "saddw2  v13.4s,  v13.4s,  v6.8h\n"            \
  "saddw   v14.4s,  v14.4s,  v7.4h\n"            \
  "smull2  v5.8h,   v0.16b,  v3.16b\n"           \
  "saddw2  v15.4s,  v15.4s,  v7.8h\n"            \
  "saddw   v16.4s,  v16.4s,  v4.4h\n"            \
  "saddw2  v17.4s,  v17.4s,  v4.8h\n"            \
  "saddw   v18.4s,  v18.4s,  v5.4h\n"            \
  "saddw2  v19.4s,  v19.4s,  v5.8h\n"            \
  "bne     0b\n"                                 \
  "1:\n"                                         \
  "dup     v0.4s,  %w[vscale]\n"                 \
  "scvtf   v1.4s,  v8.4s\n"                      \
  "scvtf   v2.4s,  v9.4s\n"                      \
  "scvtf   v3.4s,  v10.4s\n"                     \
  "scvtf   v4.4s,  v11.4s\n"                     \
  "scvtf   v5.4s,  v12.4s\n"                     \
  "scvtf   v6.4s,  v13.4s\n"                     \
  "scvtf   v7.4s,  v14.4s\n" /* scale */         \
  "fmla    v20.4s,  v1.4s,  v0.s[0]\n"           \
  "fmla    v21.4s,  v2.4s,  v0.s[0]\n"           \
  "fmla    v22.4s,  v3.4s,  v0.s[0]\n"           \
  "scvtf   v1.4s,  v15.4s\n"                     \
  "scvtf   v2.4s,  v16.4s\n"                     \
  "scvtf   v3.4s,  v17.4s\n"                     \
  "fmla    v23.4s,  v4.4s,  v0.s[0]\n"           \
  "fmla    v24.4s,  v5.4s,  v0.s[0]\n"           \
  "fmla    v25.4s,  v6.4s,  v0.s[0]\n"           \
  "fmla    v26.4s,  v7.4s,  v0.s[0]\n"           \
  "scvtf   v4.4s,  v18.4s\n"                     \
  "scvtf   v5.4s,  v19.4s\n" /* scale */         \
  "fmla    v27.4s,  v1.4s,  v0.s[0]\n"           \
  "fmla    v28.4s,  v2.4s,  v0.s[0]\n"           \
  "fmla    v29.4s,  v3.4s,  v0.s[0]\n"           \
  "fmla    v30.4s,  v4.4s,  v0.s[0]\n"           \
  "fmla    v31.4s,  v5.4s,  v0.s[0]\n"

#define SPARSE_INT8_INT8_W32_V8_KERNEL     \
  "eor v11.16b, v0.16b, v0.16b\n"          \
  "eor v12.16b, v1.16b, v1.16b\n"          \
  "prfm  pldl1keep, [%[a_ptr], #32]\n"     \
  "eor v13.16b, v2.16b, v2.16b\n"          \
  "eor v14.16b, v3.16b, v3.16b\n"          \
  "prfm  pldl1keep, [%[widx_dmap], #32]\n" \
  "eor v15.16b, v4.16b, v4.16b\n"          \
  "eor v16.16b, v5.16b, v5.16b\n"          \
  "prfm  pldl1keep, [%[b_ptr], #32]\n"     \
  "eor v17.16b, v6.16b, v6.16b\n"          \
  "eor v18.16b, v7.16b, v7.16b\n"          \
  "dup     v21.4s,  %w[vbias]\n"           \
  "dup     v22.4s,  v21.s[0]\n"            \
  "dup     v23.4s,  v21.s[0]\n"            \
  "dup     v24.4s,  v21.s[0]\n"            \
  "dup     v25.4s,  v21.s[0]\n"            \
  "dup     v26.4s,  v21.s[0]\n"            \
  "dup     v27.4s,  v21.s[0]\n"            \
  "dup     v28.4s,  v21.s[0]\n"            \
  "cbz    %w[k],    1f\n"                  \
  "0:\n"                                   \
  "ld1r  {v0.16b}, [%[a_ptr]], #1\n"       \
  "ldrsw   x1, [%[widx_dmap]],   #4\n"     \
  "ld1   {v1.16b, v2.16b}, [%[b_ptr]]\n"   \
  "subs    %w[k],   %w[k],   #1\n"         \
  "smull   v3.8h,   v0.8b,   v1.8b\n"      \
  "add   %[b_ptr],  %[b_ptr], x1\n"        \
  "smull2  v4.8h,   v0.16b,  v1.16b\n"     \
  "smull   v5.8h,   v0.8b,   v2.8b\n"      \
  "smull2  v6.8h,   v0.16b,  v2.16b\n"     \
  "saddw   v11.4s,  v11.4s,  v3.4h\n"      \
  "saddw   v13.4s,  v13.4s,  v4.4h\n"      \
  "prfm  pldl1keep, [%[b_ptr], #64]\n"     \
  "saddw   v15.4s,  v15.4s,  v5.4h\n"      \
  "saddw   v17.4s,  v17.4s,  v6.4h\n"      \
  "saddw2  v12.4s,  v12.4s,  v3.8h\n"      \
  "saddw2  v14.4s,  v14.4s,  v4.8h\n"      \
  "saddw2  v16.4s,  v16.4s,  v5.8h\n"      \
  "saddw2  v18.4s,  v18.4s,  v6.8h\n"      \
  "bne     0b\n"                           \
  "1:\n"                                   \
  "scvtf   v3.4s,  v11.4s\n"               \
  "scvtf   v4.4s,  v12.4s\n"               \
  "scvtf   v5.4s,  v13.4s\n"               \
  "scvtf   v6.4s,  v14.4s\n"               \
  "scvtf   v7.4s,  v15.4s\n"               \
  "scvtf   v8.4s,  v16.4s\n"               \
  "scvtf   v9.4s,  v17.4s\n"               \
  "scvtf   v10.4s, v18.4s\n" /* scale */   \
  "dup     v31.4s,  %w[vscale]\n"          \
  "fmla    v21.4s,  v3.4s,  v31.s[0]\n"    \
  "fmla    v22.4s,  v4.4s,  v31.s[0]\n"    \
  "fmla    v23.4s,  v5.4s,  v31.s[0]\n"    \
  "fmla    v24.4s,  v6.4s,  v31.s[0]\n"    \
  "fmla    v25.4s,  v7.4s,  v31.s[0]\n"    \
  "fmla    v26.4s,  v8.4s,  v31.s[0]\n"    \
  "fmla    v27.4s,  v9.4s,  v31.s[0]\n"    \
  "fmla    v28.4s,  v10.4s, v31.s[0]\n"

#define SPARSE_INT8_INT8_W16_V8_KERNEL     \
  "eor v11.16b, v0.16b, v0.16b\n"          \
  "prfm  pldl1keep, [%[a_ptr], #16]\n"     \
  "eor v12.16b, v1.16b, v1.16b\n"          \
  "eor v13.16b, v2.16b, v2.16b\n"          \
  "prfm  pldl1keep, [%[b_ptr], #16]\n"     \
  "eor v14.16b, v3.16b, v3.16b\n"          \
  "dup     v21.4s,  %w[vbias]\n"           \
  "dup     v22.4s,  v21.s[0]\n"            \
  "prfm  pldl1keep, [%[widx_dmap], #16]\n" \
  "dup     v23.4s,  v21.s[0]\n"            \
  "dup     v24.4s,  v21.s[0]\n"            \
  "cbz    %w[k],    1f\n"                  \
  "0:\n"                                   \
  "ld1r  {v0.16b}, [%[a_ptr]], #1\n"       \
  "ldrsw   x1, [%[widx_dmap]],   #4\n"     \
  "ld1   {v1.16b}, [%[b_ptr]]\n"           \
  "subs    %w[k],   %w[k],   #1\n"         \
  "smull   v3.8h,   v0.8b,   v1.8b\n"      \
  "smull2  v4.8h,   v0.16b,  v1.16b\n"     \
  "add   %[b_ptr],  %[b_ptr], x1\n"        \
  "saddw   v11.4s,  v11.4s,  v3.4h\n"      \
  "saddw2  v12.4s,  v12.4s,  v3.8h\n"      \
  "prfm  pldl1keep, [%[b_ptr], #32]\n"     \
  "saddw   v13.4s,  v13.4s,  v4.4h\n"      \
  "saddw2  v14.4s,  v14.4s,  v4.8h\n"      \
  "bne     0b\n"                           \
  "1:\n"                                   \
  "scvtf   v5.4s,  v11.4s\n"               \
  "scvtf   v6.4s,  v12.4s\n"               \
  "scvtf   v7.4s,  v13.4s\n"               \
  "scvtf   v8.4s,  v14.4s\n" /* scale */   \
  "dup     v2.4s,  %w[vscale]\n"           \
  "fmla    v21.4s,  v5.4s,  v2.s[0]\n"     \
  "fmla    v22.4s,  v6.4s,  v2.s[0]\n"     \
  "fmla    v23.4s,  v7.4s,  v2.s[0]\n"     \
  "fmla    v24.4s,  v8.4s,  v2.s[0]\n"

#define SPARSE_INT8_INT8_W8_V8_KERNEL      \
  "eor v11.16b, v0.16b, v0.16b\n"          \
  "prfm  pldl1keep, [%[widx_dmap], #16]\n" \
  "eor v12.16b, v1.16b, v1.16b\n"          \
  "dup     v21.4s,  %w[vbias]\n"           \
  "prfm  pldl1keep, [%[a_ptr], #16]\n"     \
  "dup     v22.4s,  v21.s[0]\n"            \
  "prfm  pldl1keep, [%[b_ptr], #16]\n"     \
  "cbz    %w[k],    1f\n"                  \
  "0:\n"                                   \
  "ld1r  {v0.8b}, [%[a_ptr]], #1\n"        \
  "ldr   w1, [%[widx_dmap]],   #4\n"       \
  "sxtw  x1,  w1\n"                        \
  "ld1   {v1.8b}, [%[b_ptr]]\n"            \
  "add   %[b_ptr],  %[b_ptr], x1\n"        \
  "smull   v3.8h,   v0.8b,   v1.8b\n"      \
  "subs    %w[k],   %w[k],   #1\n"         \
  "saddw   v11.4s,  v11.4s,  v3.4h\n"      \
  "saddw2  v12.4s,  v12.4s,  v3.8h\n"      \
  "bne     0b\n"                           \
  "1:\n"                                   \
  "scvtf   v4.4s,  v11.4s\n"               \
  "scvtf   v5.4s,  v12.4s\n" /* scale */   \
  "dup     v2.4s,   %w[vscale]\n"          \
  "fmla    v21.4s,  v4.4s,  v2.s[0]\n"     \
  "fmla    v22.4s,  v5.4s,  v2.s[0]\n"

#define SPARSE_INT8_INT8_W4_V8_KERNEL      \
  "eor v11.16b, v0.16b, v0.16b\n"          \
  "prfm  pldl1keep, [%[widx_dmap], #16]\n" \
  "dup     v21.4s,  %w[vbias]\n"           \
  "prfm  pldl1keep, [%[a_ptr], #16]\n"     \
  "cbz    %w[k],    1f       \n"           \
  "cmp    %w[k],   #1        \n"           \
  "beq    4f                \n"            \
  "0:\n"                                   \
  "ld1r  {v0.8b}, [%[a_ptr]], #1\n"        \
  "sub    %w[k],   %w[k],   #1\n"          \
  "ldrsw   x1, [%[widx_dmap]],   #4\n"     \
  "ld1   {v1.8b}, [%[b_ptr]]\n"            \
  "add   %[b_ptr],  %[b_ptr], x1\n"        \
  "smull   v3.8h,   v0.8b,   v1.8b\n"      \
  "saddw   v11.4s,  v11.4s,  v3.4h\n"      \
  "cmp    %w[k],    #1          \n"        \
  "bne    0b                    \n"        \
  "4:\n"                                   \
  "ld1r  {v0.8b}, [%[a_ptr]], #1\n"        \
  "ldr   w1, [%[widx_dmap]],   #4\n"       \
  "sub    %w[k],   %w[k],   #1\n"          \
  "ldrsb   w2, [%[b_ptr]]\n"               \
  "ldrsb   w3, [%[b_ptr], #1]\n"           \
  "ldrsb   w4, [%[b_ptr], #2]\n"           \
  "ldrsb   w5, [%[b_ptr], #3]\n"           \
  "sxtw  x1,  w1\n"                        \
  "mov   v1.b[0], w2\n"                    \
  "mov   v1.b[1], w3\n"                    \
  "mov   v1.b[2], w4\n"                    \
  "mov   v1.b[3], w5\n"                    \
  "add   %[b_ptr],  %[b_ptr], x1\n"        \
  "smull   v3.8h,   v0.8b,   v1.8b\n"      \
  "saddw   v11.4s,  v11.4s,  v3.4h\n"      \
  "1:\n"                                   \
  "scvtf   v4.4s,  v11.4s\n" /* scale */   \
  "dup     v2.4s,   %w[vscale]\n"          \
  "fmla    v21.4s,  v4.4s,  v2.s[0]\n"

#define SPARSE_INT8_INT8_W1_V8_KERNEL   \
  "eor v3.16b, v0.16b, v0.16b\n"        \
  "dup     v0.4s,  %w[vbias]\n"         \
  "cbz    %w[n],    2f\n"               \
  "0:\n"                                \
  "ld1   {v2.8b}, [%[a_ptr]], #8\n"     \
  "ld1   {v1.b}[0], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[1], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[2], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[3], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[4], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[5], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[6], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "ld1   {v1.b}[7], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "smull   v4.8h,   v1.8b,   v2.8b\n"   \
  "subs    %w[n],   %w[n],   #1\n"      \
  "sadalp  v3.4s,  v4.8h    \n"         \
  "bne     0b\n"                        \
  "2:\n"                                \
  "cbz    %w[m],    1f\n"               \
  "eor v5.16b, v4.16b, v4.16b\n"        \
  "eor v6.16b, v4.16b, v4.16b\n"        \
  "3:\n"                                \
  "ld1   {v5.b}[0], [%[a_ptr]], #1\n"   \
  "ld1   {v6.b}[0], [%[b_ptr]]\n"       \
  "ldrsw   x1,  [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], x1\n"     \
  "smull   v4.8h,   v5.8b,   v6.8b\n"   \
  "saddw   v3.4s,  v3.4s,  v4.4h\n"     \
  "subs    %w[m],   %w[m],   #1\n"      \
  "bne     3b\n"                        \
  "1:\n"                                \
  "addv    s2,      v3.4s       \n"     \
  "scvtf   v1.4s,  v2.4s\n" /* scale */ \
  "dup     v2.4s,   %w[vscale]\n"       \
  "fmadd   s0, s1, s2, s0   \n"

#define SPARSE_INT8_INT8_W64_V8_RELU                  \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v6.4s, #0\n"               /* for relu */   \
  "fmax   v21.4s, v21.4s, v6.4s\n"   /* relu */       \
  "fmax   v22.4s, v22.4s, v6.4s\n"   /* relu */       \
  "fmax   v23.4s, v23.4s, v6.4s\n"   /* relu */       \
  "fmax   v24.4s, v24.4s, v6.4s\n"   /* relu */       \
  "fmax   v25.4s, v25.4s, v6.4s\n"   /* relu */       \
  "fmax   v26.4s, v26.4s, v6.4s\n"   /* relu */       \
  "fmax   v27.4s, v27.4s, v6.4s\n"   /* relu */       \
  "fmax   v28.4s, v28.4s, v6.4s\n"   /* relu */       \
  "fmax   v29.4s, v29.4s, v6.4s\n"   /* relu */       \
  "fmax   v30.4s, v30.4s, v6.4s\n"   /* relu */       \
  "fmax   v0.4s,  v0.4s,  v6.4s\n"   /* relu */       \
  "fmax   v1.4s,  v1.4s,  v6.4s\n"   /* relu */       \
  "fmax   v2.4s,  v2.4s,  v6.4s\n"   /* relu */       \
  "fmax   v3.4s,  v3.4s,  v6.4s\n"   /* relu */       \
  "fmax   v4.4s,  v4.4s,  v6.4s\n"   /* relu */       \
  "fmax   v5.4s,  v5.4s,  v6.4s\n"   /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W48_V8_RELU                  \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v0.4s, #0\n"               /* for relu */   \
  "fmax   v20.4s, v20.4s, v0.4s\n"   /* relu */       \
  "fmax   v21.4s, v21.4s, v0.4s\n"   /* relu */       \
  "fmax   v22.4s, v22.4s, v0.4s\n"   /* relu */       \
  "fmax   v23.4s, v23.4s, v0.4s\n"   /* relu */       \
  "fmax   v24.4s, v24.4s, v0.4s\n"   /* relu */       \
  "fmax   v25.4s, v25.4s, v0.4s\n"   /* relu */       \
  "fmax   v26.4s, v26.4s, v0.4s\n"   /* relu */       \
  "fmax   v27.4s, v27.4s, v0.4s\n"   /* relu */       \
  "fmax   v28.4s, v28.4s, v0.4s\n"   /* relu */       \
  "fmax   v29.4s, v29.4s, v0.4s\n"   /* relu */       \
  "fmax   v30.4s, v30.4s, v0.4s\n"   /* relu */       \
  "fmax   v31.4s, v31.4s, v0.4s\n"   /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W32_V8_RELU                  \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v30.4s, #0\n"              /* for relu */   \
  "fmax   v21.4s, v21.4s, v30.4s\n"  /* relu */       \
  "fmax   v22.4s, v22.4s, v30.4s\n"  /* relu */       \
  "fmax   v23.4s, v23.4s, v30.4s\n"  /* relu */       \
  "fmax   v24.4s, v24.4s, v30.4s\n"  /* relu */       \
  "fmax   v25.4s, v25.4s, v30.4s\n"  /* relu */       \
  "fmax   v26.4s, v26.4s, v30.4s\n"  /* relu */       \
  "fmax   v27.4s, v27.4s, v30.4s\n"  /* relu */       \
  "fmax   v28.4s, v28.4s, v30.4s\n"  /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W16_V8_RELU                  \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v9.4s, #0\n"               /* for relu */   \
  "fmax   v21.4s, v21.4s, v9.4s\n"   /* relu */       \
  "fmax   v22.4s, v22.4s, v9.4s\n"   /* relu */       \
  "fmax   v23.4s, v23.4s, v9.4s\n"   /* relu */       \
  "fmax   v24.4s, v24.4s, v9.4s\n"   /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W8_V8_RELU                   \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v9.4s, #0\n"               /* for relu */   \
  "fmax   v21.4s, v21.4s, v9.4s\n"   /* relu */       \
  "fmax   v22.4s, v22.4s, v9.4s\n"   /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W4_V8_RELU                   \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v9.4s, #0\n"               /* for relu */   \
  "fmax   v21.4s, v21.4s, v9.4s\n"   /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W1_V8_RELU                   \
  /* do relu */                                       \
  "cmp    %w[vflag_act],    #0\n"    /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %w[vflag_act],    #1\n"    /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "movi   v9.4s, #0\n"               /* for relu */   \
  "fmax   s0,  s0,  s9\n"                             \
  "b      9f                \n"

#define SPARSE_INT8_INT8_W64_V8_RELU6                   \
  /* do relu6 */                                        \
  "10: \n"                                              \
  "cmp   %w[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n"  /* no act end */  \
  "movi   v6.4s, #0\n"                /* for relu6 */   \
  "dup    v7.4s,  %w[valpha]\n"       /* relu6 alpha */ \
  "fmax   v21.4s, v21.4s, v6.4s\n"    /* relu */        \
  "fmax   v22.4s, v22.4s, v6.4s\n"    /* relu */        \
  "fmax   v23.4s, v23.4s, v6.4s\n"    /* relu */        \
  "fmax   v24.4s, v24.4s, v6.4s\n"    /* relu */        \
  "fmax   v25.4s, v25.4s, v6.4s\n"    /* relu */        \
  "fmax   v26.4s, v26.4s, v6.4s\n"    /* relu */        \
  "fmax   v27.4s, v27.4s, v6.4s\n"    /* relu */        \
  "fmax   v28.4s, v28.4s, v6.4s\n"    /* relu */        \
  "fmax   v29.4s, v29.4s, v6.4s\n"    /* relu */        \
  "fmax   v30.4s, v30.4s, v6.4s\n"    /* relu */        \
  "fmax   v0.4s,  v0.4s,  v6.4s\n"    /* relu */        \
  "fmax   v1.4s,  v1.4s,  v6.4s\n"    /* relu */        \
  "fmax   v2.4s,  v2.4s,  v6.4s\n"    /* relu */        \
  "fmax   v3.4s,  v3.4s,  v6.4s\n"    /* relu */        \
  "fmax   v4.4s,  v4.4s,  v6.4s\n"    /* relu */        \
  "fmax   v5.4s,  v5.4s,  v6.4s\n"    /* relu */        \
  "fmin   v21.4s, v21.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v22.4s, v22.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v23.4s, v23.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v24.4s, v24.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v25.4s, v25.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v26.4s, v26.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v27.4s, v27.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v28.4s, v28.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v29.4s, v29.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v30.4s, v30.4s, v7.4s\n"    /* relu6 */       \
  "fmin   v0.4s,  v0.4s,  v7.4s\n"    /* relu6 */       \
  "fmin   v1.4s,  v1.4s,  v7.4s\n"    /* relu6 */       \
  "fmin   v2.4s,  v2.4s,  v7.4s\n"    /* relu6 */       \
  "fmin   v3.4s,  v3.4s,  v7.4s\n"    /* relu6 */       \
  "fmin   v4.4s,  v4.4s,  v7.4s\n"    /* relu6 */       \
  "fmin   v5.4s,  v5.4s,  v7.4s\n"    /* relu6 */       \
  "b      9f                    \n"   /* relu end */

#define SPARSE_INT8_INT8_W48_V8_RELU6                   \
  /* do relu6 */                                        \
  "10: \n"                                              \
  "cmp   %w[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n"  /* no act end */  \
  "movi   v0.4s, #0\n"                /* for relu6 */   \
  "dup    v1.4s,  %w[valpha]\n"       /* relu6 alpha */ \
  "fmax   v20.4s, v20.4s, v0.4s\n"    /* relu */        \
  "fmax   v21.4s, v21.4s, v0.4s\n"    /* relu */        \
  "fmax   v22.4s, v22.4s, v0.4s\n"    /* relu */        \
  "fmax   v23.4s, v23.4s, v0.4s\n"    /* relu */        \
  "fmax   v24.4s, v24.4s, v0.4s\n"    /* relu */        \
  "fmax   v25.4s, v25.4s, v0.4s\n"    /* relu */        \
  "fmax   v26.4s, v26.4s, v0.4s\n"    /* relu */        \
  "fmax   v27.4s, v27.4s, v0.4s\n"    /* relu */        \
  "fmax   v28.4s, v28.4s, v0.4s\n"    /* relu */        \
  "fmax   v29.4s, v29.4s, v0.4s\n"    /* relu */        \
  "fmax   v30.4s, v30.4s, v0.4s\n"    /* relu */        \
  "fmax   v31.4s, v31.4s, v0.4s\n"    /* relu */        \
  "fmin   v20.4s, v20.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v21.4s, v21.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v22.4s, v22.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v23.4s, v23.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v24.4s, v24.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v25.4s, v25.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v26.4s, v26.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v27.4s, v27.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v28.4s, v28.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v29.4s, v29.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v30.4s, v30.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v31.4s, v31.4s, v1.4s\n"    /* relu6 */       \
  "b      9f                    \n"   /* relu end */

#define SPARSE_INT8_INT8_W32_V8_RELU6                   \
  /* do relu6 */                                        \
  "10: \n"                                              \
  "cmp   %w[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n"  /* no act end */  \
  "movi   v0.4s, #0\n"                /* for relu6 */   \
  "dup    v1.4s,  %w[valpha]\n"       /* relu6 alpha */ \
  "fmax   v21.4s, v21.4s, v0.4s\n"    /* relu */        \
  "fmax   v22.4s, v22.4s, v0.4s\n"    /* relu */        \
  "fmax   v23.4s, v23.4s, v0.4s\n"    /* relu */        \
  "fmax   v24.4s, v24.4s, v0.4s\n"    /* relu */        \
  "fmax   v25.4s, v25.4s, v0.4s\n"    /* relu */        \
  "fmax   v26.4s, v26.4s, v0.4s\n"    /* relu */        \
  "fmax   v27.4s, v27.4s, v0.4s\n"    /* relu */        \
  "fmax   v28.4s, v28.4s, v0.4s\n"    /* relu */        \
  "fmin   v21.4s, v21.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v22.4s, v22.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v23.4s, v23.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v24.4s, v24.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v25.4s, v25.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v26.4s, v26.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v27.4s, v27.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v28.4s, v28.4s, v1.4s\n"    /* relu6 */       \
  "b      9f                    \n"   /* relu end */

#define SPARSE_INT8_INT8_W16_V8_RELU6                   \
  /* do relu6 */                                        \
  "10: \n"                                              \
  "cmp   %w[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n"  /* no act end */  \
  "movi   v0.4s, #0\n"                /* for relu6 */   \
  "dup    v1.4s,  %w[valpha]\n"       /* relu6 alpha */ \
  "fmax   v21.4s, v21.4s, v0.4s\n"    /* relu */        \
  "fmax   v22.4s, v22.4s, v0.4s\n"    /* relu */        \
  "fmax   v23.4s, v23.4s, v0.4s\n"    /* relu */        \
  "fmax   v24.4s, v24.4s, v0.4s\n"    /* relu */        \
  "fmin   v21.4s, v21.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v22.4s, v22.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v23.4s, v23.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v24.4s, v24.4s, v1.4s\n"    /* relu6 */       \
  "b      9f                    \n"   /* relu end */

#define SPARSE_INT8_INT8_W8_V8_RELU6                    \
  /* do relu6 */                                        \
  "10: \n"                                              \
  "cmp   %w[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n"  /* no act end */  \
  "movi   v0.4s, #0\n"                /* for relu6 */   \
  "dup    v1.4s,  %w[valpha]\n"       /* relu6 alpha */ \
  "fmax   v21.4s, v21.4s, v0.4s\n"    /* relu */        \
  "fmax   v22.4s, v22.4s, v0.4s\n"    /* relu */        \
  "fmin   v21.4s, v21.4s, v1.4s\n"    /* relu6 */       \
  "fmin   v22.4s, v22.4s, v1.4s\n"    /* relu6 */       \
  "b      9f                    \n"   /* relu end */

#define SPARSE_INT8_INT8_W4_V8_RELU6                    \
  /* do relu6 */                                        \
  "10: \n"                                              \
  "cmp   %w[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n"  /* no act end */  \
  "movi   v0.4s, #0\n"                /* for relu6 */   \
  "dup    v1.4s,  %w[valpha]\n"       /* relu6 alpha */ \
  "fmax   v21.4s, v21.4s, v0.4s\n"    /* relu */        \
  "fmin   v21.4s, v21.4s, v1.4s\n"    /* relu6 */       \
  "b      9f                    \n"   /* relu end */

#define SPARSE_INT8_INT8_W1_V8_RELU6  \
  /* do relu6 */                      \
  "10: \n"                            \
  "cmp   %w[vflag_act],  #2       \n" \
  "bne   11f                     \n"  \
  "movi   v9.4s, #0\n"                \
  "dup    v1.4s,  %w[valpha]\n"       \
  "fmax   s0,  s0,  s9\n"             \
  "fmin   s0,  s0,  s1\n"             \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W64_V8_LEAKY_RELU                          \
  /* do relu */                                                     \
  "11: \n"                                                          \
  "cmp   %w[vflag_act],  #3       \n"       /* check leakey relu */ \
  "bne   12f                     \n"        /* no act end */        \
  "movi   v6.4s, #0\n"                      /* for relu6 */         \
  "dup    v7.4s,  %w[valpha]\n"             /* leakey relu alpha */ \
  "fcmge  v8.4s,    v21.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v9.4s,    v21.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v10.4s,   v22.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v11.4s,   v22.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v12.4s,   v23.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v13.4s,   v23.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v14.4s,   v24.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v15.4s,   v24.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "bif    v21.16b,  v9.16b,    v8.16b  \n"  /* choose*/             \
  "bif    v22.16b,  v11.16b,   v10.16b  \n" /* choose*/             \
  "bif    v23.16b,  v13.16b,   v12.16b  \n" /* choose*/             \
  "bif    v24.16b,  v15.16b,   v14.16b  \n" /* choose*/             \
  "fcmge  v8.4s,    v25.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v9.4s,    v25.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v10.4s,   v26.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v11.4s,   v26.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v12.4s,   v27.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v13.4s,   v27.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v14.4s,   v28.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v15.4s,   v28.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "bif    v25.16b,  v9.16b,    v8.16b  \n"  /* choose*/             \
  "bif    v26.16b,  v11.16b,   v10.16b  \n" /* choose*/             \
  "bif    v27.16b,  v13.16b,   v12.16b  \n" /* choose*/             \
  "bif    v28.16b,  v15.16b,   v14.16b  \n" /* choose*/             \
  "fcmge  v8.4s,    v29.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v9.4s,    v29.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v10.4s,   v30.4s,    v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v11.4s,   v30.4s,    v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v12.4s,   v0.4s,     v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v13.4s,   v0.4s,     v7.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v14.4s,   v1.4s,     v6.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v15.4s,   v1.4s,     v7.4s   \n"  /* vmulq_f32 */         \
  "bif    v29.16b,  v9.16b,    v8.16b  \n"  /* choose*/             \
  "bif    v30.16b,  v11.16b,   v10.16b  \n" /* choose*/             \
  "bif    v0.16b,   v13.16b,   v12.16b  \n" /* choose*/             \
  "bif    v1.16b,   v15.16b,   v14.16b  \n" /* choose*/             \
  "fcmge  v8.4s,    v2.4s,    v6.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v9.4s,    v2.4s,    v7.4s   \n"   /* vmulq_f32 */         \
  "fcmge  v10.4s,   v3.4s,    v6.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v11.4s,   v3.4s,    v7.4s   \n"   /* vmulq_f32 */         \
  "fcmge  v12.4s,   v4.4s,    v6.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v13.4s,   v4.4s,    v7.4s   \n"   /* vmulq_f32 */         \
  "fcmge  v14.4s,   v5.4s,    v6.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v15.4s,   v5.4s,    v7.4s   \n"   /* vmulq_f32 */         \
  "bif    v2.16b,   v9.16b,   v8.16b  \n"   /* choose*/             \
  "bif    v3.16b,   v11.16b,  v10.16b  \n"  /* choose*/             \
  "bif    v4.16b,   v13.16b,  v12.16b  \n"  /* choose*/             \
  "bif    v5.16b,   v15.16b,  v14.16b  \n"  /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W48_V8_LEAKY_RELU                          \
  /* do relu */                                                     \
  "11: \n"                                                          \
  "cmp   %w[vflag_act],  #3       \n"       /* check leakey relu */ \
  "bne   12f                     \n"        /* no act end */        \
  "movi   v0.4s, #0\n"                      /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"             /* leakey relu alpha */ \
  "fcmge  v2.4s,    v20.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v3.4s,    v20.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v4.4s,    v21.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v5.4s,    v21.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v6.4s,    v22.4s,   v0.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v7.4s,    v22.4s,   v1.4s   \n"   /* vmulq_f32 */         \
  "fcmge  v8.4s,    v23.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v9.4s,    v23.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v10.4s,    v24.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v11.4s,    v24.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v12.4s,    v25.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v13.4s,    v25.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v20.16b,   v3.16b,   v2.16b  \n"  /* choose*/             \
  "bif    v21.16b,   v5.16b,   v4.16b  \n"  /* choose*/             \
  "bif    v22.16b,  v7.16b,   v6.16b  \n"   /* choose*/             \
  "bif    v23.16b,  v9.16b,   v8.16b  \n"   /* choose*/             \
  "bif    v24.16b,  v11.16b,   v10.16b  \n" /* choose*/             \
  "bif    v25.16b,  v13.16b,   v12.16b  \n" /* choose*/             \
  "fcmge  v2.4s,    v26.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v3.4s,    v26.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v4.4s,    v27.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v5.4s,    v27.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v6.4s,    v28.4s,   v0.4s   \n"   /* vcgeq_f32 */         \
  "fmul   v7.4s,    v28.4s,   v1.4s   \n"   /* vmulq_f32 */         \
  "fcmge  v8.4s,    v29.4s,    v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v9.4s,    v29.4s,    v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v10.4s,    v30.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v11.4s,    v30.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v12.4s,    v31.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v13.4s,    v31.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v26.16b,   v3.16b,   v2.16b  \n"  /* choose*/             \
  "bif    v27.16b,   v5.16b,   v4.16b  \n"  /* choose*/             \
  "bif    v28.16b,  v7.16b,   v6.16b  \n"   /* choose*/             \
  "bif    v29.16b,  v9.16b,   v8.16b  \n"   /* choose*/             \
  "bif    v30.16b,  v11.16b,   v10.16b  \n" /* choose*/             \
  "bif    v31.16b,  v13.16b,   v12.16b  \n" /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W32_V8_LEAKY_RELU                         \
  /* do relu */                                                    \
  "11: \n"                                                         \
  "cmp   %w[vflag_act],  #3       \n"      /* check leakey relu */ \
  "bne   12f                     \n"       /* no act end */        \
  "movi   v0.4s, #0\n"                     /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"            /* leakey relu alpha */ \
  "fcmge  v2.4s,    v21.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v21.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v4.4s,    v22.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v5.4s,    v22.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v6.4s,    v23.4s,   v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v7.4s,    v23.4s,   v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v8.4s,    v24.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v9.4s,    v24.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v21.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "bif    v22.16b,   v5.16b,   v4.16b  \n" /* choose*/             \
  "bif    v23.16b,  v7.16b,   v6.16b  \n"  /* choose*/             \
  "bif    v24.16b,  v9.16b,   v8.16b  \n"  /* choose*/             \
  "fcmge  v2.4s,    v25.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v25.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v4.4s,    v26.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v5.4s,    v26.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v6.4s,    v27.4s,   v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v7.4s,    v27.4s,   v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v8.4s,    v28.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v9.4s,    v28.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v25.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "bif    v26.16b,   v5.16b,   v4.16b  \n" /* choose*/             \
  "bif    v27.16b,  v7.16b,   v6.16b  \n"  /* choose*/             \
  "bif    v28.16b,  v9.16b,   v8.16b  \n"  /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W16_V8_LEAKY_RELU                         \
  /* do relu */                                                    \
  "11: \n"                                                         \
  "cmp   %w[vflag_act],  #3       \n"      /* check leakey relu */ \
  "bne   12f                     \n"       /* no act end */        \
  "movi   v0.4s, #0\n"                     /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"            /* leakey relu alpha */ \
  "fcmge  v2.4s,    v21.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v21.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v4.4s,    v22.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v5.4s,    v22.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v6.4s,    v23.4s,   v0.4s   \n"  /* vcgeq_f32 */         \
  "fmul   v7.4s,    v23.4s,   v1.4s   \n"  /* vmulq_f32 */         \
  "fcmge  v8.4s,    v24.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v9.4s,    v24.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v21.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "bif    v22.16b,   v5.16b,   v4.16b  \n" /* choose*/             \
  "bif    v23.16b,  v7.16b,   v6.16b  \n"  /* choose*/             \
  "bif    v24.16b,  v9.16b,   v8.16b  \n"  /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W8_V8_LEAKY_RELU                          \
  /* do relu */                                                    \
  "11: \n"                                                         \
  "cmp   %w[vflag_act],  #3       \n"      /* check leakey relu */ \
  "bne   12f                     \n"       /* no act end */        \
  "movi   v0.4s, #0\n"                     /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"            /* leakey relu alpha */ \
  "fcmge  v2.4s,    v21.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v21.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "fcmge  v4.4s,    v22.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v5.4s,    v22.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v21.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "bif    v22.16b,   v5.16b,   v4.16b  \n" /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W4_V8_LEAKY_RELU                          \
  /* do relu */                                                    \
  "11: \n"                                                         \
  "cmp   %w[vflag_act],  #3       \n"      /* check leakey relu */ \
  "bne   12f                     \n"       /* no act end */        \
  "movi   v0.4s, #0\n"                     /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"            /* leakey relu alpha */ \
  "fcmge  v2.4s,    v21.4s,    v0.4s   \n" /* vcgeq_f32 */         \
  "fmul   v3.4s,    v21.4s,    v1.4s   \n" /* vmulq_f32 */         \
  "bif    v21.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W1_V8_LEAKY_RELU                         \
  /* do relu */                                                   \
  "11: \n"                                                        \
  "cmp    %w[vflag_act],  #3       \n"                            \
  "bne    12f                     \n"                             \
  "movi   v9.4s, #0\n"                    /* for relu6 */         \
  "dup    v1.4s,  %w[valpha]\n"           /* leakey relu alpha */ \
  "fcmge  s2,    s0,    s9   \n"          /* vcgeq_f32 */         \
  "fmul   s3,    s0,    s1   \n"          /* vmulq_f32 */         \
  "bif    v0.16b,   v3.16b,   v2.16b  \n" /* choose*/             \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W64_V8_HARD_SWISH                       \
  /* do relu */                                                  \
  "12: \n"                                                       \
  "movi   v6.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q7,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q8,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q9,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v10.4s,  v21.4s, v7.4s        \n"                      \
  "fadd   v12.4s,  v22.4s, v7.4s        \n"                      \
  "fadd   v14.4s,  v23.4s, v7.4s        \n"                      \
  "fmul   v11.4s,  v21.4s, v8.4s        \n"                      \
  "fmul   v13.4s,  v22.4s, v8.4s        \n"                      \
  "fmul   v15.4s,  v23.4s, v8.4s        \n"                      \
  "fmax   v10.4s, v10.4s, v6.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v6.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v6.4s       \n"                        \
  "fmin   v10.4s, v10.4s, v9.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v9.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v9.4s       \n"                        \
  "fmul   v21.4s, v11.4s, v10.4s        \n"                      \
  "fmul   v22.4s, v13.4s, v12.4s        \n"                      \
  "fmul   v23.4s, v15.4s, v14.4s        \n"                      \
  "fadd   v10.4s,  v24.4s, v7.4s        \n"                      \
  "fadd   v12.4s,  v25.4s, v7.4s        \n"                      \
  "fadd   v14.4s,  v26.4s, v7.4s        \n"                      \
  "fmul   v11.4s,  v24.4s, v8.4s        \n"                      \
  "fmul   v13.4s,  v25.4s, v8.4s        \n"                      \
  "fmul   v15.4s,  v26.4s, v8.4s        \n"                      \
  "fmax   v10.4s, v10.4s, v6.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v6.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v6.4s       \n"                        \
  "fmin   v10.4s, v10.4s, v9.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v9.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v9.4s       \n"                        \
  "fmul   v24.4s, v11.4s, v10.4s        \n"                      \
  "fmul   v25.4s, v13.4s, v12.4s        \n"                      \
  "fmul   v26.4s, v15.4s, v14.4s        \n"                      \
  "fadd   v10.4s,  v27.4s, v7.4s        \n"                      \
  "fadd   v12.4s,  v28.4s, v7.4s        \n"                      \
  "fadd   v14.4s,  v29.4s, v7.4s        \n"                      \
  "fmul   v11.4s,  v27.4s, v8.4s        \n"                      \
  "fmul   v13.4s,  v28.4s, v8.4s        \n"                      \
  "fmul   v15.4s,  v29.4s, v8.4s        \n"                      \
  "fmax   v10.4s, v10.4s, v6.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v6.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v6.4s       \n"                        \
  "fmin   v10.4s, v10.4s, v9.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v9.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v9.4s       \n"                        \
  "fmul   v27.4s, v11.4s, v10.4s        \n"                      \
  "fmul   v28.4s, v13.4s, v12.4s        \n"                      \
  "fmul   v29.4s, v15.4s, v14.4s        \n"                      \
  "fadd   v10.4s,  v30.4s, v7.4s        \n"                      \
  "fadd   v12.4s,  v0.4s, v7.4s        \n"                       \
  "fadd   v14.4s,  v1.4s, v7.4s        \n"                       \
  "fmul   v11.4s,  v30.4s, v8.4s        \n"                      \
  "fmul   v13.4s,  v0.4s, v8.4s        \n"                       \
  "fmul   v15.4s,  v1.4s, v8.4s        \n"                       \
  "fmax   v10.4s, v10.4s, v6.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v6.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v6.4s       \n"                        \
  "fmin   v10.4s, v10.4s, v9.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v9.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v9.4s       \n"                        \
  "fmul   v30.4s, v11.4s, v10.4s        \n"                      \
  "fmul   v0.4s, v13.4s, v12.4s        \n"                       \
  "fmul   v1.4s, v15.4s, v14.4s        \n"                       \
  "fadd   v10.4s, v2.4s, v7.4s        \n"                        \
  "fadd   v12.4s, v3.4s, v7.4s        \n"                        \
  "fadd   v14.4s, v4.4s, v7.4s        \n"                        \
  "fmul   v11.4s, v2.4s, v8.4s        \n"                        \
  "fmul   v13.4s, v3.4s, v8.4s        \n"                        \
  "fmul   v15.4s, v4.4s, v8.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v6.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v6.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v6.4s       \n"                        \
  "fmin   v10.4s, v10.4s, v9.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v9.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v9.4s       \n"                        \
  "fmul   v2.4s, v11.4s, v10.4s        \n"                       \
  "fmul   v3.4s, v13.4s, v12.4s        \n"                       \
  "fmul   v4.4s, v15.4s, v14.4s        \n"                       \
  "fadd   v10.4s, v5.4s, v7.4s        \n"                        \
  "fmul   v11.4s, v5.4s, v8.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v6.4s       \n"                        \
  "fmin   v10.4s, v10.4s, v9.4s       \n"                        \
  "fmul   v5.4s, v11.4s, v10.4s        \n"                       \
  "9:\n"

#define SPARSE_INT8_INT8_W48_V8_HARD_SWISH                       \
  /* do relu */                                                  \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v4.4s,  v20.4s, v1.4s        \n"                       \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v22.4s, v1.4s        \n"                       \
  "fadd   v10.4s,  v23.4s, v1.4s        \n"                      \
  "fadd   v12.4s,  v24.4s, v1.4s        \n"                      \
  "fadd   v14.4s,  v25.4s, v1.4s        \n"                      \
  "fadd   v16.4s,  v26.4s, v1.4s        \n"                      \
  "fadd   v18.4s,  v27.4s, v1.4s        \n"                      \
  "fmul   v5.4s,   v20.4s, v2.4s        \n"                      \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v22.4s, v2.4s        \n"                      \
  "fmul   v11.4s,  v23.4s, v2.4s        \n"                      \
  "fmul   v13.4s,  v24.4s, v2.4s        \n"                      \
  "fmul   v15.4s,  v25.4s, v2.4s        \n"                      \
  "fmul   v17.4s,  v26.4s, v2.4s        \n"                      \
  "fmul   v19.4s,  v27.4s, v2.4s        \n"                      \
  "fmax   v4.4s,  v4.4s, v0.4s        \n"                        \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v0.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v0.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v0.4s       \n"                        \
  "fmax   v16.4s, v16.4s, v0.4s       \n"                        \
  "fmax   v18.4s, v18.4s, v0.4s       \n"                        \
  "fmin   v4.4s,  v4.4s, v3.4s        \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmin   v10.4s, v10.4s, v3.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v3.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v3.4s       \n"                        \
  "fmin   v16.4s, v16.4s, v3.4s       \n"                        \
  "fmin   v18.4s, v18.4s, v3.4s       \n"                        \
  "fmul   v20.4s,  v5.4s,  v4.4s        \n"                      \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v22.4s,  v9.4s,  v8.4s        \n"                      \
  "fmul   v23.4s,  v11.4s, v10.4s        \n"                     \
  "fmul   v24.4s,  v13.4s, v12.4s        \n"                     \
  "fmul   v25.4s,  v15.4s, v14.4s        \n"                     \
  "fmul   v26.4s,  v17.4s, v16.4s        \n"                     \
  "fmul   v27.4s,  v19.4s, v18.4s        \n"                     \
  "fadd   v4.4s,  v28.4s, v1.4s        \n"                       \
  "fadd   v6.4s,  v29.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v30.4s, v1.4s        \n"                       \
  "fadd   v10.4s,  v31.4s, v1.4s        \n"                      \
  "fmul   v5.4s,   v28.4s, v2.4s        \n"                      \
  "fmul   v7.4s,   v29.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v30.4s, v2.4s        \n"                      \
  "fmul   v11.4s,  v31.4s, v2.4s        \n"                      \
  "fmax   v4.4s,  v4.4s, v0.4s        \n"                        \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v0.4s       \n"                        \
  "fmin   v4.4s,  v4.4s, v3.4s        \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmin   v10.4s, v10.4s, v3.4s       \n"                        \
  "fmul   v28.4s,  v5.4s,  v4.4s        \n"                      \
  "fmul   v29.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v30.4s,  v9.4s,  v8.4s        \n"                      \
  "fmul   v31.4s,  v11.4s, v10.4s        \n"                     \
  "9:\n"

#define SPARSE_INT8_INT8_W32_V8_HARD_SWISH                       \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v22.4s, v1.4s        \n"                       \
  "fadd   v10.4s,  v23.4s, v1.4s        \n"                      \
  "fadd   v12.4s,  v24.4s, v1.4s        \n"                      \
  "fadd   v14.4s,  v25.4s, v1.4s        \n"                      \
  "fadd   v16.4s,  v26.4s, v1.4s        \n"                      \
  "fadd   v18.4s,  v27.4s, v1.4s        \n"                      \
  "fadd   v30.4s,  v28.4s, v1.4s        \n"                      \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v22.4s, v2.4s        \n"                      \
  "fmul   v11.4s,  v23.4s, v2.4s        \n"                      \
  "fmul   v13.4s,  v24.4s, v2.4s        \n"                      \
  "fmul   v15.4s,  v25.4s, v2.4s        \n"                      \
  "fmul   v17.4s,  v26.4s, v2.4s        \n"                      \
  "fmul   v19.4s,  v27.4s, v2.4s        \n"                      \
  "fmul   v31.4s,  v28.4s, v2.4s        \n"                      \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v0.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v0.4s       \n"                        \
  "fmax   v14.4s, v14.4s, v0.4s       \n"                        \
  "fmax   v16.4s, v16.4s, v0.4s       \n"                        \
  "fmax   v18.4s, v18.4s, v0.4s       \n"                        \
  "fmax   v30.4s, v30.4s, v0.4s       \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmin   v10.4s, v10.4s, v3.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v3.4s       \n"                        \
  "fmin   v14.4s, v14.4s, v3.4s       \n"                        \
  "fmin   v16.4s, v16.4s, v3.4s       \n"                        \
  "fmin   v18.4s, v18.4s, v3.4s       \n"                        \
  "fmin   v30.4s, v30.4s, v3.4s       \n"                        \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v22.4s,  v9.4s,  v8.4s        \n"                      \
  "fmul   v23.4s,  v11.4s, v10.4s        \n"                     \
  "fmul   v24.4s,  v13.4s, v12.4s        \n"                     \
  "fmul   v25.4s,  v15.4s, v14.4s        \n"                     \
  "fmul   v26.4s,  v17.4s, v16.4s        \n"                     \
  "fmul   v27.4s,  v19.4s, v18.4s        \n"                     \
  "fmul   v28.4s,  v31.4s, v30.4s        \n"                     \
  "9:\n"

#define SPARSE_INT8_INT8_W16_V8_HARD_SWISH                       \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v22.4s, v1.4s        \n"                       \
  "fadd   v10.4s,  v23.4s, v1.4s        \n"                      \
  "fadd   v12.4s,  v24.4s, v1.4s        \n"                      \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v22.4s, v2.4s        \n"                      \
  "fmul   v11.4s,  v23.4s, v2.4s        \n"                      \
  "fmul   v13.4s,  v24.4s, v2.4s        \n"                      \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmax   v10.4s, v10.4s, v0.4s       \n"                        \
  "fmax   v12.4s, v12.4s, v0.4s       \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmin   v10.4s, v10.4s, v3.4s       \n"                        \
  "fmin   v12.4s, v12.4s, v3.4s       \n"                        \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v22.4s,  v9.4s,  v8.4s        \n"                      \
  "fmul   v23.4s,  v11.4s, v10.4s        \n"                     \
  "fmul   v24.4s,  v13.4s, v12.4s        \n"                     \
  "9:\n"

#define SPARSE_INT8_INT8_W8_V8_HARD_SWISH                        \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fadd   v8.4s,  v22.4s, v1.4s        \n"                       \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmul   v9.4s,   v22.4s, v2.4s        \n"                      \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmax   v8.4s,  v8.4s, v0.4s        \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmin   v8.4s,  v8.4s, v3.4s        \n"                        \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "fmul   v22.4s,  v9.4s,  v8.4s        \n"                      \
  "9:\n"

#define SPARSE_INT8_INT8_W4_V8_HARD_SWISH                        \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v0.4s,    #0                \n"    /* for hardswish */ \
  "ldr    q1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    q2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    q3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   v6.4s,  v21.4s, v1.4s        \n"                       \
  "fmul   v7.4s,   v21.4s, v2.4s        \n"                      \
  "fmax   v6.4s,  v6.4s, v0.4s        \n"                        \
  "fmin   v6.4s,  v6.4s, v3.4s        \n"                        \
  "fmul   v21.4s,  v7.4s,  v6.4s        \n"                      \
  "9:\n"

#define SPARSE_INT8_INT8_W1_V8_HARD_SWISH                        \
  /* do hard_swish */                                            \
  "12: \n"                                                       \
  "movi   v9.4s,    #0                \n"    /* for hardswish */ \
  "ldr    s1,  [%[hs_param], #0]         \n" /* offset */        \
  "ldr    s2,  [%[hs_param], #16]        \n" /* scale */         \
  "ldr    s3,  [%[hs_param], #32]        \n" /* threshold */     \
  "fadd   s6,  s0,  s1        \n"                                \
  "fmul   s7,  s0,  s2        \n"                                \
  "fmax   s6,  s6,  s9        \n"                                \
  "fmin   s6,  s6,  s3        \n"                                \
  "fmul   s0,  s7,  s6        \n"                                \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx64, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx64, and the required data is
 * MxKxKx64.
 */
#define SPARSE_INT8_INT8_W64_V8_OUT                                            \
  SPARSE_INT8_INT8_W64_V8_KERNEL                                               \
  SPARSE_INT8_INT8_W64_V8_RELU                                                 \
  SPARSE_INT8_INT8_W64_V8_RELU6                                                \
  SPARSE_INT8_INT8_W64_V8_LEAKY_RELU                                           \
  SPARSE_INT8_INT8_W64_V8_HARD_SWISH                                           \
  /* store result */                                                           \
  "ld1    {v15.4s},   [%[vmax]] \n" /* v8 = -127 */ /* data >= -127 */         \
  "fcmge v6.4s,  v21.4s, v15.4s\n"                                             \
  "fcmge v7.4s,  v22.4s, v15.4s\n"                                             \
  "fcmge v8.4s,  v23.4s, v15.4s\n"                                             \
  "fcmge v9.4s,  v24.4s, v15.4s\n"                                             \
  "fcmge v10.4s,  v25.4s, v15.4s\n"                                            \
  "fcmge v11.4s,  v26.4s, v15.4s\n"                                            \
  "fcmge v12.4s,  v27.4s, v15.4s\n"                                            \
  "fcmge v13.4s,  v28.4s, v15.4s\n"                                            \
  "fcmge v14.4s,  v29.4s, v15.4s\n"                                            \
  "bif v21.16b, v15.16b, v6.16b           \n"                                  \
  "bif v22.16b, v15.16b, v7.16b           \n"                                  \
  "bif v23.16b, v15.16b, v8.16b           \n"                                  \
  "bif v24.16b, v15.16b, v9.16b           \n"                                  \
  "bif v25.16b, v15.16b, v10.16b           \n"                                 \
  "bif v26.16b, v15.16b, v11.16b           \n"                                 \
  "bif v27.16b, v15.16b, v12.16b           \n"                                 \
  "bif v28.16b, v15.16b, v13.16b           \n"                                 \
  "bif v29.16b, v15.16b, v14.16b           \n"                                 \
  "fcmge v6.4s,  v30.4s, v15.4s\n"                                             \
  "fcmge v7.4s,  v0.4s,  v15.4s\n"                                             \
  "fcmge v8.4s,  v1.4s,  v15.4s\n"                                             \
  "fcmge v9.4s,  v2.4s,  v15.4s\n"                                             \
  "fcmge v10.4s, v3.4s,  v15.4s\n"                                             \
  "fcmge v11.4s, v4.4s,  v15.4s\n"                                             \
  "fcmge v12.4s, v5.4s,  v15.4s\n"                                             \
  "bif v30.16b, v15.16b, v6.16b           \n"                                  \
  "bif v0.16b,  v15.16b, v7.16b           \n"                                  \
  "bif v1.16b,  v15.16b, v8.16b           \n"                                  \
  "bif v2.16b,  v15.16b, v9.16b           \n"                                  \
  "bif v3.16b,  v15.16b, v10.16b           \n"                                 \
  "bif v4.16b,  v15.16b, v11.16b           \n"                                 \
  "bif v5.16b,  v15.16b, v12.16b           \n"                                 \
  "fcvtas v6.4s, v21.4s\n"                           /*  cvt to int */         \
  "fcvtas v7.4s, v22.4s\n"                           /*  cvt to int */         \
  "fcvtas v8.4s, v23.4s\n"                           /*  cvt to int */         \
  "fcvtas v9.4s, v24.4s\n"                           /*  cvt to int */         \
  "fcvtas v10.4s, v25.4s\n"                          /*  cvt to int */         \
  "fcvtas v11.4s, v26.4s\n"                          /*  cvt to int */         \
  "fcvtas v12.4s, v27.4s\n"                          /*  cvt to int */         \
  "fcvtas v13.4s, v28.4s\n"                          /*  cvt to int */         \
  "fcvtas v14.4s, v29.4s\n"                          /*  cvt to int */         \
  "fcvtas v15.4s, v30.4s\n"                          /*  cvt to int */         \
  "fcvtas v16.4s, v0.4s\n"                           /*  cvt to int */         \
  "fcvtas v17.4s, v1.4s\n"                           /*  cvt to int */         \
  "fcvtas v18.4s, v2.4s\n"                           /*  cvt to int */         \
  "fcvtas v19.4s, v3.4s\n"                           /*  cvt to int */         \
  "fcvtas v20.4s, v4.4s\n"                           /*  cvt to int */         \
  "fcvtas v21.4s, v5.4s\n"                           /*  cvt to int */         \
  "sqxtn  v0.4h,  v6.4s\n"                           /*  cvt int32 to int16 */ \
  "sqxtn2 v0.8h,  v7.4s\n"                           /*  cvt int32 to int16 */ \
  "sqxtn  v1.4h,  v8.4s\n"                           /*  cvt int32 to int16 */ \
  "sqxtn2 v1.8h,  v9.4s\n"                           /*  cvt int32 to int16 */ \
  "sqxtn  v2.4h,  v10.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn2 v2.8h,  v11.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn  v3.4h,  v12.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn2 v3.8h,  v13.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn  v4.4h,  v14.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn2 v4.8h,  v15.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn  v5.4h,  v16.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn2 v5.8h,  v17.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn  v6.4h,  v18.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn2 v6.8h,  v19.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn  v7.4h,  v20.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn2 v7.8h,  v21.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn  v10.8b, v0.8h\n"                           /*  cvt int16 to int8 */  \
  "sqxtn2 v10.16b, v1.8h\n"                          /*  cvt int16 to int8 */  \
  "sqxtn  v11.8b,  v2.8h\n"                          /*  cvt int16 to int8 */  \
  "sqxtn2 v11.16b, v3.8h\n"                          /*  cvt int16 to int8 */  \
  "sqxtn  v12.8b,  v4.8h\n"                          /*  cvt int16 to int8 */  \
  "sqxtn2 v12.16b, v5.8h\n" /*  cvt int16 to int8 */ /* store result */        \
  "sqxtn  v13.8b,  v6.8h\n"                          /*  cvt int16 to int8 */  \
  "sqxtn2 v13.16b, v7.8h\n"                          /*  cvt int16 to int8 */  \
  "stp   q10, q11,  [%[c_ptr]]\n"                                              \
  "stp   q12, q13,  [%[c_ptr], #32]\n"

/**
 * The data block size for sparse matrix calculation is Mx48, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx48, and the required data is
 * MxKxKx48.
 */
#define SPARSE_INT8_INT8_W48_V8_OUT                                      \
  SPARSE_INT8_INT8_W48_V8_KERNEL                                         \
  SPARSE_INT8_INT8_W48_V8_RELU                                           \
  SPARSE_INT8_INT8_W48_V8_RELU6                                          \
  SPARSE_INT8_INT8_W48_V8_LEAKY_RELU                                     \
  SPARSE_INT8_INT8_W48_V8_HARD_SWISH                                     \
  /* store result */                                                     \
  "ld1    {v12.4s},   [%[vmax]] \n" /* v8 = -127 */ /* data >= -127 */   \
  "fcmge v0.4s,  v20.4s, v12.4s\n"                                       \
  "fcmge v1.4s,  v21.4s, v12.4s\n"                                       \
  "fcmge v2.4s,  v22.4s, v12.4s\n"                                       \
  "fcmge v3.4s,  v23.4s, v12.4s\n"                                       \
  "fcmge v4.4s,  v24.4s, v12.4s\n"                                       \
  "fcmge v5.4s,  v25.4s, v12.4s\n"                                       \
  "fcmge v6.4s,  v26.4s, v12.4s\n"                                       \
  "fcmge v7.4s,  v27.4s, v12.4s\n"                                       \
  "fcmge v8.4s,  v28.4s, v12.4s\n"                                       \
  "fcmge v9.4s,  v29.4s, v12.4s\n"                                       \
  "fcmge v10.4s, v30.4s, v12.4s\n"                                       \
  "fcmge v11.4s, v31.4s, v12.4s\n" /* choose data */                     \
  "bif v20.16b, v12.16b, v0.16b           \n"                            \
  "bif v21.16b, v12.16b, v1.16b           \n"                            \
  "bif v22.16b, v12.16b, v2.16b           \n"                            \
  "bif v23.16b, v12.16b, v3.16b           \n"                            \
  "bif v24.16b, v12.16b, v4.16b           \n"                            \
  "bif v25.16b, v12.16b, v5.16b           \n"                            \
  "bif v26.16b, v12.16b, v6.16b           \n"                            \
  "bif v27.16b, v12.16b, v7.16b           \n"                            \
  "bif v28.16b, v12.16b, v8.16b           \n"                            \
  "bif v29.16b, v12.16b, v9.16b           \n"                            \
  "bif v30.16b, v12.16b, v10.16b          \n"                            \
  "bif v31.16b, v12.16b, v11.16b          \n"                            \
  "fcvtas v0.4s, v20.4s\n"   /*  cvt to int */                           \
  "fcvtas v1.4s, v21.4s\n"   /*  cvt to int */                           \
  "fcvtas v2.4s, v22.4s\n"   /*  cvt to int */                           \
  "fcvtas v3.4s, v23.4s\n"   /*  cvt to int */                           \
  "fcvtas v4.4s, v24.4s\n"   /*  cvt to int */                           \
  "fcvtas v5.4s, v25.4s\n"   /*  cvt to int */                           \
  "fcvtas v6.4s, v26.4s\n"   /*  cvt to int */                           \
  "fcvtas v7.4s, v27.4s\n"   /*  cvt to int */                           \
  "fcvtas v8.4s, v28.4s\n"   /*  cvt to int */                           \
  "fcvtas v9.4s, v29.4s\n"   /*  cvt to int */                           \
  "fcvtas v10.4s, v30.4s\n"  /*  cvt to int */                           \
  "fcvtas v11.4s, v31.4s\n"  /*  cvt to int */                           \
  "sqxtn  v14.4h, v0.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn2 v14.8h, v1.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn  v15.4h, v2.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn2 v15.8h, v3.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn  v16.4h, v4.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn2 v16.8h, v5.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn  v17.4h, v6.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn2 v17.8h, v7.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn  v18.4h, v8.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn2 v18.8h, v9.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn  v19.4h, v10.4s\n"  /*  cvt int32 to int16 */                   \
  "sqxtn2 v19.8h, v11.4s\n"  /*  cvt int32 to int16 */                   \
  "sqxtn  v21.8b, v14.8h\n"  /*  cvt int16 to int8 */                    \
  "sqxtn2 v21.16b, v15.8h\n" /*  cvt int16 to int8 */                    \
  "sqxtn  v22.8b, v16.8h\n"  /*  cvt int16 to int8 */                    \
  "sqxtn2 v22.16b, v17.8h\n" /*  cvt int16 to int8 */                    \
  "sqxtn  v23.8b, v18.8h\n"  /*  cvt int16 to int8 */                    \
  "sqxtn2 v23.16b, v19.8h\n" /*  cvt int16 to int8 */ /* store result */ \
  "stp   q21, q22,  [%[c_ptr]]\n"                                        \
  "str   q23,  [%[c_ptr], #32]\n"

/**
 * The data block size for sparse matrix calculation is Mx32, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx32, and the required data is
 * MxKxKx32.
 */
#define SPARSE_INT8_INT8_W32_V8_OUT                                      \
  SPARSE_INT8_INT8_W32_V8_KERNEL                                         \
  SPARSE_INT8_INT8_W32_V8_RELU                                           \
  SPARSE_INT8_INT8_W32_V8_RELU6                                          \
  SPARSE_INT8_INT8_W32_V8_LEAKY_RELU                                     \
  SPARSE_INT8_INT8_W32_V8_HARD_SWISH                                     \
  /* store result */                                                     \
  "ld1    {v8.4s},   [%[vmax]] \n" /* v8 = -127 */ /* data >= -127 */    \
  "fcmge v0.4s, v21.4s, v8.4s\n"                                         \
  "fcmge v1.4s, v22.4s, v8.4s\n"                                         \
  "fcmge v2.4s, v23.4s, v8.4s\n"                                         \
  "fcmge v3.4s, v24.4s, v8.4s\n"                                         \
  "fcmge v4.4s, v25.4s, v8.4s\n"                                         \
  "fcmge v5.4s, v26.4s, v8.4s\n"                                         \
  "fcmge v6.4s, v27.4s, v8.4s\n"                                         \
  "fcmge v7.4s, v28.4s, v8.4s\n" /* choose data */                       \
  "bif v21.16b,  v8.16b, v0.16b           \n"                            \
  "bif v22.16b, v8.16b, v1.16b            \n"                            \
  "bif v23.16b, v8.16b, v2.16b            \n"                            \
  "bif v24.16b, v8.16b, v3.16b            \n"                            \
  "bif v25.16b, v8.16b, v4.16b            \n"                            \
  "bif v26.16b, v8.16b, v5.16b            \n"                            \
  "bif v27.16b, v8.16b, v6.16b            \n"                            \
  "bif v28.16b, v8.16b, v7.16b            \n"                            \
  "fcvtas v0.4s, v21.4s\n"   /*  cvt to int */                           \
  "fcvtas v1.4s, v22.4s\n"   /*  cvt to int */                           \
  "fcvtas v2.4s, v23.4s\n"   /*  cvt to int */                           \
  "fcvtas v3.4s, v24.4s\n"   /*  cvt to int */                           \
  "fcvtas v4.4s, v25.4s\n"   /*  cvt to int */                           \
  "fcvtas v5.4s, v26.4s\n"   /*  cvt to int */                           \
  "fcvtas v6.4s, v27.4s\n"   /*  cvt to int */                           \
  "fcvtas v7.4s, v28.4s\n"   /*  cvt to int */                           \
  "sqxtn  v16.4h, v0.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn2 v16.8h, v1.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn  v17.4h, v2.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn2 v17.8h, v3.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn  v18.4h, v4.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn2 v18.8h, v5.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn  v19.4h, v6.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn2 v19.8h, v7.4s\n"   /*  cvt int32 to int16 */                   \
  "sqxtn  v21.8b, v16.8h\n"  /*  cvt int16 to int8 */                    \
  "sqxtn2 v21.16b, v17.8h\n" /*  cvt int16 to int8 */                    \
  "sqxtn  v22.8b, v18.8h\n"  /*  cvt int16 to int8 */                    \
  "sqxtn2 v22.16b, v19.8h\n" /*  cvt int16 to int8 */ /* store result */ \
  "stp   q21, q22,  [%[c_ptr]]\n"

/**
 * The data block size for sparse matrix calculation is Mx16, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx16, and the required data is
 * MxKxKx16.
 */
#define SPARSE_INT8_INT8_W16_V8_OUT                                      \
  SPARSE_INT8_INT8_W16_V8_KERNEL                                         \
  SPARSE_INT8_INT8_W16_V8_RELU                                           \
  SPARSE_INT8_INT8_W16_V8_RELU6                                          \
  SPARSE_INT8_INT8_W16_V8_LEAKY_RELU                                     \
  SPARSE_INT8_INT8_W16_V8_HARD_SWISH                                     \
  "ld1    {v8.4s},   [%[vmax]] \n" /* v8 = -127 */ /* data >= -127 */    \
  "fcmge v0.4s, v21.4s, v8.4s\n"                                         \
  "fcmge v1.4s, v22.4s, v8.4s\n"                                         \
  "fcmge v2.4s, v23.4s, v8.4s\n"                                         \
  "fcmge v3.4s, v24.4s, v8.4s\n" /* choose data */                       \
  "bif v21.16b,  v8.16b, v0.16b           \n"                            \
  "bif v22.16b, v8.16b, v1.16b            \n"                            \
  "bif v23.16b, v8.16b, v2.16b            \n"                            \
  "bif v24.16b, v8.16b, v3.16b            \n"                            \
  "fcvtas v0.4s, v21.4s\n"  /*  cvt to int */                            \
  "fcvtas v1.4s, v22.4s\n"  /*  cvt to int */                            \
  "fcvtas v2.4s, v23.4s\n"  /*  cvt to int */                            \
  "fcvtas v3.4s, v24.4s\n"  /*  cvt to int */                            \
  "sqxtn  v16.4h, v0.4s\n"  /*  cvt int32 to int16 */                    \
  "sqxtn2 v16.8h, v1.4s\n"  /*  cvt int32 to int16 */                    \
  "sqxtn  v17.4h, v2.4s\n"  /*  cvt int32 to int16 */                    \
  "sqxtn2 v17.8h, v3.4s\n"  /*  cvt int32 to int16 */                    \
  "sqxtn  v21.8b, v16.8h\n" /*  cvt int16 to int8 */                     \
  "sqxtn2 v21.16b, v17.8h\n" /*  cvt int16 to int8 */ /* store result */ \
  "str   q21,  [%[c_ptr]]\n"

/**
 * The data block size for sparse matrix calculation is Mx8, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx8, and the required data is
 * MxKxKx8.
 */
#define SPARSE_INT8_INT8_W8_V8_OUT                                             \
  SPARSE_INT8_INT8_W8_V8_KERNEL                                                \
  SPARSE_INT8_INT8_W8_V8_RELU                                                  \
  SPARSE_INT8_INT8_W8_V8_RELU6                                                 \
  SPARSE_INT8_INT8_W8_V8_LEAKY_RELU                                            \
  SPARSE_INT8_INT8_W8_V8_HARD_SWISH                                            \
  "ld1    {v8.4s},   [%[vmax]] \n" /* v8 = -127 */ /* data >= -127 */          \
  "fcmge v0.4s, v21.4s, v8.4s\n"                                               \
  "fcmge v1.4s, v22.4s, v8.4s\n" /* choose data */                             \
  "bif v21.16b,  v8.16b, v0.16b            \n"                                 \
  "bif v22.16b, v8.16b, v1.16b            \n"                                  \
  "fcvtas v0.4s, v21.4s\n"                           /*  cvt to int */         \
  "fcvtas v1.4s, v22.4s\n"                           /*  cvt to int */         \
  "sqxtn  v16.4h, v0.4s\n"                           /*  cvt int32 to int16 */ \
  "sqxtn2 v16.8h, v1.4s\n"                           /*  cvt int32 to int16 */ \
  "sqxtn  v21.8b, v16.8h\n" /*  cvt int16 to int8 */ /* store result */        \
  "str   d21,  [%[c_ptr]]\n"

/**
 * The data block size for sparse matrix calculation is Mx4, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx4, and the required data is
 * MxKxKx4.
 */
#define SPARSE_INT8_INT8_W4_V8_OUT                                            \
  SPARSE_INT8_INT8_W4_V8_KERNEL                                               \
  SPARSE_INT8_INT8_W4_V8_RELU                                                 \
  SPARSE_INT8_INT8_W4_V8_RELU6                                                \
  SPARSE_INT8_INT8_W4_V8_LEAKY_RELU                                           \
  SPARSE_INT8_INT8_W4_V8_HARD_SWISH                                           \
  "ld1    {v8.4s},   [%[vmax]]  \n" /* v8 = -127 */ /* data >= -127 */        \
  "fcmge v0.4s, v21.4s, v8.4s   \n"                 /* choose data */         \
  "bif v21.16b,  v8.16b, v0.16b \n"                                           \
  "fcvtas v0.4s, v21.4s\n"                          /*  cvt to int */         \
  "sqxtn  v16.4h, v0.4s\n"                          /*  cvt int32 to int16 */ \
  "sqxtn  v21.8b, v16.8h\n" /* cvt int16 to int8 */ /* store result */        \
  "str   s21,  [%[c_ptr]]\n"

#define SPARSE_INT8_INT8_W1_V8_OUT                                          \
  SPARSE_INT8_INT8_W1_V8_KERNEL                                             \
  SPARSE_INT8_INT8_W1_V8_RELU                                               \
  SPARSE_INT8_INT8_W1_V8_RELU6                                              \
  SPARSE_INT8_INT8_W1_V8_LEAKY_RELU                                         \
  SPARSE_INT8_INT8_W1_V8_HARD_SWISH                                         \
  "ld1    {v8.4s},   [%[vmax]]  \n" /* v8 = -127 */ /* data >= -127 */      \
  "fcmge v1.4s, v0.4s, v8.4s   \n"                  /* choose data */       \
  "bif v0.16b,  v8.16b, v1.16b \n"                                          \
  "fcvtas v1.4s, v0.4s\n"                         /*  cvt to int */         \
  "sqxtn  v7.4h, v1.4s\n"                         /*  cvt int32 to int16 */ \
  "sqxtn  v0.8b, v7.8h\n" /* cvt int16 to int8 */ /* store result */        \
  "st1   {v0.b}[0],  [%[c_ptr]]\n"

#define PARAM1                                                        \
  [a_ptr] "+r"(cur_w), [b_ptr] "+r"(cur_b), [c_ptr] "+r"(cur_output), \
      [k] "+r"(nnz), [widx_dmap] "+r"(dmap)

#define PARAM2                                                         \
  [vscale] "r"(vsclae), [vbias] "r"(vbias), [vflag_act] "r"(flag_act), \
      [valpha] "r"(alpha), [vmax] "r"(vmax), [hs_param] "r"(hs_param)

#define ASM_PARAM                                                           \
  "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", \
      "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v21", "v22", \
      "v23", "v24", "v25", "v26", "v27", "v28", "v30", "v31", "w0", "w1",   \
      "x1", "cc", "memory"

#define INIT_PTR_1x1_SPMM_INT8(i)                                            \
  int8_t* cur_output =                                                       \
      reinterpret_cast<int8_t*>((uintptr_t)output + output_stride * i);      \
  const int8_t* cur_w = A;                                                   \
  uint32_t nnz = nidx_nnzmap[i];                                             \
  const int8_t* cur_b = B;                                                   \
  const int32_t* dmap = widx_dmap;                                           \
  if (i != 0) {                                                              \
    cur_w = A + nidx_nnzmap[i - 1];                                          \
    nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];                               \
    cur_b +=                                                                 \
        ((nidx_nnzmap[i - 1] == 0) ? 0 : widx_dmap[nidx_nnzmap[i - 1] - 1]); \
    dmap = widx_dmap + nidx_nnzmap[i - 1];                                   \
  }                                                                          \
  float vsclae = scale[i];                                                   \
  float vbias = (bias != nullptr) ? bias[i] : 0.0;

#define REDEFINE_PTR_SPMM_INT8  \
  uint32_t nnz = nnz0;          \
  const int32_t* dmap = dmap0;  \
  const int8_t* cur_w = cur_w0; \
  const int8_t* cur_b = cur_b0; \
  int8_t* cur_output = cur_output0;

/**
 * \brief Sparse calculation implementation of 1x1 convolution, both input and
 * output are int8.
 * Sparse matrix multiplication is calculated in blocks, the block size is Mx48,
 * that is,
 * the parameter matrix is MxK, and the activation matrix is Kx48; when N is
 * less than 48,
 * it is calculated in blocks of Mx32, Mx16, Mx8, and Mx4 in turn;
 * @param A sparse weight data
 * @param B dense input data
 * @param widx_dmap An array of int32_t values storing scaled [by sizeof(input
 * element)] difference
 * between input channels corresponding to successive non-zero element
 * @param nidx_nnzmap the number of non-zero kernel elements per each output
 * channel
 * @param bias
 * @param output
 * @param M
 * @param N
 * @param K
 * @param param
 * @param ctx
 */

static void sparse_conv_int8_int8_mxn_pipelined(
    const int8_t* A,
    const int8_t* B,
    const int32_t* widx_dmap,
    const uint32_t* nidx_nnzmap,
    const float* bias,
    const float* scale,
    int8_t* output,
    int M,
    int K,
    int N,
    const operators::SparseConvParam& param,
    ARMContext* ctx) {
  auto act_param = param.activation_param;
  auto act_type = act_param.active_type;
  volatile float alpha = 0.f;
  float hs_param[12] = {0.f};
  int flag_act = 0x00;  // relu: 1, relu6: 2, leakey: 3
  if (act_param.has_active) {
    if (act_type == lite_api::ActivationType::kRelu) {
      flag_act = 0x01;
    } else if (act_type == lite_api::ActivationType::kRelu6) {
      flag_act = 0x02;
      alpha = act_param.Relu_clipped_coef;
    } else if (act_type == lite_api::ActivationType::kLeakyRelu) {
      flag_act = 0x03;
      alpha = act_param.Leaky_relu_alpha;
    } else if (act_type == lite_api::ActivationType::kHardSwish) {
      flag_act = 0x04;
      for (int i = 0; i < 4; i++) {
        hs_param[i] = act_param.hard_swish_offset / param.output_scale;
        hs_param[i + 4] = 1.0 / act_param.hard_swish_scale;
        hs_param[i + 8] = act_param.hard_swish_threshold / param.output_scale;
      }
    }
  }

  int flag_bias = (bias != nullptr) ? 1 : 0;
  size_t mc = N * sizeof(int8_t);
  size_t nc = M;
  size_t output_stride = N * sizeof(int8_t);
  float vmax[4] = {-127.0, -127.0, -127.0, -127.0};

  while
    SPARSE_LIKELY(mc >= 64 * sizeof(int8_t)) {
      LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
        INIT_PTR_1x1_SPMM_INT8(i)
            // clang-format off
          asm volatile(SPARSE_INT8_INT8_W64_V8_OUT
            : PARAM1
            : PARAM2
            : ASM_PARAM);
        // clang-format on
      }
      LITE_PARALLEL_COMMON_END();
      output =
          reinterpret_cast<int8_t*>((uintptr_t)output + 64 * sizeof(int8_t));
      B += 64;
      mc -= 64 * sizeof(int8_t);
    }

  if
    SPARSE_UNLIKELY(mc != 0) {
      if (mc >= (48 * sizeof(int8_t))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          INIT_PTR_1x1_SPMM_INT8(i)
              // clang-format off
            asm volatile(SPARSE_INT8_INT8_W48_V8_OUT
              : PARAM1
              : PARAM2
              : ASM_PARAM);
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<int8_t*>((uintptr_t)output + 48 * sizeof(int8_t));
        B += 48;
        mc -= 48 * sizeof(int8_t);
      }

      if (mc >= 32) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          INIT_PTR_1x1_SPMM_INT8(i)
              // clang-format off
            asm volatile(SPARSE_INT8_INT8_W32_V8_OUT
              : PARAM1
              : PARAM2
              : ASM_PARAM);
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<int8_t*>((uintptr_t)output + 32 * sizeof(int8_t));
        B += 32;
        mc -= 32 * sizeof(int8_t);
      }

      if (mc >= 16) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          INIT_PTR_1x1_SPMM_INT8(i)
              // clang-format off
            asm volatile(SPARSE_INT8_INT8_W16_V8_OUT
              :PARAM1
              : PARAM2
              : ASM_PARAM);
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<int8_t*>((uintptr_t)output + 16 * sizeof(int8_t));
        B += 16;
        mc -= 16 * sizeof(int8_t);
      }

      if (mc >= 8) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          INIT_PTR_1x1_SPMM_INT8(i)
              // clang-format off
            asm volatile(SPARSE_INT8_INT8_W8_V8_OUT
              : PARAM1
              : PARAM2
              : ASM_PARAM);
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<int8_t*>((uintptr_t)output + 8 * sizeof(int8_t));
        B += 8;
        mc -= 8;
      }

      if ((mc >= 4)) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          INIT_PTR_1x1_SPMM_INT8(i)
              // clang-format off
            asm volatile(SPARSE_INT8_INT8_W4_V8_OUT
              : PARAM1
              : PARAM2
              : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v11", "v16", "v21", 
              "w1", "w2", "w3", "w4", "w5", "x1", "cc", "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<int8_t*>((uintptr_t)output + 4 * sizeof(int8_t));
        B += 4;
        mc -= 4 * sizeof(int8_t);
      }

      int m = mc;
      for (; m > 0; m--) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          INIT_PTR_1x1_SPMM_INT8(i) uint32_t pair_num = nnz / 8;
          uint32_t lave_num = nnz % 8;
          // clang-format off
            asm volatile(SPARSE_INT8_INT8_W1_V8_OUT
            : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap)
            : [vscale] "r"(vsclae),
                [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha),
                [vmax] "r"(vmax),
                [hs_param] "r"(hs_param)
            : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "x1", "cc", "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output = reinterpret_cast<int8_t*>((uintptr_t)output + sizeof(int8_t));
        B += 1;
        mc -= 1;
      }
    }
}
static void sparse_conv_int8_int8_nxm_pipelined(
    const int8_t* A,
    const int8_t* B,
    const int32_t* widx_dmap,
    const uint32_t* nidx_nnzmap,
    const float* bias,
    const float* scale,
    int8_t* output,
    int M,
    int K,
    int N,
    const operators::SparseConvParam& param,
    ARMContext* ctx) {
  auto act_param = param.activation_param;
  auto act_type = act_param.active_type;
  volatile float alpha = 0.f;
  float hs_param[12] = {0.f};
  int flag_act = 0x00;  // relu: 1, relu6: 2, leakey: 3
  if (act_param.has_active) {
    if (act_type == lite_api::ActivationType::kRelu) {
      flag_act = 0x01;
    } else if (act_type == lite_api::ActivationType::kRelu6) {
      flag_act = 0x02;
      alpha = act_param.Relu_clipped_coef;
    } else if (act_type == lite_api::ActivationType::kLeakyRelu) {
      flag_act = 0x03;
      alpha = act_param.Leaky_relu_alpha;
    } else if (act_type == lite_api::ActivationType::kHardSwish) {
      flag_act = 0x04;
      for (int i = 0; i < 4; i++) {
        hs_param[i] = act_param.hard_swish_offset / param.output_scale;
        hs_param[i + 4] = 1.0 / act_param.hard_swish_scale;
        hs_param[i + 8] = act_param.hard_swish_threshold / param.output_scale;
      }
    }
  }

  int flag_bias = (bias != nullptr) ? 1 : 0;
  size_t mc = N * sizeof(int8_t);
  size_t nc = M;
  size_t output_stride = N * sizeof(int8_t);
  float vmax[4] = {-127.0, -127.0, -127.0, -127.0};

  LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
    int8_t* cur_output0 =
        reinterpret_cast<int8_t*>((uintptr_t)output + output_stride * i);
    const int8_t* cur_w0 = A;
    volatile uint32_t nnz0 = nidx_nnzmap[i];
    const int8_t* cur_b0 = B;
    const int32_t* dmap0 = widx_dmap;
    if (i != 0) {
      int nidx = nidx_nnzmap[i - 1];
      cur_w0 = A + nidx;
      nnz0 = nidx_nnzmap[i] - nidx;
      cur_b0 += ((nidx == 0) ? 0 : widx_dmap[nidx - 1]);
      dmap0 = widx_dmap + nidx;
    }
    volatile float vsclae = scale[i];
    volatile float vbias = (bias != nullptr) ? bias[i] : 0.0;

    int mc0 = mc;
    while
      SPARSE_LIKELY(mc0 >= 64 * sizeof(int8_t)) {
        REDEFINE_PTR_SPMM_INT8
        // clang-format off
        asm volatile(SPARSE_INT8_INT8_W64_V8_OUT
          : PARAM1
          : PARAM2
          : ASM_PARAM);
        // clang-format on
        cur_output0 += 64;
        cur_b0 += 64;
        mc0 -= 64 * sizeof(int8_t);
      }

    if
      SPARSE_UNLIKELY(mc != 0) {
        if (mc0 >= 48) {
          REDEFINE_PTR_SPMM_INT8
          // clang-format off
        asm volatile(SPARSE_INT8_INT8_W48_V8_OUT
          : PARAM1
          : PARAM2
          : ASM_PARAM);
          // clang-format on
          mc0 -= 48;
          cur_b0 += 48;
          cur_output0 += 48;
        }

        if (mc0 >= 32) {
          REDEFINE_PTR_SPMM_INT8
          // clang-format off
        asm volatile(SPARSE_INT8_INT8_W32_V8_OUT
          : PARAM1
          : PARAM2
          : ASM_PARAM);
          // clang-format on
          mc0 -= 32;
          cur_b0 += 32;
          cur_output0 += 32;
        }

        if (mc0 >= 16) {
          REDEFINE_PTR_SPMM_INT8
          // clang-format off
        asm volatile(SPARSE_INT8_INT8_W16_V8_OUT
          : PARAM1
          : PARAM2
          : ASM_PARAM);
          // clang-format on
          mc0 -= 16;
          cur_b0 += 16;
          cur_output0 += 16;
        }

        if (mc0 >= 8) {
          REDEFINE_PTR_SPMM_INT8
          // clang-format off
        asm volatile(SPARSE_INT8_INT8_W8_V8_OUT
          : PARAM1
          : PARAM2
          : ASM_PARAM);
          // clang-format on
          mc0 -= 8;
          cur_b0 += 8;
          cur_output0 += 8;
        }

        if ((mc0 >= 4)) {
          REDEFINE_PTR_SPMM_INT8
          // clang-format off
        asm volatile(SPARSE_INT8_INT8_W4_V8_OUT
          : PARAM1
          : PARAM2
          : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v11", "v16", "v21", 
          "w1", "w2", "w3", "w4", "w5", "x1", "cc", "memory");
          // clang-format on
          mc0 -= 4;
          cur_b0 += 4;
          cur_output0 += 4;
        }

        int m = mc0;
        for (; m > 0; m--) {
          REDEFINE_PTR_SPMM_INT8

          uint32_t pair_num = nnz / 8;
          uint32_t lave_num = nnz % 8;
          // clang-format off
        asm volatile(SPARSE_INT8_INT8_W1_V8_OUT
        : [a_ptr] "+r"(cur_w),
            [b_ptr] "+r"(cur_b),
            [c_ptr] "+r"(cur_output),
            [k] "+r"(nnz),
            [n] "+r"(pair_num),
            [m] "+r"(lave_num),
            [widx_dmap] "+r"(dmap)
        : [vscale] "r"(vsclae),
            [vbias] "r"(vbias),
            [vflag_act] "r"(flag_act),
            [valpha] "r"(alpha),
            [vmax] "r"(vmax),
            [hs_param] "r"(hs_param)
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "x1", "cc", "memory");
          // clang-format on
          mc0 -= 1;
          cur_b0 += 1;
          cur_output0 += 1;
        }
      }
  }
  LITE_PARALLEL_COMMON_END();
}
void sparse_conv_int8_int8_pipelined(const int8_t* A,
                                     const int8_t* B,
                                     const int32_t* widx_dmap,
                                     const uint32_t* nidx_nnzmap,
                                     const float* bias,
                                     const float* scale,
                                     int8_t* output,
                                     int M,
                                     int K,
                                     int N,
                                     const operators::SparseConvParam& param,
                                     ARMContext* ctx) {
  if (M * N > MAX_CACHE_SIZE) {
    sparse_conv_int8_int8_mxn_pipelined(
        A, B, widx_dmap, nidx_nnzmap, bias, scale, output, M, K, N, param, ctx);

  } else {
    sparse_conv_int8_int8_nxm_pipelined(
        A, B, widx_dmap, nidx_nnzmap, bias, scale, output, M, K, N, param, ctx);
  }
}

#else  // armv7

#define SPARSE_F32_F32_W48_v7_KERNEL \
  "vdup.32    q4,    %[vbias]\n"     \
  "vdup.32    q5,    d8[0]\n"        \
  "vdup.32    q6,    d8[0]\n"        \
  "pld  [%[a_ptr], #128]    \n"      \
  "vdup.32    q7,    d8[0]\n"        \
  "vdup.32    q8,    d8[0]\n"        \
  "vdup.32    q9,    d8[0]\n"        \
  "pld  [%[widx_dmap], #128]    \n"  \
  "vdup.32    q10,   d8[0]\n"        \
  "vdup.32    q11,   d8[0]\n"        \
  "vdup.32    q12,   d8[0]\n"        \
  "pld  [%[b_ptr], #192]    \n"      \
  "vdup.32    q13,   d8[0]\n"        \
  "vdup.32    q14,   d8[0]\n"        \
  "vdup.32    q15,   d8[0]\n"        \
  "cmp    %[k],    #0\n"             \
  "beq    1f\n" /* main loop*/       \
  "0:\n"                             \
  "ldr   r0, [%[a_ptr]], #4\n"       \
  "mov   r2,   %[b_ptr]\n"           \
  "vdup.32    q0,   r0\n"            \
  "vld1.32  {d2-d5}, [r2]\n"         \
  "vmla.f32    q4,   q1,  q0\n"      \
  "pld  [%[widx_dmap], #128]    \n"  \
  "vmla.f32    q5,   q2,  q0\n"      \
  "add  r2,  r2,   #32\n"            \
  "vld1.32  {d2-d5}, [r2]\n"         \
  "vmla.f32    q6,  q1,  q0\n"       \
  "vmla.f32    q7,  q2,  q0\n"       \
  "add  r2,  r2,   #32\n"            \
  "vld1.32  {d2-d5}, [r2]\n"         \
  "vmla.f32    q8,  q1,  q0\n"       \
  "vmla.f32    q9,  q2,  q0\n"       \
  "ldr   r0, [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], r0\n"  \
  "subs    %[k],   %[k],   #1\n"     \
  "add  r2,  r2,   #32\n"            \
  "vld1.32  {d2-d5}, [r2]\n"         \
  "vmla.f32    q10,  q1,  q0\n"      \
  "vmla.f32    q11,  q2,  q0\n"      \
  "pld  [%[b_ptr], #192]    \n"      \
  "add  r2,  r2,   #32\n"            \
  "vld1.32  {d2-d5}, [r2]\n"         \
  "vmla.f32    q12,  q1,  q0\n"      \
  "vmla.f32    q13,  q2,  q0\n"      \
  "pld  [%[a_ptr], #128]    \n"      \
  "add  r2,  r2,   #32\n"            \
  "vld1.32  {d2-d5}, [r2]\n"         \
  "vmla.f32    q14,  q1,  q0\n"      \
  "vmla.f32    q15,  q2,  q0\n"      \
  "bne     0b\n"                     \
  "1:\n"

#define SPARSE_F32_F32_W48_v7_RELU               \
  /* do relu */                                  \
  "cmp    %[vflag_act],   #0\n" /* skip relu */  \
  "beq   9f                 \n" /* no act end */ \
  "cmp    %[vflag_act],   #1\n" /* skip relu */  \
  "bne   10f                \n" /* other act */  \
  "vmov.i32   q0, #0\n"         /* for relu */   \
  "vmax.f32   q4,   q4,   q0\n" /* relu */       \
  "vmax.f32   q5,   q5,   q0\n" /* relu */       \
  "vmax.f32   q6,   q6,   q0\n" /* relu */       \
  "vmax.f32   q7,   q7,   q0\n" /* relu */       \
  "vmax.f32   q8,   q8,   q0\n" /* relu */       \
  "vmax.f32   q9,   q9,   q0\n" /* relu */       \
  "vmax.f32   q10,  q10,  q0\n" /* relu */       \
  "vmax.f32   q11,  q11,  q0\n" /* relu */       \
  "vmax.f32   q12,  q12,  q0\n" /* relu */       \
  "vmax.f32   q13,  q13,  q0\n" /* relu */       \
  "vmax.f32   q14,  q14,  q0\n" /* relu */       \
  "vmax.f32   q15,  q15,  q0\n" /* relu */       \
  "b      9f                \n" /* relu end */

#define SPARSE_F32_F32_W48_v7_RELU6                    \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q4,   q4,   q0\n"      /* relu6 */       \
  "vmax.f32   q5,   q5,   q0\n"      /* relu6 */       \
  "vmax.f32   q6,   q6,   q0\n"      /* relu6 */       \
  "vmax.f32   q7,   q7,   q0\n"      /* relu6 */       \
  "vmax.f32   q8,   q8,   q0\n"      /* relu6 */       \
  "vmax.f32   q9,   q9,   q0\n"      /* relu6 */       \
  "vmax.f32   q10,  q10,  q0\n"      /* relu6 */       \
  "vmax.f32   q11,  q11,  q0\n"      /* relu6 */       \
  "vmax.f32   q12,  q12,  q0\n"      /* relu6 */       \
  "vmax.f32   q13,  q13,  q0\n"      /* relu6 */       \
  "vmax.f32   q14,  q14,  q0\n"      /* relu6 */       \
  "vmax.f32   q15,  q15,  q0\n"      /* relu6 */       \
  "vmin.f32   q4,   q4,   q1\n"      /* relu6 */       \
  "vmin.f32   q5,   q5,   q1\n"      /* relu6 */       \
  "vmin.f32   q6,   q6,   q1\n"      /* relu6 */       \
  "vmin.f32   q7,   q7,   q1\n"      /* relu6 */       \
  "vmin.f32   q8,   q8,   q1\n"      /* relu6 */       \
  "vmin.f32   q9,   q9,   q1\n"      /* relu6 */       \
  "vmin.f32   q10,  q10,  q1\n"      /* relu6 */       \
  "vmin.f32   q11,  q11,  q1\n"      /* relu6 */       \
  "vmin.f32   q12,  q12,  q1\n"      /* relu6 */       \
  "vmin.f32   q13,  q13,  q1\n"      /* relu6 */       \
  "vmin.f32   q14,  q14,  q1\n"      /* relu6 */       \
  "vmin.f32   q15,  q15,  q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_F32_F32_W48_v7_LEAKY_RELU                       \
  /* do relu */                                                \
  "11: \n"                                                     \
  "cmp   %[vflag_act],  #3       \n"   /* check leakey relu */ \
  "bne   12f                     \n"   /* no act end */        \
  "vmov.i32   q0, #0\n"                /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"        /* leakey relu alpha */ \
  "vcge.f32   q2,    q4,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q4,    q1     \n" /* vmulq_f32 */         \
  "vbif       q4,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q5,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q5,    q1     \n" /* vmulq_f32 */         \
  "vbif       q5,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q6,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q6,    q1     \n" /* vmulq_f32 */         \
  "vbif       q6,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q7,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q7,    q1     \n" /* vmulq_f32 */         \
  "vbif       q7,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q8,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q8,    q1     \n" /* vmulq_f32 */         \
  "vbif       q8,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q9,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q9,    q1     \n" /* vmulq_f32 */         \
  "vbif       q9,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q10,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q10,    q1    \n" /* vmulq_f32 */         \
  "vbif       q10,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q11,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q11,    q1    \n" /* vmulq_f32 */         \
  "vbif       q11,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q12,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q12,    q1    \n" /* vmulq_f32 */         \
  "vbif       q12,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q13,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q13,    q1    \n" /* vmulq_f32 */         \
  "vbif       q13,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q14,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q14,    q1    \n" /* vmulq_f32 */         \
  "vbif       q14,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q15,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q15,    q1    \n" /* vmulq_f32 */         \
  "vbif       q15,    q3,    q2    \n"                         \
  "b      9f                    \n"

#define SPARSE_F32_F32_W48_v7_HARD_SWISH                              \
  /* do relu */                                                       \
  "12: \n"                                                            \
  "vld1.f32   {d0-d3}, [%[hs_param]]!      @ load hard swish alpha\n" \
  "vadd.f32   q3, q4, q0                \n"                           \
  "vmul.f32   q4, q4, q1                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]       \n"                        \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q4, q4, q3                \n"                           \
  "vadd.f32   q3, q5, q0                \n"                           \
  "vmul.f32   q5, q5, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]       \n"                        \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q5, q5, q3                \n"                           \
  "vadd.f32   q3, q6, q0                \n"                           \
  "vmul.f32   q6, q6, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]       \n"                        \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q6, q6, q3                \n"                           \
  "vadd.f32   q3, q7, q0                \n"                           \
  "vmul.f32   q7, q7, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q7, q7, q3                \n"                           \
  "vadd.f32   q3, q8, q0                \n"                           \
  "vmul.f32   q8, q8, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q8, q8, q3                \n"                           \
  "vadd.f32   q3, q9, q0                \n"                           \
  "vmul.f32   q9, q9, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q9, q9, q3                \n"                           \
  "vadd.f32   q3, q10, q0               \n"                           \
  "vmul.f32   q10, q10, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q10, q10, q3              \n"                           \
  "vadd.f32   q3, q11, q0               \n"                           \
  "vmul.f32   q11, q11, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q11, q11, q3              \n"                           \
  "vadd.f32   q3, q12, q0               \n"                           \
  "vmul.f32   q12, q12, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q12, q12, q3              \n"                           \
  "vadd.f32   q3, q13, q0               \n"                           \
  "vmul.f32   q13, q13, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q13, q13, q3              \n"                           \
  "vadd.f32   q3, q14, q0               \n"                           \
  "vmul.f32   q14, q14, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q14, q14, q3              \n"                           \
  "vadd.f32   q3, q15, q0               \n"                           \
  "vmul.f32   q15, q15, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q15, q15, q3              \n"                           \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx48, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx48, and the required data is
 * MxKxKx48.
 */
#define SPARSE_F32_F32_W48_v7_OUT                                  \
  SPARSE_F32_F32_W48_v7_KERNEL SPARSE_F32_F32_W48_v7_RELU          \
      SPARSE_F32_F32_W48_v7_RELU6 SPARSE_F32_F32_W48_v7_LEAKY_RELU \
          SPARSE_F32_F32_W48_v7_HARD_SWISH                         \
      "mov   r2,   %[c_ptr]\n" /* store result */                  \
      "vst1.32   {d8-d11},  [r2]\n"                                \
      "add  r2,  r2,   #32\n"                                      \
      "vst1.32   {d12-d15},  [r2]\n"                               \
      "add  r2,  r2,   #32\n"                                      \
      "vst1.32   {d16-d19},  [r2]\n"                               \
      "add  r2,  r2,   #32\n"                                      \
      "vst1.32   {d20-d23},  [r2]\n"                               \
      "add  r2,  r2,   #32\n"                                      \
      "vst1.32   {d24-d27},  [r2]\n"                               \
      "add  r2,  r2,   #32\n"                                      \
      "vst1.32   {d28-d31},  [r2]\n"

#define SPARSE_F32_F32_W32_v7_KERNEL \
  "vdup.32    q8,   %[vbias]\n"      \
  "vdup.32    q9,   d16[0]\n"        \
  "pld  [%[a_ptr], #128]    \n"      \
  "vdup.32    q10,  d16[0]\n"        \
  "vdup.32    q11,  d16[0]\n"        \
  "pld  [%[widx_dmap], #128]    \n"  \
  "vdup.32    q12,  d16[0]\n"        \
  "vdup.32    q13,  d16[0]\n"        \
  "pld  [%[b_ptr], #128]    \n"      \
  "vdup.32    q14,  d16[0]\n"        \
  "vdup.32    q15,  d16[0]\n"        \
  "cmp    %[k],    #0\n"             \
  "beq    1f\n" /* main loop*/       \
  "0:\n"                             \
  "ldr   r0, [%[a_ptr]], #4\n"       \
  "mov   r2,   %[b_ptr]\n"           \
  "vdup.32    q0,   r0\n"            \
  "vld1.32  {d2-d5}, [r2]\n"         \
  "add  r2,  r2,   #32\n"            \
  "vld1.32  {d6-d9}, [r2]\n"         \
  "vmla.f32    q8,   q1,  q0\n"      \
  "pld  [%[widx_dmap], #128]    \n"  \
  "vmla.f32    q9,   q2,  q0\n"      \
  "vmla.f32    q10,  q3,  q0\n"      \
  "vmla.f32    q11,  q4,  q0\n"      \
  "ldr   r0, [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], r0\n"  \
  "subs    %[k],   %[k],   #1\n"     \
  "add  r2,  r2,   #32\n"            \
  "vld1.32  {d2-d5}, [r2]\n"         \
  "pld  [%[b_ptr], #128]    \n"      \
  "add  r2,  r2,   #32\n"            \
  "vld1.32  {d6-d9}, [r2]\n"         \
  "vmla.f32    q12,  q1,  q0\n"      \
  "pld  [%[a_ptr], #128]    \n"      \
  "vmla.f32    q13,  q2,  q0\n"      \
  "vmla.f32    q14,  q3,  q0\n"      \
  "vmla.f32    q15,  q4,  q0\n"      \
  "bne     0b\n"                     \
  "1:\n"

#define SPARSE_F32_F32_W32_v7_RELU               \
  /* do relu */                                  \
  "cmp    %[vflag_act],   #0\n" /* skip relu */  \
  "beq   9f                 \n" /* no act end */ \
  "cmp    %[vflag_act],   #1\n" /* skip relu */  \
  "bne   10f                \n" /* other act */  \
  "vmov.i32   q0, #0\n"         /* for relu */   \
  "vmax.f32   q8,   q8,   q0\n" /* relu */       \
  "vmax.f32   q9,   q9,   q0\n" /* relu */       \
  "vmax.f32   q10,  q10,  q0\n" /* relu */       \
  "vmax.f32   q11,  q11,  q0\n" /* relu */       \
  "vmax.f32   q12,  q12,  q0\n" /* relu */       \
  "vmax.f32   q13,  q13,  q0\n" /* relu */       \
  "vmax.f32   q14,  q14,  q0\n" /* relu */       \
  "vmax.f32   q15,  q15,  q0\n" /* relu */       \
  "b      9f                \n" /* relu end */

#define SPARSE_F32_F32_W32_v7_RELU6                    \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q8,   q8,   q0\n"      /* relu6 */       \
  "vmax.f32   q9,   q9,   q0\n"      /* relu6 */       \
  "vmax.f32   q10,  q10,  q0\n"      /* relu6 */       \
  "vmax.f32   q11,  q11,  q0\n"      /* relu6 */       \
  "vmax.f32   q12,  q12,  q0\n"      /* relu6 */       \
  "vmax.f32   q13,  q13,  q0\n"      /* relu6 */       \
  "vmax.f32   q14,  q14,  q0\n"      /* relu6 */       \
  "vmax.f32   q15,  q15,  q0\n"      /* relu6 */       \
  "vmin.f32   q8,   q8,   q1\n"      /* relu6 */       \
  "vmin.f32   q9,   q9,   q1\n"      /* relu6 */       \
  "vmin.f32   q10,  q10,  q1\n"      /* relu6 */       \
  "vmin.f32   q11,  q11,  q1\n"      /* relu6 */       \
  "vmin.f32   q12,  q12,  q1\n"      /* relu6 */       \
  "vmin.f32   q13,  q13,  q1\n"      /* relu6 */       \
  "vmin.f32   q14,  q14,  q1\n"      /* relu6 */       \
  "vmin.f32   q15,  q15,  q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_F32_F32_W32_v7_LEAKY_RELU                      \
  /* do relu */                                               \
  "11: \n"                                                    \
  "cmp   %[vflag_act],  #3       \n"  /* check leakey relu */ \
  "bne   12f                     \n"  /* no act end */        \
  "vmov.i32   q0, #0\n"               /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"       /* leakey relu alpha */ \
  "vcge.f32   q2,    q8,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q8,    q1    \n" /* vmulq_f32 */         \
  "vcge.f32   q4,    q9,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q5,    q9,    q1    \n" /* vmulq_f32 */         \
  "vcge.f32   q6,    q10,   q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q7,    q10,   q1    \n" /* vmulq_f32 */         \
  "vbif       q8,    q3,    q2    \n"                         \
  "vbif       q9,    q5,    q4    \n"                         \
  "vbif       q10,   q7,    q6    \n"                         \
  "vcge.f32   q2,    q11,   q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q11,   q1    \n" /* vmulq_f32 */         \
  "vcge.f32   q4,    q12,   q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q5,    q12,   q1    \n" /* vmulq_f32 */         \
  "vcge.f32   q6,    q13,   q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q7,    q13,   q1    \n" /* vmulq_f32 */         \
  "vbif       q11,   q3,    q2    \n"                         \
  "vbif       q12,   q5,    q4    \n"                         \
  "vbif       q13,   q7,    q6    \n"                         \
  "vcge.f32   q2,    q14,   q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q14,   q1    \n" /* vmulq_f32 */         \
  "vcge.f32   q4,    q15,   q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q5,    q15,   q1    \n" /* vmulq_f32 */         \
  "vbif       q14,   q3,    q2    \n"                         \
  "vbif       q15,   q5,    q4    \n"                         \
  "b      9f                    \n"

#define SPARSE_F32_F32_W32_v7_HARD_SWISH       \
  /* do relu */                                \
  "12: \n"                                     \
  "vmov.u32   q0,   #0                     \n" \
  "vld1.f32   {d2-d5}, [%[hs_param]]!      \n" \
  "vld1.f32   {d6-d7}, [%[hs_param]]       \n" \
  "vadd.f32   q4, q8, q1                \n"    \
  "vadd.f32   q6, q9, q1                \n"    \
  "vmul.f32   q5, q8, q2                \n"    \
  "vmul.f32   q7, q9, q2                \n"    \
  "vmax.f32   q4, q4, q0                \n"    \
  "vmax.f32   q6, q6, q0                \n"    \
  "vmin.f32   q4, q4, q3                \n"    \
  "vmin.f32   q6, q6, q3                \n"    \
  "vmul.f32   q8, q5, q4                \n"    \
  "vmul.f32   q9, q7, q6                \n"    \
  "vadd.f32   q4, q10, q1                \n"   \
  "vadd.f32   q6, q11, q1                \n"   \
  "vmul.f32   q5, q10, q2                \n"   \
  "vmul.f32   q7, q11, q2                \n"   \
  "vmax.f32   q4, q4, q0                \n"    \
  "vmax.f32   q6, q6, q0                \n"    \
  "vmin.f32   q4, q4, q3                \n"    \
  "vmin.f32   q6, q6, q3                \n"    \
  "vmul.f32   q10, q5, q4                \n"   \
  "vmul.f32   q11, q7, q6                \n"   \
  "vadd.f32   q4, q12, q1                \n"   \
  "vadd.f32   q6, q13, q1                \n"   \
  "vmul.f32   q5, q12, q2                \n"   \
  "vmul.f32   q7, q13, q2                \n"   \
  "vmax.f32   q4, q4, q0                \n"    \
  "vmax.f32   q6, q6, q0                \n"    \
  "vmin.f32   q4, q4, q3                \n"    \
  "vmin.f32   q6, q6, q3                \n"    \
  "vmul.f32   q12, q5, q4                \n"   \
  "vmul.f32   q13, q7, q6                \n"   \
  "vadd.f32   q4, q14, q1                \n"   \
  "vadd.f32   q6, q15, q1                \n"   \
  "vmul.f32   q5, q14, q2                \n"   \
  "vmul.f32   q7, q15, q2                \n"   \
  "vmax.f32   q4, q4, q0                \n"    \
  "vmax.f32   q6, q6, q0                \n"    \
  "vmin.f32   q4, q4, q3                \n"    \
  "vmin.f32   q6, q6, q3                \n"    \
  "vmul.f32   q14, q5, q4                \n"   \
  "vmul.f32   q15, q7, q6                \n"   \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx32, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx32, and the required data is
 * MxKxKx32.
 */
#define SPARSE_F32_F32_W32_v7_OUT                                  \
  SPARSE_F32_F32_W32_v7_KERNEL SPARSE_F32_F32_W32_v7_RELU          \
      SPARSE_F32_F32_W32_v7_RELU6 SPARSE_F32_F32_W32_v7_LEAKY_RELU \
          SPARSE_F32_F32_W32_v7_HARD_SWISH                         \
      "mov   r2,   %[c_ptr]\n" /* store result */                  \
      "vst1.32   {d16-d19},  [r2]\n"                               \
      "add  r2,  r2,   #32\n"                                      \
      "vst1.32   {d20-d23},  [r2]\n"                               \
      "add  r2,  r2,   #32\n"                                      \
      "vst1.32   {d24-d27},  [r2]\n"                               \
      "add  r2,  r2,   #32\n"                                      \
      "vst1.32   {d28-d31},  [r2]\n"

#define SPARSE_F32_F32_W16_v7_KERNEL \
  "vdup.32    q8,   %[vbias]\n"      \
  "pld  [%[a_ptr], #128]    \n"      \
  "vdup.32    q9,   d16[0]\n"        \
  "pld  [%[widx_dmap], #128]    \n"  \
  "vdup.32    q10,  d16[0]\n"        \
  "pld  [%[b_ptr], #128]    \n"      \
  "vdup.32    q11,  d16[0]\n"        \
  "cmp    %[k],    #0\n"             \
  "beq    1f\n" /* main loop*/       \
  "0:\n"                             \
  "ldr   r0, [%[a_ptr]], #4\n"       \
  "subs    %[k],   %[k],   #1\n"     \
  "mov   r2,   %[b_ptr]\n"           \
  "pld  [%[widx_dmap], #128]    \n"  \
  "vdup.32    q0,   r0\n"            \
  "vld1.32  {d2-d5}, [r2]\n"         \
  "add  r2,  r2,   #32\n"            \
  "vld1.32  {d6-d9}, [r2]\n"         \
  "ldr   r0, [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], r0\n"  \
  "vmla.f32    q8,   q1,  q0\n"      \
  "vmla.f32    q9,   q2,  q0\n"      \
  "pld  [%[b_ptr], #128]    \n"      \
  "vmla.f32    q10,  q3,  q0\n"      \
  "vmla.f32    q11,  q4,  q0\n"      \
  "bne     0b\n"                     \
  "1:\n"

#define SPARSE_F32_F32_W16_v7_RELU                    \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q0, #0\n"              /* for relu */   \
  "vmax.f32   q8,   q8,   q0\n"      /* relu */       \
  "vmax.f32   q9,   q9,   q0\n"      /* relu */       \
  "vmax.f32   q10,  q10,  q0\n"      /* relu */       \
  "vmax.f32   q11,  q11,  q0\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_F32_F32_W16_v7_RELU6                    \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q8,   q8,   q0\n"      /* relu6 */       \
  "vmax.f32   q9,   q9,   q0\n"      /* relu6 */       \
  "vmax.f32   q10,  q10,  q0\n"      /* relu6 */       \
  "vmax.f32   q11,  q11,  q0\n"      /* relu6 */       \
  "vmin.f32   q8,   q8,   q1\n"      /* relu6 */       \
  "vmin.f32   q9,   q9,   q1\n"      /* relu6 */       \
  "vmin.f32   q10,  q10,  q1\n"      /* relu6 */       \
  "vmin.f32   q11,  q11,  q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_F32_F32_W16_v7_LEAKY_RELU                      \
  /* do relu */                                               \
  "11: \n"                                                    \
  "cmp   %[vflag_act],  #3       \n"  /* check leakey relu */ \
  "bne   12f                     \n"  /* no act end */        \
  "vmov.i32   q0, #0\n"               /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"       /* leakey relu alpha */ \
  "vcge.f32   q2,    q8,    q0   \n"  /* vcgeq_f32 */         \
  "vmul.f32   q3,    q8,    q1   \n"  /* vmulq_f32 */         \
  "vcge.f32   q4,    q9,    q0   \n"  /* vcgeq_f32 */         \
  "vmul.f32   q5,    q9,    q1   \n"  /* vmulq_f32 */         \
  "vcge.f32   q6,    q10,   q0   \n"  /* vcgeq_f32 */         \
  "vmul.f32   q7,    q10,   q1   \n"  /* vmulq_f32 */         \
  "vbif       q8,    q3,    q2   \n"  /* vmulq_f32 */         \
  "vbif       q9,    q5,    q4   \n"  /* vmulq_f32 */         \
  "vbif       q10,   q7,    q6   \n"  /* vmulq_f32 */         \
  "vcge.f32   q2,    q11,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q11,    q1   \n" /* vmulq_f32 */         \
  "vbif       q11,   q3,    q2   \n"  /* vmulq_f32 */         \
  "b      9f                    \n"

#define SPARSE_F32_F32_W16_v7_HARD_SWISH       \
  /* do relu */                                \
  "12: \n"                                     \
  "vmov.u32   q0,   #0                     \n" \
  "vld1.f32   {d2-d5}, [%[hs_param]]!      \n" \
  "vld1.f32   {d6-d7}, [%[hs_param]]       \n" \
  "vadd.f32   q4, q8, q1                \n"    \
  "vadd.f32   q6, q9, q1                \n"    \
  "vmul.f32   q5, q8, q2                \n"    \
  "vmul.f32   q7, q9, q2                \n"    \
  "vmax.f32   q4, q4, q0                \n"    \
  "vmax.f32   q6, q6, q0                \n"    \
  "vmin.f32   q4, q4, q3                \n"    \
  "vmin.f32   q6, q6, q3                \n"    \
  "vmul.f32   q8, q5, q4                \n"    \
  "vmul.f32   q9, q7, q6                \n"    \
  "vadd.f32   q4, q10, q1                \n"   \
  "vadd.f32   q6, q11, q1                \n"   \
  "vmul.f32   q5, q10, q2                \n"   \
  "vmul.f32   q7, q11, q2                \n"   \
  "vmax.f32   q4, q4, q0                \n"    \
  "vmax.f32   q6, q6, q0                \n"    \
  "vmin.f32   q4, q4, q3                \n"    \
  "vmin.f32   q6, q6, q3                \n"    \
  "vmul.f32   q10, q5, q4                \n"   \
  "vmul.f32   q11, q7, q6                \n"   \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx16, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx16, and the required data is
 * MxKxKx16.
 */
#define SPARSE_F32_F32_W16_v7_OUT                                  \
  SPARSE_F32_F32_W16_v7_KERNEL SPARSE_F32_F32_W16_v7_RELU          \
      SPARSE_F32_F32_W16_v7_RELU6 SPARSE_F32_F32_W16_v7_LEAKY_RELU \
          SPARSE_F32_F32_W16_v7_HARD_SWISH                         \
      "mov   r2,   %[c_ptr]\n" /* store result */                  \
      "vst1.32   {d16-d19},  [r2]\n"                               \
      "add  r2,  r2,   #32\n"                                      \
      "vst1.32   {d20-d23},  [r2]\n"

#define SPARSE_F32_F32_W8_v7_KERNEL \
  "vdup.32    q3,   %[vbias]\n"     \
  "vdup.32    q4,   d6[0]\n"        \
  "cmp    %[k],    #0\n"            \
  "beq    1f\n" /* main loop*/      \
  "0:\n"                            \
  "ldr   r0, [%[a_ptr]], #4\n"      \
  "vdup.32    q0,   r0\n"           \
  "vld1.32  {d2-d5}, [%[b_ptr]]\n"  \
  "vmla.f32    q3,   q1,  q0\n"     \
  "vmla.f32    q4,   q2,  q0\n"     \
  "ldr   r1, [%[widx_dmap]], #4\n"  \
  "add   %[b_ptr],  %[b_ptr], r1\n" \
  "subs    %[k],   %[k],   #1\n"    \
  "bne     0b\n"                    \
  "1:\n"

#define SPARSE_F32_F32_W8_v7_RELU                     \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q0, #0\n"              /* for relu */   \
  "vmax.f32   q3,   q3,   q0\n"      /* relu */       \
  "vmax.f32   q4,   q4,   q0\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_F32_F32_W8_v7_RELU6                     \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q3,   q3,   q0\n"      /* relu6 */       \
  "vmax.f32   q4,   q4,   q0\n"      /* relu6 */       \
  "vmin.f32   q3,   q3,   q1\n"      /* relu6 */       \
  "vmin.f32   q4,   q4,   q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_F32_F32_W8_v7_LEAKY_RELU                      \
  /* do relu */                                              \
  "11: \n"                                                   \
  "cmp   %[vflag_act],  #3       \n" /* check leakey relu */ \
  "bne   12f                     \n" /* no act end */        \
  "vmov.i32   q0, #0\n"              /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"      /* leakey relu alpha */ \
  "vcge.f32   q5,    q3,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q6,    q3,    q1   \n" /* vmulq_f32 */         \
  "vcge.f32   q7,    q4,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q8,    q4,    q1   \n" /* vmulq_f32 */         \
  "vbif       q3,    q6,    q5   \n" /* vmulq_f32 */         \
  "vbif       q4,    q8,    q7   \n" /* vmulq_f32 */         \
  "b      9f                    \n"

#define SPARSE_F32_F32_W8_v7_HARD_SWISH        \
  /* do relu */                                \
  "12: \n"                                     \
  "vmov.u32   q0,   #0                     \n" \
  "vld1.f32   {d2-d5}, [%[hs_param]]!      \n" \
  "vld1.f32   {d10-d11}, [%[hs_param]]     \n" \
  "vadd.f32   q6, q3, q1                \n"    \
  "vadd.f32   q8, q4, q1                \n"    \
  "vmul.f32   q7, q3, q2                \n"    \
  "vmul.f32   q9, q4, q2                \n"    \
  "vmax.f32   q6, q6, q0                \n"    \
  "vmax.f32   q8, q8, q0                \n"    \
  "vmin.f32   q6, q6, q5                \n"    \
  "vmin.f32   q8, q8, q5                \n"    \
  "vmul.f32   q3, q7, q6                \n"    \
  "vmul.f32   q4, q9, q8                \n"    \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx8, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx8, and the required data is
 * MxKxKx8.
 */
#define SPARSE_F32_F32_W8_v7_OUT                             \
  SPARSE_F32_F32_W8_v7_KERNEL SPARSE_F32_F32_W8_v7_RELU      \
      SPARSE_F32_F32_W8_v7_RELU6                             \
          SPARSE_F32_F32_W8_v7_LEAKY_RELU /* store result */ \
      SPARSE_F32_F32_W8_v7_HARD_SWISH "vst1.32   {d6-d9},  [%[c_ptr]]\n"

#define SPARSE_F32_F32_W4_v7_KERNEL \
  "vdup.32    q3,   %[vbias]\n"     \
  "cmp    %[k],    #0\n"            \
  "beq    1f\n" /* main loop*/      \
  "0:\n"                            \
  "ldr   r0, [%[a_ptr]], #4\n"      \
  "subs    %[k],   %[k],   #1\n"    \
  "vdup.32    q0,   r0\n"           \
  "vld1.32  {d2-d3}, [%[b_ptr]]\n"  \
  "vmla.f32    q3,   q1,  q0\n"     \
  "ldr   r1, [%[widx_dmap]], #4\n"  \
  "add   %[b_ptr],  %[b_ptr], r1\n" \
  "bne     0b\n"                    \
  "1:\n"

#define SPARSE_F32_F32_W4_v7_RELU                     \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q0, #0\n"              /* for relu */   \
  "vmax.f32   q3,   q3,   q0\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_F32_F32_W4_v7_RELU6                     \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q3,   q3,   q0\n"      /* relu6 */       \
  "vmin.f32   q3,   q3,   q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_F32_F32_W4_v7_LEAKY_RELU                      \
  /* do relu */                                              \
  "11: \n"                                                   \
  "cmp   %[vflag_act],  #3       \n" /* check leakey relu */ \
  "bne   12f                     \n" /* no act end */        \
  "vmov.i32   q0, #0\n"              /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"      /* leakey relu alpha */ \
  "vcge.f32   q4,    q3,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q5,    q3,    q1   \n" /* vmulq_f32 */         \
  "vbif       q3,    q5,    q4   \n" /* vmulq_f32 */         \
  "b      9f                    \n"

#define SPARSE_F32_F32_W4_v7_HARD_SWISH        \
  /* do relu */                                \
  "12: \n"                                     \
  "vmov.u32   q0,   #0                     \n" \
  "vld1.f32   {d2-d5}, [%[hs_param]]!      \n" \
  "vld1.f32   {d10-d11}, [%[hs_param]]     \n" \
  "vadd.f32   q6, q3, q1                \n"    \
  "vmul.f32   q7, q3, q2                \n"    \
  "vmax.f32   q6, q6, q0                \n"    \
  "vmin.f32   q6, q6, q5                \n"    \
  "vmul.f32   q3, q7, q6                \n"    \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx4, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx4, and the required data is
 * MxKxKx4.
 */
#define SPARSE_F32_F32_W4_v7_OUT                             \
  SPARSE_F32_F32_W4_v7_KERNEL SPARSE_F32_F32_W4_v7_RELU      \
      SPARSE_F32_F32_W4_v7_RELU6                             \
          SPARSE_F32_F32_W4_v7_LEAKY_RELU /* store result */ \
      SPARSE_F32_F32_W4_v7_HARD_SWISH "vst1.32   {d6-d7},  [%[c_ptr]]\n"

/**
 * \brief Sparse calculation implementation of 1x1 convolution, both input and
 * output are f32.
 * Sparse matrix multiplication is calculated in blocks, the block size is Mx48,
 * that is,
 * the parameter matrix is MxK, and the activation matrix is Kx48; when N is
 * less than 48,
 * it is calculated in blocks of Mx32, Mx16, Mx8, and Mx4 in turn;
 * @param A sparse weight data
 * @param B dense input data
 * @param widx_dmap An array of int32_t values storing scaled [by sizeof(input
 * element)] difference
 * between input channels corresponding to successive non-zero element
 * @param nidx_nnzmap the number of non-zero kernel elements per each output
 * channel
 * @param bias
 * @param output
 * @param M
 * @param N
 * @param K
 * @param param
 * @param ctx
 */
void sparse_conv_fp32_pipelined(const float* A,
                                const float* B,
                                const int32_t* widx_dmap,
                                const uint32_t* nidx_nnzmap,
                                const float* bias,
                                float* output,
                                const int M,
                                const int K,
                                const int N,
                                const operators::SparseConvParam& param,
                                ARMContext* ctx) {
  auto act_param = param.activation_param;
  auto act_type = act_param.active_type;
  volatile float alpha = 0.f;
  int flag_act = 0x00;  // relu: 1, relu6: 2, leakey: 3
  float vhs_param[12] = {0.f};
  if (act_param.has_active) {
    if (act_type == lite_api::ActivationType::kRelu) {
      flag_act = 0x01;
    } else if (act_type == lite_api::ActivationType::kRelu6) {
      flag_act = 0x02;
      alpha = act_param.Relu_clipped_coef;
    } else if (act_type == lite_api::ActivationType::kLeakyRelu) {
      flag_act = 0x03;
      alpha = act_param.Leaky_relu_alpha;
    } else if (act_type == lite_api::ActivationType::kHardSwish) {
      flag_act = 0x04;
      for (int i = 0; i < 4; i++) {
        vhs_param[i] = act_param.hard_swish_offset;
        vhs_param[i + 4] = 1.0 / act_param.hard_swish_scale;
        vhs_param[i + 8] = act_param.hard_swish_threshold;
      }
    }
  }
  int flag_bias = (bias != nullptr) ? 1 : 0;
  size_t mc = N * sizeof(float);
  size_t nc = M;
  size_t output_stride = N * sizeof(float);
  size_t output_decrement = output_stride * nc - 48 * sizeof(float);
  while
    SPARSE_LIKELY(mc >= 48 * sizeof(float)) {
      LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
        float* cur_output =
            reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
        const float* cur_w = A;
        uint32_t nnz = nidx_nnzmap[i];
        const float* cur_b = B;
        const int32_t* dmap = widx_dmap;
        if (i != 0) {
          int cur_rem = nidx_nnzmap[i - 1] & 3;
          if (cur_rem != 0) {
            cur_rem = 4 - cur_rem;
          }
          nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
          cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
          cur_b += ((nidx_nnzmap[i - 1] == 0)
                        ? 0
                        : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
          dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
        }
        uint32_t pair_num = nnz % 4;
        uint32_t lave_num = (pair_num == 0) ? 0 : (4 - pair_num);
        float vbias = (bias != nullptr) ? bias[i] : 0.0;
        float* hs_param = vhs_param;
        // clang-format off
            asm volatile(SPARSE_F32_F32_W48_v7_OUT  
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha)
              : "q0",
                "q1",
                "q2",
                "q3",
                "q4",
                "q5",
                "q6",
                "q7",
                "q8",
                "q9",
                "q10",
                "q11",
                "q12",
                "q13",
                "q14",
                "q15",
                "r0",
                "r2",
                "cc",
                "memory");
        // clang-format on
      }
      LITE_PARALLEL_COMMON_END();
      output = reinterpret_cast<float*>((uintptr_t)output + 48 * sizeof(float));
      B += 48;
      mc -= 48 * sizeof(float);
    }

  if
    SPARSE_UNLIKELY(mc != 0) {
      output_decrement += 16 * sizeof(float);
      if (mc & (32 * sizeof(float))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const float* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const float* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            int cur_rem = nidx_nnzmap[i - 1] & 3;
            if (cur_rem != 0) {
              cur_rem = 4 - cur_rem;
            }
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
            cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
            cur_b += ((nidx_nnzmap[i - 1] == 0)
                          ? 0
                          : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
            dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
          }
          uint32_t pair_num = nnz % 4;
          uint32_t lave_num = (pair_num == 0) ? 0 : (4 - pair_num);
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float* hs_param = vhs_param;
          // clang-format off
            asm volatile(SPARSE_F32_F32_W32_v7_OUT  
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha)
              : "q0",
                "q1",
                "q2",
                "q3",
                "q4",
                "q5",
                "q6",
                "q7",
                "q8",
                "q9",
                "q10",
                "q11",
                "q12",
                "q13",
                "q14",
                "q15",
                "r0",
                "r2",
                "cc",
                "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 32 * sizeof(float));
        B += 32;
        mc -= 32 * sizeof(float);
      }
      output_decrement += 16 * sizeof(float);
      if (mc & (16 * sizeof(float))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const float* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const float* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            int cur_rem = nidx_nnzmap[i - 1] & 3;
            if (cur_rem != 0) {
              cur_rem = 4 - cur_rem;
            }
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
            cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
            cur_b += ((nidx_nnzmap[i - 1] == 0)
                          ? 0
                          : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
            dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
          }
          uint32_t pair_num = nnz % 4;
          uint32_t lave_num = (pair_num == 0) ? 0 : (4 - pair_num);
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float* hs_param = vhs_param;
          // clang-format off
            asm volatile(SPARSE_F32_F32_W16_v7_OUT  
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha)
              : "q0",
                "q1",
                "q2",
                "q3",
                "q4",
                "q5",
                "q6",
                "q7",
                "q8",
                "q9",
                "q10",
                "q11",
                "r0",
                "r2",
                "cc",
                "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 16 * sizeof(float));
        B += 16;
        mc -= 16 * sizeof(float);
      }
      output_decrement += 8 * sizeof(float);
      if (mc & (8 * sizeof(float))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const float* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const float* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            int cur_rem = nidx_nnzmap[i - 1] & 3;
            if (cur_rem != 0) {
              cur_rem = 4 - cur_rem;
            }
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
            cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
            cur_b += ((nidx_nnzmap[i - 1] == 0)
                          ? 0
                          : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
            dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
          }
          uint32_t pair_num = nnz % 4;
          uint32_t lave_num = (pair_num == 0) ? 0 : (4 - pair_num);
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float* hs_param = vhs_param;
          // clang-format off
            asm volatile(SPARSE_F32_F32_W8_v7_OUT  
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha)
              : "q0",
                "q1",
                "q2",
                "q3",
                "q4",
                "q5",
                "q6",
                "q7",
                "q8",
                "q9",
                "r0",
                "r1",
                "cc",
                "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 8 * sizeof(float));
        B += 8;
        mc -= 8 * sizeof(float);
      }
      output_decrement += 4 * sizeof(float);
      if (mc & (4 * sizeof(float))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const float* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const float* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            int cur_rem = nidx_nnzmap[i - 1] & 3;
            if (cur_rem != 0) {
              cur_rem = 4 - cur_rem;
            }
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
            cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
            cur_b += ((nidx_nnzmap[i - 1] == 0)
                          ? 0
                          : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
            dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
          }
          uint32_t pair_num = nnz % 4;
          uint32_t lave_num = (pair_num == 0) ? 0 : (4 - pair_num);
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float* hs_param = vhs_param;
          // clang-format off
            asm volatile(SPARSE_F32_F32_W4_v7_OUT  
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [n] "+r"(pair_num),
                [m] "+r"(lave_num),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha)
              : "q0",
                "q1",
                "q3",
                "q4",
                "q5",
                "q6",
                "q7",
                "r0",
                "r1",
                "cc",
                "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 4 * sizeof(float));
        B += 4;
        mc -= 4 * sizeof(float);
      }
      output_decrement += 2 * sizeof(float);
      if (mc & (2 * sizeof(float))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const float* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const float* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            int cur_rem = nidx_nnzmap[i - 1] & 3;
            if (cur_rem != 0) {
              cur_rem = 4 - cur_rem;
            }
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
            cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
            cur_b += ((nidx_nnzmap[i - 1] == 0)
                          ? 0
                          : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
            dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
          }
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float32x2_t vacc01n0 = vdup_n_f32(vbias);
          if
            SPARSE_LIKELY(nnz != 0) {
              do {
                const intptr_t diff = *dmap++;
                const float32x2_t vi01 = vld1_f32(cur_b);
                cur_b = (const float*)((uintptr_t)cur_b + (uintptr_t)diff);
                const float32x2_t vw = vld1_dup_f32(cur_w);
                cur_w += 1;
                vacc01n0 = vmla_lane_f32(vacc01n0, vi01, vw, 0);
              } while (--nnz != 0);
            }
          if (flag_act == 1) {
            float32x2_t vzero = vdup_n_f32(0);
            vacc01n0 = vmax_f32(vacc01n0, vzero);
          } else if (flag_act == 2) {
            float32x2_t vzero = vdup_n_f32(0);
            float32x2_t aph = vdup_n_f32(alpha);
            vacc01n0 = vmax_f32(vacc01n0, vzero);
            vacc01n0 = vmin_f32(vacc01n0, aph);
          } else if (flag_act == 0) {
          } else if (flag_act == 3) {
            float32x2_t vzero = vdup_n_f32(0);
            float32x2_t aph = vdup_n_f32(alpha);
            uint32x2_t vflag0123 = vcge_f32(vacc01n0, vzero);
            float32x2_t v0123 = vmul_f32(vacc01n0, aph);
            vacc01n0 = vbsl_f32(vflag0123, vacc01n0, v0123);
          } else {
            float32x2_t vzero = vdup_n_f32(0);
            float32x2_t offset = vdup_n_f32(act_param.hard_swish_offset);
            float32x2_t hs_scale = vdup_n_f32(1.0 / act_param.hard_swish_scale);
            float32x2_t thre = vdup_n_f32(act_param.hard_swish_threshold);
            float32x2_t vset0123 = vadd_f32(vacc01n0, offset);
            float32x2_t vscale0123 = vmul_f32(vacc01n0, hs_scale);
            vset0123 = vmax_f32(vset0123, vzero);
            vset0123 = vmin_f32(vset0123, thre);
            vacc01n0 = vmul_f32(vscale0123, vset0123);
          }
          vst1_f32(cur_output, vacc01n0);
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 2 * sizeof(float));
        B += 2;
        mc -= 2 * sizeof(float);
      }

      output_decrement += 1 * sizeof(float);
      if (mc & (1 * sizeof(float))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const float* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const float* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            int cur_rem = nidx_nnzmap[i - 1] & 3;
            if (cur_rem != 0) {
              cur_rem = 4 - cur_rem;
            }
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1] - cur_rem;
            cur_w = A + nidx_nnzmap[i - 1] + cur_rem;
            cur_b += ((nidx_nnzmap[i - 1] == 0)
                          ? 0
                          : widx_dmap[nidx_nnzmap[i - 1] - 1] / sizeof(float));
            dmap = widx_dmap + nidx_nnzmap[i - 1] + cur_rem;
          }
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float32x2_t vacc01n0 = vdup_n_f32(vbias);
          if
            SPARSE_LIKELY(nnz != 0) {
              do {
                const intptr_t diff = *dmap++;
                const float32x2_t vi01 = vld1_dup_f32(cur_b);
                cur_b = (const float*)((uintptr_t)cur_b + (uintptr_t)diff);
                const float32x2_t vw = vld1_dup_f32(cur_w);
                cur_w += 1;
                vacc01n0 = vmla_lane_f32(vacc01n0, vi01, vw, 0);
              } while (--nnz != 0);
            }
          if (flag_act == 1) {
            float32x2_t vzero = vdup_n_f32(0);
            vacc01n0 = vmax_f32(vacc01n0, vzero);
          } else if (flag_act == 2) {
            float32x2_t vzero = vdup_n_f32(0);
            float32x2_t aph = vdup_n_f32(alpha);
            vacc01n0 = vmax_f32(vacc01n0, vzero);
            vacc01n0 = vmin_f32(vacc01n0, aph);
          } else if (flag_act == 0) {
          } else if (flag_act == 3) {
            float32x2_t vzero = vdup_n_f32(0);
            float32x2_t aph = vdup_n_f32(alpha);
            uint32x2_t vflag0123 = vcge_f32(vacc01n0, vzero);
            float32x2_t v0123 = vmul_f32(vacc01n0, aph);
            vacc01n0 = vbsl_f32(vflag0123, vacc01n0, v0123);
          } else {
            float32x2_t vzero = vdup_n_f32(0);
            float32x2_t offset = vdup_n_f32(act_param.hard_swish_offset);
            float32x2_t hs_scale = vdup_n_f32(1.0 / act_param.hard_swish_scale);
            float32x2_t thre = vdup_n_f32(act_param.hard_swish_threshold);
            float32x2_t vset0123 = vadd_f32(vacc01n0, offset);
            float32x2_t vscale0123 = vmul_f32(vacc01n0, hs_scale);
            vset0123 = vmax_f32(vset0123, vzero);
            vset0123 = vmin_f32(vset0123, thre);
            vacc01n0 = vmul_f32(vscale0123, vset0123);
          }
          vst1_lane_f32(cur_output, vacc01n0, 0);
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 1 * sizeof(float));
        B += 1;
        mc -= 1 * sizeof(float);
      }
    }
}

#define SPARSE_INT8_F32_W48_v7_KERNEL  \
  "veor   q4,   q0,  q0\n"             \
  "veor   q5,   q1,  q1\n"             \
  "veor   q6,   q2,  q2\n"             \
  "veor   q7,   q3,  q3\n"             \
  "pld  [%[a_ptr], #32]    \n"         \
  "veor   q8,   q0,  q0\n"             \
  "veor   q9,   q1,  q1\n"             \
  "pld  [%[widx_dmap], #32]    \n"     \
  "veor   q10,  q2,  q2\n"             \
  "veor   q11,  q3,  q3\n"             \
  "veor   q12,  q0,  q0\n"             \
  "pld  [%[b_ptr], #64]    \n"         \
  "veor   q13,  q1,  q1\n"             \
  "veor   q14,  q2,  q2\n"             \
  "veor   q15,  q3,  q3\n"             \
  "cmp    %[k],    #0\n"               \
  "beq    1f\n" /* main loop*/         \
  "0:\n"                               \
  "ldrsb   r0, [%[a_ptr]], #1\n"       \
  "subs    %[k],   %[k],   #1\n"       \
  "mov   r2,   %[b_ptr]\n"             \
  "vld1.8  {d2-d5}, [r2]\n"            \
  "vdup.8    d0,   r0\n"               \
  "vmull.s8    q3,    d2,  d0\n"       \
  "vaddw.s16   q4,    q4,  d6\n"       \
  "vaddw.s16   q5,    q5,  d7\n"       \
  "vmull.s8    q3,    d3,  d0\n"       \
  "pld  [%[widx_dmap], #32]    \n"     \
  "vaddw.s16   q6,    q6,  d6\n"       \
  "vaddw.s16   q7,    q7,  d7\n"       \
  "vmull.s8    q3,    d4,  d0\n"       \
  "vaddw.s16   q8,    q8, d6\n"        \
  "vaddw.s16   q9,    q9, d7\n"        \
  "vmull.s8    q3,    d5,  d0\n"       \
  "ldr     r1, [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], r1\n"    \
  "vaddw.s16   q10,   q10, d6\n"       \
  "vaddw.s16   q11,   q11, d7\n"       \
  "pld  [%[b_ptr], #64]    \n"         \
  "add  r2,  r2,   #32\n"              \
  "vld1.8  {d2-d3}, [r2]\n"            \
  "vmull.s8    q3,    d2,  d0\n"       \
  "vaddw.s16   q12,   q12,  d6\n"      \
  "pld  [%[a_ptr], #32]    \n"         \
  "vaddw.s16   q13,   q13,  d7\n"      \
  "vmull.s8    q3,    d3,  d0\n"       \
  "vaddw.s16   q14,   q14,  d6\n"      \
  "vaddw.s16   q15,   q15,  d7\n"      \
  "bne     0b\n"                       \
  "1:\n"

#define SPARSE_INT8_TRANS_INT32_TO_FP32_W48_v7      \
  /* write output */                                \
  "vdup.32    q0,   %[vscale]\n"                    \
  "vdup.32    q1,   %[vbias]\n"                     \
  "vcvt.f32.s32   q2, q4\n" /* cvt int32 to fp32*/  \
  "vcvt.f32.s32   q3, q5\n" /* cvt int32 to fp32*/  \
  "vdup.32    q4,   d2[0]\n"                        \
  "vdup.32    q5,   d2[0]\n"                        \
  "vmla.f32   q4,  q2,  q0\n"                       \
  "vmla.f32   q5,  q3,  q0\n"                       \
  "vcvt.f32.s32   q2, q6\n" /* cvt int32 to fp32*/  \
  "vcvt.f32.s32   q3, q7\n" /* cvt int32 to fp32*/  \
  "vdup.32    q6,   d2[0]\n"                        \
  "vdup.32    q7,   d2[0]\n"                        \
  "vmla.f32   q6,  q2,  q0\n"                       \
  "vmla.f32   q7,  q3,  q0\n"                       \
  "vcvt.f32.s32   q2, q8\n" /* cvt int32 to fp32*/  \
  "vcvt.f32.s32   q3, q9\n" /* cvt int32 to fp32*/  \
  "vdup.32    q8,   d2[0]\n"                        \
  "vdup.32    q9,   d2[0]\n"                        \
  "vmla.f32   q8,  q2,  q0\n"                       \
  "vmla.f32   q9,  q3,  q0\n"                       \
  "vcvt.f32.s32   q2, q10\n" /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q3, q11\n" /* cvt int32 to fp32*/ \
  "vdup.32    q10,   d2[0]\n"                       \
  "vdup.32    q11,   d2[0]\n"                       \
  "vmla.f32   q10,  q2,  q0\n"                      \
  "vmla.f32   q11,  q3,  q0\n"                      \
  "vcvt.f32.s32   q2, q12\n" /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q3, q13\n" /* cvt int32 to fp32*/ \
  "vdup.32    q12,   d2[0]\n"                       \
  "vdup.32    q13,   d2[0]\n"                       \
  "vmla.f32   q12,  q2,  q0\n"                      \
  "vmla.f32   q13,  q3,  q0\n"                      \
  "vcvt.f32.s32   q2, q14\n" /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q3, q15\n" /* cvt int32 to fp32*/ \
  "vdup.32    q14,   d2[0]\n"                       \
  "vdup.32    q15,   d2[0]\n"                       \
  "vmla.f32   q14,  q2,  q0\n"                      \
  "vmla.f32   q15,  q3,  q0\n"

#define SPARSE_INT8_F32_W48_v7_RELU                   \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q0, #0\n"              /* for relu */   \
  "vmax.f32   q4,   q4,   q0\n"      /* relu */       \
  "vmax.f32   q5,   q5,   q0\n"      /* relu */       \
  "vmax.f32   q6,   q6,   q0\n"      /* relu */       \
  "vmax.f32   q7,   q7,   q0\n"      /* relu */       \
  "vmax.f32   q8,   q8,   q0\n"      /* relu */       \
  "vmax.f32   q9,   q9,   q0\n"      /* relu */       \
  "vmax.f32   q10,  q10,  q0\n"      /* relu */       \
  "vmax.f32   q11,  q11,  q0\n"      /* relu */       \
  "vmax.f32   q12,  q12,  q0\n"      /* relu */       \
  "vmax.f32   q13,  q13,  q0\n"      /* relu */       \
  "vmax.f32   q14,  q14,  q0\n"      /* relu */       \
  "vmax.f32   q15,  q15,  q0\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W48_v7_RELU6                   \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q4,   q4,   q0\n"      /* relu6 */       \
  "vmax.f32   q5,   q5,   q0\n"      /* relu6 */       \
  "vmax.f32   q6,   q6,   q0\n"      /* relu6 */       \
  "vmax.f32   q7,   q7,   q0\n"      /* relu6 */       \
  "vmax.f32   q8,   q8,   q0\n"      /* relu6 */       \
  "vmax.f32   q9,   q9,   q0\n"      /* relu6 */       \
  "vmax.f32   q10,  q10,  q0\n"      /* relu6 */       \
  "vmax.f32   q11,  q11,  q0\n"      /* relu6 */       \
  "vmax.f32   q12,  q12,  q0\n"      /* relu6 */       \
  "vmax.f32   q13,  q13,  q0\n"      /* relu6 */       \
  "vmax.f32   q14,  q14,  q0\n"      /* relu6 */       \
  "vmax.f32   q15,  q15,  q0\n"      /* relu6 */       \
  "vmin.f32   q4,   q4,   q1\n"      /* relu6 */       \
  "vmin.f32   q5,   q5,   q1\n"      /* relu6 */       \
  "vmin.f32   q6,   q6,   q1\n"      /* relu6 */       \
  "vmin.f32   q7,   q7,   q1\n"      /* relu6 */       \
  "vmin.f32   q8,   q8,   q1\n"      /* relu6 */       \
  "vmin.f32   q9,   q9,   q1\n"      /* relu6 */       \
  "vmin.f32   q10,  q10,  q1\n"      /* relu6 */       \
  "vmin.f32   q11,  q11,  q1\n"      /* relu6 */       \
  "vmin.f32   q12,  q12,  q1\n"      /* relu6 */       \
  "vmin.f32   q13,  q13,  q1\n"      /* relu6 */       \
  "vmin.f32   q14,  q14,  q1\n"      /* relu6 */       \
  "vmin.f32   q15,  q15,  q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W48_v7_LEAKY_RELU                      \
  /* do relu */                                                \
  "11: \n"                                                     \
  "cmp   %[vflag_act],  #3       \n"   /* check leakey relu */ \
  "bne   12f                     \n"   /* no act end */        \
  "vmov.i32   q0, #0\n"                /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"        /* leakey relu alpha */ \
  "vcge.f32   q2,    q4,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q4,    q1     \n" /* vmulq_f32 */         \
  "vbif       q4,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q5,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q5,    q1     \n" /* vmulq_f32 */         \
  "vbif       q5,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q6,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q6,    q1     \n" /* vmulq_f32 */         \
  "vbif       q6,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q7,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q7,    q1     \n" /* vmulq_f32 */         \
  "vbif       q7,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q8,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q8,    q1     \n" /* vmulq_f32 */         \
  "vbif       q8,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q9,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q9,    q1     \n" /* vmulq_f32 */         \
  "vbif       q9,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q10,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q10,    q1    \n" /* vmulq_f32 */         \
  "vbif       q10,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q11,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q11,    q1    \n" /* vmulq_f32 */         \
  "vbif       q11,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q12,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q12,    q1    \n" /* vmulq_f32 */         \
  "vbif       q12,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q13,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q13,    q1    \n" /* vmulq_f32 */         \
  "vbif       q13,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q14,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q14,    q1    \n" /* vmulq_f32 */         \
  "vbif       q14,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q15,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q15,    q1    \n" /* vmulq_f32 */         \
  "vbif       q15,    q3,    q2    \n"                         \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W48_v7_HARD_SWISH                             \
  /* do relu */                                                       \
  "12: \n"                                                            \
  "vld1.f32   {d0-d3}, [%[hs_param]]!      @ load hard swish alpha\n" \
  "vadd.f32   q3, q4, q0                \n"                           \
  "vmul.f32   q4, q4, q1                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]       \n"                        \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q4, q4, q3                \n"                           \
  "vadd.f32   q3, q5, q0                \n"                           \
  "vmul.f32   q5, q5, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]       \n"                        \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q5, q5, q3                \n"                           \
  "vadd.f32   q3, q6, q0                \n"                           \
  "vmul.f32   q6, q6, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]       \n"                        \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q6, q6, q3                \n"                           \
  "vadd.f32   q3, q7, q0                \n"                           \
  "vmul.f32   q7, q7, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q7, q7, q3                \n"                           \
  "vadd.f32   q3, q8, q0                \n"                           \
  "vmul.f32   q8, q8, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q8, q8, q3                \n"                           \
  "vadd.f32   q3, q9, q0                \n"                           \
  "vmul.f32   q9, q9, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q9, q9, q3                \n"                           \
  "vadd.f32   q3, q10, q0               \n"                           \
  "vmul.f32   q10, q10, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q10, q10, q3              \n"                           \
  "vadd.f32   q3, q11, q0               \n"                           \
  "vmul.f32   q11, q11, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q11, q11, q3              \n"                           \
  "vadd.f32   q3, q12, q0               \n"                           \
  "vmul.f32   q12, q12, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q12, q12, q3              \n"                           \
  "vadd.f32   q3, q13, q0               \n"                           \
  "vmul.f32   q13, q13, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q13, q13, q3              \n"                           \
  "vadd.f32   q3, q14, q0               \n"                           \
  "vmul.f32   q14, q14, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q14, q14, q3              \n"                           \
  "vadd.f32   q3, q15, q0               \n"                           \
  "vmul.f32   q15, q15, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q15, q15, q3              \n"                           \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx48, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx48, and the required data is
 * MxKxKx48.
 */
#define SPARSE_INT8_F32_W48_v7_OUT                                            \
  SPARSE_INT8_F32_W48_v7_KERNEL SPARSE_INT8_TRANS_INT32_TO_FP32_W48_v7        \
      SPARSE_INT8_F32_W48_v7_RELU SPARSE_INT8_F32_W48_v7_RELU6                \
          SPARSE_INT8_F32_W48_v7_LEAKY_RELU SPARSE_INT8_F32_W48_v7_HARD_SWISH \
      "mov   r0,   %[c_ptr]\n" /* store result */                             \
      "vst1.32   {d8-d11},  [r0]\n"                                           \
      "add  r0,  r0,   #32\n"                                                 \
      "vst1.32   {d12-d15},  [r0]\n"                                          \
      "add  r0,  r0,   #32\n"                                                 \
      "vst1.32   {d16-d19},  [r0]\n"                                          \
      "add  r0,  r0,   #32\n"                                                 \
      "vst1.32   {d20-d23},  [r0]\n"                                          \
      "add  r0,  r0,   #32\n"                                                 \
      "vst1.32   {d24-d27},  [r0]\n"                                          \
      "add  r0,  r0,   #32\n"                                                 \
      "vst1.32   {d28-d31},  [r0]\n"

#define SPARSE_INT8_F32_W32_v7_KERNEL  \
  "veor   q8,   q0,  q0\n"             \
  "veor   q9,   q1,  q1\n"             \
  "pld  [%[a_ptr], #32]    \n"         \
  "veor   q10,  q2,  q2\n"             \
  "veor   q11,  q3,  q3\n"             \
  "pld  [%[widx_dmap], #32]    \n"     \
  "veor   q12,  q4,  q4\n"             \
  "veor   q13,  q5,  q5\n"             \
  "pld  [%[b_ptr], #64]    \n"         \
  "veor   q14,  q6,  q6\n"             \
  "veor   q15,  q7,  q7\n"             \
  "cmp    %[k],    #0\n"               \
  "beq    1f\n" /* main loop*/         \
  "0:\n"                               \
  "ldrsb   r0, [%[a_ptr]], #1\n"       \
  "subs    %[k],   %[k],   #1\n"       \
  "vld1.8  {d2-d5}, [%[b_ptr]]\n"      \
  "vdup.8    d0,   r0\n"               \
  "vmull.s8    q3,    d2,  d0\n"       \
  "vmull.s8    q4,    d3,  d0\n"       \
  "pld  [%[widx_dmap], #32]    \n"     \
  "vmull.s8    q5,    d4,  d0\n"       \
  "vmull.s8    q6,    d5,  d0\n"       \
  "ldr     r1, [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], r1\n"    \
  "vaddw.s16   q8,     q8,  d6\n"      \
  "vaddw.s16   q9,     q9,  d7\n"      \
  "pld  [%[a_ptr], #32]    \n"         \
  "vaddw.s16   q10,    q10,  d8\n"     \
  "vaddw.s16   q11,    q11,  d9\n"     \
  "vaddw.s16   q12,    q12,  d10\n"    \
  "pld  [%[b_ptr], #64]    \n"         \
  "vaddw.s16   q13,    q13,  d11\n"    \
  "vaddw.s16   q14,    q14,  d12\n"    \
  "vaddw.s16   q15,    q15,  d13\n"    \
  "bne     0b\n"                       \
  "1:\n"

#define SPARSE_INT8_TRANS_INT32_TO_FP32_W32_v7       \
  /* write output */                                 \
  "vcvt.f32.s32   q0, q8\n"  /* cvt int32 to fp32*/  \
  "vcvt.f32.s32   q1, q9\n"  /* cvt int32 to fp32*/  \
  "vcvt.f32.s32   q2, q10\n" /* cvt int32 to fp32*/  \
  "vcvt.f32.s32   q3, q11\n" /* cvt int32 to fp32*/  \
  "vdup.32    q8,   %[vscale]\n"                     \
  "vdup.32    q4,   %[vbias]\n"                      \
  "vdup.32    q5,   d8[0]\n"                         \
  "vdup.32    q6,   d8[0]\n"                         \
  "vdup.32    q7,   d8[0]\n"                         \
  "3:\n"                                             \
  "vmla.f32  q4,  q0,  q8\n"                         \
  "vmla.f32  q5,  q1,  q8\n"                         \
  "vmla.f32  q6,  q2,  q8\n"                         \
  "vmla.f32  q7,  q3,  q8\n"                         \
  "4:\n"                                             \
  "vcvt.f32.s32   q8, q12\n"  /* int32 to fp32*/     \
  "vcvt.f32.s32   q9, q13\n"  /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q10, q14\n" /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q11, q15\n" /* cvt int32 to fp32*/ \
  "vdup.32    q12,   %[vscale]\n"                    \
  "vdup.32    q0,   %[vbias]\n"                      \
  "vdup.32    q1,   d0[0]\n"                         \
  "vdup.32    q2,   d0[0]\n"                         \
  "vdup.32    q3,   d0[0]\n"                         \
  "6:\n"                                             \
  "vmla.f32  q0,  q8,  q12\n"                        \
  "vmla.f32  q1,  q9,  q12\n"                        \
  "vmla.f32  q2,  q10,  q12\n"                       \
  "vmla.f32  q3,  q11,  q12\n"

#define SPARSE_INT8_F32_W32_v7_RELU                   \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q8, #0\n"              /* for relu */   \
  "vmax.f32   q0,   q0,   q8\n"      /* relu */       \
  "vmax.f32   q1,   q1,   q8\n"      /* relu */       \
  "vmax.f32   q2,   q2,   q8\n"      /* relu */       \
  "vmax.f32   q3,   q3,   q8\n"      /* relu */       \
  "vmax.f32   q4,   q4,   q8\n"      /* relu */       \
  "vmax.f32   q5,   q5,   q8\n"      /* relu */       \
  "vmax.f32   q6,   q6,   q8\n"      /* relu */       \
  "vmax.f32   q7,   q7,   q8\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W32_v7_RELU6                   \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q8,   #0\n"            /* for relu6 */   \
  "vdup.32    q9,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q0,   q0,   q8\n"      /* relu6 */       \
  "vmax.f32   q1,   q1,   q8\n"      /* relu6 */       \
  "vmax.f32   q2,   q2,   q8\n"      /* relu6 */       \
  "vmax.f32   q3,   q3,   q8\n"      /* relu6 */       \
  "vmax.f32   q4,   q4,   q8\n"      /* relu6 */       \
  "vmax.f32   q5,   q5,   q8\n"      /* relu6 */       \
  "vmax.f32   q6,   q6,   q8\n"      /* relu6 */       \
  "vmax.f32   q7,   q7,   q8\n"      /* relu6 */       \
  "vmin.f32   q0,   q0,   q9\n"      /* relu6 */       \
  "vmin.f32   q1,   q1,   q9\n"      /* relu6 */       \
  "vmin.f32   q2,   q2,   q9\n"      /* relu6 */       \
  "vmin.f32   q3,   q3,   q9\n"      /* relu6 */       \
  "vmin.f32   q4,   q4,   q9\n"      /* relu6 */       \
  "vmin.f32   q5,   q5,   q9\n"      /* relu6 */       \
  "vmin.f32   q6,   q6,   q9\n"      /* relu6 */       \
  "vmin.f32   q7,   q7,   q9\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W32_v7_LEAKY_RELU                     \
  /* do relu */                                               \
  "11: \n"                                                    \
  "cmp   %[vflag_act],  #3       \n"  /* check leakey relu */ \
  "bne   12f                     \n"  /* no act end */        \
  "vmov.i32   q8, #0\n"               /* for relu */          \
  "vdup.32    q9,  %[valpha]\n"       /* leakey relu alpha */ \
  "vcge.f32   q10,    q0,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q11,    q0,    q9   \n" /* vmulq_f32 */         \
  "vcge.f32   q12,    q1,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q13,    q1,    q9   \n" /* vmulq_f32 */         \
  "vcge.f32   q14,    q2,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q15,    q2,    q9   \n" /* vmulq_f32 */         \
  "vbif       q0,    q11,    q10   \n"                        \
  "vbif       q1,    q13,    q12   \n"                        \
  "vbif       q2,    q15,    q14   \n"                        \
  "vcge.f32   q10,    q3,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q11,    q3,    q9   \n" /* vmulq_f32 */         \
  "vcge.f32   q12,    q4,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q13,    q4,    q9   \n" /* vmulq_f32 */         \
  "vcge.f32   q14,    q5,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q15,    q5,    q9   \n" /* vmulq_f32 */         \
  "vbif       q3,    q11,    q10   \n"                        \
  "vbif       q4,    q13,    q12   \n"                        \
  "vbif       q5,    q15,    q14   \n"                        \
  "vcge.f32   q10,    q6,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q11,    q6,    q9   \n" /* vmulq_f32 */         \
  "vcge.f32   q12,    q7,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q13,    q7,    q9   \n" /* vmulq_f32 */         \
  "vbif       q6,    q11,    q10   \n"                        \
  "vbif       q7,    q13,    q12   \n"                        \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W32_v7_HARD_SWISH        \
  /* do relu */                                  \
  "12: \n"                                       \
  "vmov.u32   q8,   #0                     \n"   \
  "vld1.f32   {d18-d21}, [%[hs_param]]!      \n" \
  "vld1.f32   {d22-d23}, [%[hs_param]]       \n" \
  "vadd.f32   q12, q0, q9                 \n"    \
  "vadd.f32   q14, q1, q9                 \n"    \
  "vmul.f32   q13, q0, q10                \n"    \
  "vmul.f32   q14, q1, q10                \n"    \
  "vmax.f32   q12, q12, q8                \n"    \
  "vmax.f32   q14, q14, q8                \n"    \
  "vmin.f32   q12, q12, q11               \n"    \
  "vmin.f32   q14, q14, q11               \n"    \
  "vmul.f32   q0,  q13, q12               \n"    \
  "vmul.f32   q1,  q15, q14               \n"    \
  "vadd.f32   q12, q2, q9                 \n"    \
  "vadd.f32   q14, q3, q9                 \n"    \
  "vmul.f32   q13, q2, q10                \n"    \
  "vmul.f32   q14, q3, q10                \n"    \
  "vmax.f32   q12, q12, q8                \n"    \
  "vmax.f32   q14, q14, q8                \n"    \
  "vmin.f32   q12, q12, q11               \n"    \
  "vmin.f32   q14, q14, q11               \n"    \
  "vmul.f32   q2,  q13, q12               \n"    \
  "vmul.f32   q3,  q15, q14               \n"    \
  "vadd.f32   q12, q4, q9                 \n"    \
  "vadd.f32   q14, q5, q9                 \n"    \
  "vmul.f32   q13, q4, q10                \n"    \
  "vmul.f32   q14, q5, q10                \n"    \
  "vmax.f32   q12, q12, q8                \n"    \
  "vmax.f32   q14, q14, q8                \n"    \
  "vmin.f32   q12, q12, q11               \n"    \
  "vmin.f32   q14, q14, q11               \n"    \
  "vmul.f32   q4,  q13, q12               \n"    \
  "vmul.f32   q5,  q15, q14               \n"    \
  "vadd.f32   q12, q6, q9                 \n"    \
  "vadd.f32   q14, q7, q9                 \n"    \
  "vmul.f32   q13, q6, q10                \n"    \
  "vmul.f32   q14, q7, q10                \n"    \
  "vmax.f32   q12, q12, q8                \n"    \
  "vmax.f32   q14, q14, q8                \n"    \
  "vmin.f32   q12, q12, q11               \n"    \
  "vmin.f32   q14, q14, q11               \n"    \
  "vmul.f32   q6,  q13, q12               \n"    \
  "vmul.f32   q7,  q15, q14               \n"    \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx32, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx32, and the required data is
 * MxKxKx32.
 */
#define SPARSE_INT8_F32_W32_v7_OUT                                            \
  SPARSE_INT8_F32_W32_v7_KERNEL SPARSE_INT8_TRANS_INT32_TO_FP32_W32_v7        \
      SPARSE_INT8_F32_W32_v7_RELU SPARSE_INT8_F32_W32_v7_RELU6                \
          SPARSE_INT8_F32_W32_v7_LEAKY_RELU SPARSE_INT8_F32_W32_v7_HARD_SWISH \
      "mov   r0,   %[c_ptr]\n" /* store result */                             \
      "vst1.32   {d8-d11},  [r0]\n"                                           \
      "add  r0,  r0,   #32\n"                                                 \
      "vst1.32   {d12-d15},  [r0]\n"                                          \
      "add  r0,  r0,   #32\n"                                                 \
      "vst1.32   {d0-d3},  [r0]\n"                                            \
      "add  r0,  r0,   #32\n"                                                 \
      "vst1.32   {d4-d7},  [r0]\n"

#define SPARSE_INT8_F32_W16_v7_KERNEL  \
  "veor   q4,  q0,  q0\n"              \
  "pld  [%[a_ptr], #32]    \n"         \
  "veor   q5,  q1,  q1\n"              \
  "pld  [%[widx_dmap], #32]    \n"     \
  "veor   q6,  q2,  q2\n"              \
  "pld  [%[b_ptr], #32]    \n"         \
  "veor   q7,  q3,  q3\n"              \
  "cmp    %[k],    #0\n"               \
  "beq    1f\n" /* main loop*/         \
  "0:\n"                               \
  "ldrsb   r0, [%[a_ptr]], #1\n"       \
  "subs    %[k],   %[k],   #1\n"       \
  "vld1.8  {d2-d3}, [%[b_ptr]]\n"      \
  "vdup.8    d0,   r0\n"               \
  "vmull.s8    q2,    d2,  d0\n"       \
  "vmull.s8    q3,    d3,  d0\n"       \
  "ldr     r1, [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], r1\n"    \
  "vaddw.s16   q4,    q4,  d4\n"       \
  "vaddw.s16   q5,    q5,  d5\n"       \
  "pld  [%[b_ptr], #32]    \n"         \
  "vaddw.s16   q6,    q6,  d6\n"       \
  "vaddw.s16   q7,    q7,  d7\n"       \
  "bne     0b\n"                       \
  "1:\n"

#define SPARSE_INT8_TRANS_INT32_TO_FP32_W16_v7     \
  /* write output */                               \
  "vcvt.f32.s32   q0, q4\n" /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q1, q5\n" /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q2, q6\n" /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q3, q7\n" /* cvt int32 to fp32*/ \
  "vdup.32    q4,   %[vscale]\n"                   \
  "vdup.32    q8,   %[vbias]\n"                    \
  "vdup.32    q9,   d16[0]\n"                      \
  "vdup.32    q10,  d16[0]\n"                      \
  "vdup.32    q11,  d16[0]\n"                      \
  "3:\n"                                           \
  "vmla.f32  q8,   q0,  q4\n"                      \
  "vmla.f32  q9,   q1,  q4\n"                      \
  "vmla.f32  q10,  q2,  q4\n"                      \
  "vmla.f32  q11,  q3,  q4\n"

#define SPARSE_INT8_F32_W16_v7_RELU                   \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q0, #0\n"              /* for relu */   \
  "vmax.f32   q8,   q8,   q0\n"      /* relu */       \
  "vmax.f32   q9,   q9,   q0\n"      /* relu */       \
  "vmax.f32   q10,  q10,  q0\n"      /* relu */       \
  "vmax.f32   q11,  q11,  q0\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W16_v7_RELU6                   \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q8,   q8,   q0\n"      /* relu6 */       \
  "vmax.f32   q9,   q9,   q0\n"      /* relu6 */       \
  "vmax.f32   q10,  q10,  q0\n"      /* relu6 */       \
  "vmax.f32   q11,  q11,  q0\n"      /* relu6 */       \
  "vmin.f32   q8,   q8,   q1\n"      /* relu6 */       \
  "vmin.f32   q9,   q9,   q1\n"      /* relu6 */       \
  "vmin.f32   q10,  q10,  q1\n"      /* relu6 */       \
  "vmin.f32   q11,  q11,  q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W16_v7_LEAKY_RELU                    \
  /* do relu */                                              \
  "11: \n"                                                   \
  "cmp   %[vflag_act],  #3       \n" /* check leakey relu */ \
  "bne   12f                     \n" /* no act end */        \
  "vmov.i32   q0, #0\n"              /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"      /* leakey relu alpha */ \
  "vcge.f32   q2,    q8,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q8,    q1   \n" /* vmulq_f32 */         \
  "vcge.f32   q4,    q9,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q5,    q9,    q1   \n" /* vmulq_f32 */         \
  "vcge.f32   q6,    q10,   q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q7,    q10,   q1   \n" /* vmulq_f32 */         \
  "vbif       q8,    q3,    q2   \n"                         \
  "vbif       q9,    q5,    q4   \n"                         \
  "vbif       q10,   q7,    q6   \n"                         \
  "vcge.f32   q2,    q11,    q0   \n" /* vcgeq_f32 */        \
  "vmul.f32   q3,    q11,    q1   \n" /* vmulq_f32 */        \
  "vbif       q11,   q3,    q2   \n"                         \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W16_v7_HARD_SWISH      \
  /* do relu */                                \
  "12: \n"                                     \
  "vmov.u32   q0,   #0                     \n" \
  "vld1.f32   {d2-d5}, [%[hs_param]]!      \n" \
  "vld1.f32   {d6-d7}, [%[hs_param]]       \n" \
  "vadd.f32   q4, q8, q1                \n"    \
  "vadd.f32   q6, q9, q1                \n"    \
  "vmul.f32   q5, q8, q2                \n"    \
  "vmul.f32   q7, q9, q2                \n"    \
  "vmax.f32   q4, q4, q0                \n"    \
  "vmax.f32   q6, q6, q0                \n"    \
  "vmin.f32   q4, q4, q3                \n"    \
  "vmin.f32   q6, q6, q3                \n"    \
  "vmul.f32   q8, q5, q4                \n"    \
  "vmul.f32   q9, q7, q6                \n"    \
  "vadd.f32   q4, q10, q1                \n"   \
  "vadd.f32   q6, q11, q1                \n"   \
  "vmul.f32   q5, q10, q2                \n"   \
  "vmul.f32   q7, q11, q2                \n"   \
  "vmax.f32   q4, q4, q0                \n"    \
  "vmax.f32   q6, q6, q0                \n"    \
  "vmin.f32   q4, q4, q3                \n"    \
  "vmin.f32   q6, q6, q3                \n"    \
  "vmul.f32   q10, q5, q4                \n"   \
  "vmul.f32   q11, q7, q6                \n"   \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx16, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx16, and the required data is
 * MxKxKx16.
 */
#define SPARSE_INT8_F32_W16_v7_OUT                                            \
  SPARSE_INT8_F32_W16_v7_KERNEL SPARSE_INT8_TRANS_INT32_TO_FP32_W16_v7        \
      SPARSE_INT8_F32_W16_v7_RELU SPARSE_INT8_F32_W16_v7_RELU6                \
          SPARSE_INT8_F32_W16_v7_LEAKY_RELU SPARSE_INT8_F32_W16_v7_HARD_SWISH \
      "mov   r0,   %[c_ptr]\n" /* store result */                             \
      "vst1.32   {d16-d19},  [r0]\n"                                          \
      "add  r0,  r0,   #32\n"                                                 \
      "vst1.32   {d20-d23},  [r0]\n"

#define SPARSE_INT8_F32_W8_v7_KERNEL   \
  "veor   q4,  q0,  q0\n"              \
  "veor   q5,  q1,  q1\n"              \
  "cmp    %[k],    #0\n"               \
  "beq    1f\n" /* main loop*/         \
  "0:\n"                               \
  "ldrsb   r0, [%[a_ptr]], #1\n"       \
  "ldr     r1, [%[widx_dmap]],   #4\n" \
  "vld1.8  d2, [%[b_ptr]]\n"           \
  "vdup.8    d0,   r0\n"               \
  "vmull.s8    q2,    d2,  d0\n"       \
  "add   %[b_ptr],  %[b_ptr], r1\n"    \
  "subs    %[k],   %[k],   #1\n"       \
  "vaddw.s16   q4,    q4,  d4\n"       \
  "vaddw.s16   q5,    q5,  d5\n"       \
  "bne     0b\n"                       \
  "1:\n"

#define SPARSE_INT8_TRANS_INT32_TO_FP32_W8_v7      \
  /* write output */                               \
  "vcvt.f32.s32   q0, q4\n" /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q1, q5\n" /* cvt int32 to fp32*/ \
  "vdup.32    q2,   %[vscale]\n"                   \
  "vdup.32    q6,   %[vbias]\n"                    \
  "vdup.32    q7,   d12[0]\n"                      \
  "3:\n"                                           \
  "vmla.f32  q6,   q0,  q2\n"                      \
  "vmla.f32  q7,   q1,  q2\n"

#define SPARSE_INT8_F32_W8_v7_RELU                    \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q0, #0\n"              /* for relu */   \
  "vmax.f32   q6,   q6,   q0\n"      /* relu */       \
  "vmax.f32   q7,   q7,   q0\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W8_v7_RELU6                    \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q6,   q6,   q0\n"      /* relu6 */       \
  "vmax.f32   q7,   q7,   q0\n"      /* relu6 */       \
  "vmin.f32   q6,   q6,   q1\n"      /* relu6 */       \
  "vmin.f32   q7,   q7,   q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W8_v7_LEAKY_RELU                     \
  /* do relu */                                              \
  "11: \n"                                                   \
  "cmp   %[vflag_act],  #3       \n" /* check leakey relu */ \
  "bne   12f                     \n" /* no act end */        \
  "vmov.i32   q0, #0\n"              /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"      /* leakey relu alpha */ \
  "vcge.f32   q2,    q6,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q6,    q1   \n" /* vmulq_f32 */         \
  "vcge.f32   q4,    q7,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q5,    q7,    q1   \n" /* vmulq_f32 */         \
  "vbif       q6,    q3,    q2   \n" /* vmulq_f32 */         \
  "vbif       q7,    q5,    q4   \n" /* vmulq_f32 */         \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W8_v7_HARD_SWISH       \
  /* do relu */                                \
  "12: \n"                                     \
  "vmov.u32   q0,   #0                     \n" \
  "vld1.f32   {d2-d5}, [%[hs_param]]!      \n" \
  "vld1.f32   {d6-d7}, [%[hs_param]]       \n" \
  "vadd.f32   q4, q6, q1                \n"    \
  "vadd.f32   q8, q7, q1                \n"    \
  "vmul.f32   q5, q6, q2                \n"    \
  "vmul.f32   q9, q7, q2                \n"    \
  "vmax.f32   q4, q4, q0                \n"    \
  "vmax.f32   q8, q8, q0                \n"    \
  "vmin.f32   q4, q4, q3                \n"    \
  "vmin.f32   q8, q8, q3                \n"    \
  "vmul.f32   q6, q5, q4                \n"    \
  "vmul.f32   q7, q9, q8                \n"    \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx8, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx8, and the required data is
 * MxKxKx8.
 */
#define SPARSE_INT8_F32_W8_v7_OUT                                    \
  SPARSE_INT8_F32_W8_v7_KERNEL SPARSE_INT8_TRANS_INT32_TO_FP32_W8_v7 \
      SPARSE_INT8_F32_W8_v7_RELU SPARSE_INT8_F32_W8_v7_RELU6         \
          SPARSE_INT8_F32_W8_v7_LEAKY_RELU /* store result */        \
      SPARSE_INT8_F32_W8_v7_HARD_SWISH "vst1.32   {d12-d15},  [%[c_ptr]]\n"

#define SPARSE_INT8_F32_W4_v7_KERNEL   \
  "veor   q3,  q0,  q0\n"              \
  "cmp    %[k],    #0\n"               \
  "beq    1f\n" /* main loop*/         \
  "0:\n"                               \
  "ldrsb   r0, [%[a_ptr]], #1\n"       \
  "vdup.8     d0,    r0\n"             \
  "subs    %[k],   %[k],   #1\n"       \
  "ldr     r0, [%[b_ptr]]\n"           \
  "vdup.32    d2,   r0\n"              \
  "vmull.s8    q2,    d2,  d0\n"       \
  "ldr     r1, [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], r1\n"    \
  "vaddw.s16   q3,    q3,  d4\n"       \
  "bne     0b\n"                       \
  "1:\n"

#define SPARSE_INT8_TRANS_INT32_TO_FP32_W4_v7      \
  /* write output */                               \
  "vcvt.f32.s32   q0, q3\n" /* cvt int32 to fp32*/ \
  "vdup.32    q1,   %[vscale]\n"                   \
  "vdup.32    q4,   %[vbias]\n"                    \
  "3:\n"                                           \
  "vmla.f32  q4,   q0,  q1\n"

#define SPARSE_INT8_F32_W4_v7_RELU                    \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q0, #0\n"              /* for relu */   \
  "vmax.f32   q4,   q4,   q0\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W4_v7_RELU6                    \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q4,   q4,   q0\n"      /* relu6 */       \
  "vmin.f32   q4,   q4,   q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_F32_W4_v7_LEAKY_RELU                     \
  /* do relu */                                              \
  "11: \n"                                                   \
  "cmp   %[vflag_act],  #3       \n" /* check leakey relu */ \
  "bne   12f                     \n" /* no act end */        \
  "vmov.i32   q0, #0\n"              /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"      /* leakey relu alpha */ \
  "vcge.f32   q2,    q4,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q4,    q1   \n" /* vmulq_f32 */         \
  "vbif       q4,    q3,    q2   \n"                         \
  "b      9f                    \n"

#define SPARSE_INT8_F32_W4_v7_HARD_SWISH       \
  /* do relu */                                \
  "12: \n"                                     \
  "vmov.u32   q0,   #0                     \n" \
  "vld1.f32   {d2-d5}, [%[hs_param]]!      \n" \
  "vld1.f32   {d6-d7}, [%[hs_param]]       \n" \
  "vadd.f32   q5, q4, q1                \n"    \
  "vmul.f32   q6, q4, q2                \n"    \
  "vmax.f32   q5, q5, q0                \n"    \
  "vmin.f32   q5, q5, q3                \n"    \
  "vmul.f32   q4, q6, q5                \n"    \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx4, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx4, and the required data is
 * MxKxKx4.
 */
#define SPARSE_INT8_F32_W4_v7_OUT                                    \
  SPARSE_INT8_F32_W4_v7_KERNEL SPARSE_INT8_TRANS_INT32_TO_FP32_W4_v7 \
      SPARSE_INT8_F32_W4_v7_RELU SPARSE_INT8_F32_W4_v7_RELU6         \
          SPARSE_INT8_F32_W4_v7_LEAKY_RELU /* store result */        \
      SPARSE_INT8_F32_W4_v7_HARD_SWISH "vst1.32   {d8-d9},  [%[c_ptr]]\n"

/**
 * \brief Sparse calculation implementation of 1x1 convolution, the input-output
 * type is int8-f32.
 * Sparse matrix multiplication is calculated in blocks, the block size is Mx48,
 * that is,
 * the parameter matrix is MxK, and the activation matrix is Kx48; when N is
 * less than 48,
 * it is calculated in blocks of Mx32, Mx16, Mx8, and Mx4 in turn;
 * @param A sparse weight data
 * @param B dense input data
 * @param widx_dmap An array of int32_t values storing scaled [by sizeof(input
 * element)] difference
 * between input channels corresponding to successive non-zero element
 * @param nidx_nnzmap the number of non-zero kernel elements per each output
 * channel
 * @param bias
 * @param output
 * @param M
 * @param N
 * @param K
 * @param param
 * @param ctx
 */
void sparse_conv_int8_fp32_pipelined(const int8_t* A,
                                     const int8_t* B,
                                     const int32_t* widx_dmap,
                                     const uint32_t* nidx_nnzmap,
                                     const float* bias,
                                     const float* scale,
                                     float* output,
                                     int M,
                                     int K,
                                     int N,
                                     const operators::SparseConvParam& param,
                                     ARMContext* ctx) {
  auto act_param = param.activation_param;
  auto act_type = act_param.active_type;
  volatile float alpha = 0.f;
  float vhs_param[12] = {0.f};
  int flag_act = 0x00;  // relu: 1, relu6: 2, leakey: 3
  if (act_param.has_active) {
    if (act_type == lite_api::ActivationType::kRelu) {
      flag_act = 0x01;
    } else if (act_type == lite_api::ActivationType::kRelu6) {
      flag_act = 0x02;
      alpha = act_param.Relu_clipped_coef;
    } else if (act_type == lite_api::ActivationType::kLeakyRelu) {
      flag_act = 0x03;
      alpha = act_param.Leaky_relu_alpha;
    } else if (act_type == lite_api::ActivationType::kHardSwish) {
      flag_act = 0x04;
      for (int i = 0; i < 4; i++) {
        vhs_param[i] = act_param.hard_swish_offset;
        vhs_param[i + 4] = 1.0 / act_param.hard_swish_scale;
        vhs_param[i + 8] = act_param.hard_swish_threshold;
      }
    }
  }
  int flag_bias = (bias != nullptr) ? 1 : 0;
  size_t mc = N * sizeof(int8_t);
  size_t nc = M;
  size_t output_stride = N * sizeof(float);
  size_t output_decrement = output_stride * nc - 48 * sizeof(float);

  while
    SPARSE_LIKELY(mc >= 48 * sizeof(int8_t)) {
      LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
        float* cur_output =
            reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
        const int8_t* cur_w = A;
        uint32_t nnz = nidx_nnzmap[i];
        const int8_t* cur_b = B;
        const int32_t* dmap = widx_dmap;
        if (i != 0) {
          cur_w = A + nidx_nnzmap[i - 1];
          nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];
          cur_b +=
              ((nidx_nnzmap[i - 1] == 0) ? 0
                                         : widx_dmap[nidx_nnzmap[i - 1] - 1]);
          dmap = widx_dmap + nidx_nnzmap[i - 1];
        }
        float vsclae = scale[i];
        float vbias = (bias != nullptr) ? bias[i] : 0.0;
        float* hs_param = vhs_param;
        // clang-format off
          asm volatile(SPARSE_INT8_F32_W48_v7_OUT
            : [a_ptr] "+r"(cur_w),
              [b_ptr] "+r"(cur_b),
              [c_ptr] "+r"(cur_output),
              [k] "+r"(nnz),
              [widx_dmap] "+r"(dmap),
              [hs_param] "+r"(hs_param)
            : [vscale] "r"(vsclae),
              [vbias] "r"(vbias),
              [vflag_act] "r"(flag_act),
              [valpha] "r"(alpha)
            : "q0",
              "q1",
              "q2",
              "q3",
              "q4",
              "q5",
              "q6",
              "q7",
              "q8",
              "q9",
              "q10",
              "q11",
              "q12",
              "q13",
              "q14",
              "q15",
              "r0",
              "r1",
              "r2",
              "cc",
              "memory");
        // clang-format on
      }
      LITE_PARALLEL_COMMON_END();
      output = reinterpret_cast<float*>((uintptr_t)output + 48 * sizeof(float));
      B += 48;
      mc -= 48 * sizeof(int8_t);
    }
  if
    SPARSE_UNLIKELY(mc != 0) {
      output_decrement += 16 * sizeof(float);
      if (mc & (32 * sizeof(int8_t))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const int8_t* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const int8_t* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            cur_w = A + nidx_nnzmap[i - 1];
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];
            cur_b +=
                ((nidx_nnzmap[i - 1] == 0) ? 0
                                           : widx_dmap[nidx_nnzmap[i - 1] - 1]);
            dmap = widx_dmap + nidx_nnzmap[i - 1];
          }
          float vsclae = scale[i];
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float* hs_param = vhs_param;
          // clang-format off
            asm volatile(SPARSE_INT8_F32_W32_v7_OUT
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vscale] "r"(vsclae),
                [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha)
              : "q0",
                "q1",
                "q2",
                "q3",
                "q4",
                "q5",
                "q6",
                "q7",
                "q8",
                "q9",
                "q10",
                "q11",
                "q12",
                "q13",
                "q14",
                "q15",
                "r0",
                "r1",
                "r2",
                "cc",
                "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 32 * sizeof(float));
        B += 32;
        mc -= 32 * sizeof(int8_t);
      }
      output_decrement += 16 * sizeof(float);
      if (mc & (16 * sizeof(int8_t))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const int8_t* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const int8_t* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            cur_w = A + nidx_nnzmap[i - 1];
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];
            cur_b +=
                ((nidx_nnzmap[i - 1] == 0) ? 0
                                           : widx_dmap[nidx_nnzmap[i - 1] - 1]);
            dmap = widx_dmap + nidx_nnzmap[i - 1];
          }
          float vsclae = scale[i];
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float* hs_param = vhs_param;
          // clang-format off
            asm volatile(SPARSE_INT8_F32_W16_v7_OUT
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vscale] "r"(vsclae),
                [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha)
              : "q0",
                "q1",
                "q2",
                "q3",
                "q4",
                "q5",
                "q6",
                "q7",
                "q8",
                "q9",
                "q10",
                "q11",
                "r0",
                "r1",
                "r2",
                "cc",
                "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 16 * sizeof(float));
        B += 16;
        mc -= 16 * sizeof(int8_t);
      }
      output_decrement += 8 * sizeof(float);
      if (mc & (8 * sizeof(int8_t))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const int8_t* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const int8_t* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            cur_w = A + nidx_nnzmap[i - 1];
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];
            cur_b +=
                ((nidx_nnzmap[i - 1] == 0) ? 0
                                           : widx_dmap[nidx_nnzmap[i - 1] - 1]);
            dmap = widx_dmap + nidx_nnzmap[i - 1];
          }
          float vsclae = scale[i];
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float* hs_param = vhs_param;
          // clang-format off
            asm volatile(SPARSE_INT8_F32_W8_v7_OUT
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vscale] "r"(vsclae),
                [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha)
              : "q0",
                "q1",
                "q2",
                "q3",
                "q4",
                "q5",
                "q6",
                "q7",
                "q8",
                "q9",
                "r0",
                "r1",
                "r2",
                "cc",
                "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 8 * sizeof(float));
        B += 8;
        mc -= 8 * sizeof(int8_t);
      }
      output_decrement += 4 * sizeof(float);
      if (mc & (4 * sizeof(int8_t))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          float* cur_output =
              reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
          const int8_t* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const int8_t* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            cur_w = A + nidx_nnzmap[i - 1];
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];
            cur_b +=
                ((nidx_nnzmap[i - 1] == 0) ? 0
                                           : widx_dmap[nidx_nnzmap[i - 1] - 1]);
            dmap = widx_dmap + nidx_nnzmap[i - 1];
          }
          float vsclae = scale[i];
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float* hs_param = vhs_param;
          // clang-format off
            asm volatile(SPARSE_INT8_F32_W4_v7_OUT
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vscale] "r"(vsclae),
                [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha)
              : "q0",
                "q1",
                "q2",
                "q3",
                "q4",
                "q5",
                "q6",
                "r0",
                "r1",
                "r2",
                "cc",
                "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<float*>((uintptr_t)output + 4 * sizeof(float));
        B += 4;
        mc -= 4 * sizeof(int8_t);
      }

      if
        SPARSE_UNLIKELY(mc != 0 && mc < 4 * sizeof(int8_t)) {
          int mindex = mc / sizeof(int8_t);

          LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
            float* out_ptr =
                reinterpret_cast<float*>((uintptr_t)output + output_stride * i);
            const int8_t* cur_w = A;
            uint32_t nnz = nidx_nnzmap[i];
            const int8_t* cur_b = B;
            const int32_t* dmap = widx_dmap;
            if (i != 0) {
              cur_w = A + nidx_nnzmap[i - 1];
              nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];
              cur_b += ((nidx_nnzmap[i - 1] == 0)
                            ? 0
                            : widx_dmap[nidx_nnzmap[i - 1] - 1]);
              dmap = widx_dmap + nidx_nnzmap[i - 1];
            }
            float32x4_t vbias =
                (bias != nullptr) ? vdupq_n_f32(bias[i]) : vdupq_n_f32(0);
            float vscale = scale[i];
            int32x4_t vacc0123 = vdupq_n_s32(0);
            for (int j = 0; j < nnz; j++) {
              int8x8_t vi0123 = vdup_n_s8(0);
              if (mc == 1) {
                vi0123 = vld1_lane_s8(cur_b, vi0123, 0);
              } else if (mc == 2) {
                vi0123 = vld1_lane_s8(cur_b, vi0123, 0);
                vi0123 = vld1_lane_s8(cur_b + 1, vi0123, 1);
              } else {
                vi0123 = vld1_lane_s8(cur_b, vi0123, 0);
                vi0123 = vld1_lane_s8(cur_b + 1, vi0123, 1);
                vi0123 = vld1_lane_s8(cur_b + 2, vi0123, 2);
              }
              int8x8_t vw = vld1_dup_s8(cur_w);
              cur_w += 1;
              intptr_t diff = *dmap++;
              cur_b = (const int8_t*)((uintptr_t)cur_b + (uintptr_t)diff);
              int16x8_t vo0123 = vmull_s8(vi0123, vw);
              vacc0123 = vaddw_s16(vacc0123, vget_low_s16(vo0123));
            }

            float32x4_t vaccf0123 = vcvtq_f32_s32(vacc0123);
            vaccf0123 = vmlaq_n_f32(vbias, vaccf0123, vscale);
            float32x4_t vzero = vdupq_n_f32(0);
            if (flag_act == 1) {
              vaccf0123 = vmaxq_f32(vaccf0123, vzero);
            } else if (flag_act == 0) {
            } else if (flag_act == 2) {
              float32x4_t aph = vdupq_n_f32(alpha);
              vaccf0123 = vmaxq_f32(vaccf0123, vzero);
              vaccf0123 = vminq_f32(vaccf0123, aph);
            } else if (flag_act == 3) {
              float32x4_t aph = vdupq_n_f32(alpha);
              uint32x4_t vflag0123 = vcgeq_f32(vaccf0123, vzero);
              float32x4_t v0123 = vmulq_f32(vaccf0123, aph);
              vaccf0123 = vbslq_f32(vflag0123, vaccf0123, v0123);
            } else {
              float32x4_t offset = vdupq_n_f32(act_param.hard_swish_offset);
              float32x4_t hs_scale =
                  vdupq_n_f32(1.0 / act_param.hard_swish_scale);
              float32x4_t thre = vdupq_n_f32(act_param.hard_swish_threshold);
              float32x4_t vset0123 = vaddq_f32(vaccf0123, offset);
              float32x4_t vscale0123 = vmulq_f32(vaccf0123, hs_scale);
              vset0123 = vmaxq_f32(vset0123, vzero);
              vset0123 = vminq_f32(vset0123, thre);
              vaccf0123 = vmulq_f32(vscale0123, vset0123);
            }

            if (mc == 1) {
              vst1q_lane_f32(out_ptr, vaccf0123, 0);
            } else if (mc == 2) {
              vst1q_lane_f32(out_ptr, vaccf0123, 0);
              vst1q_lane_f32(out_ptr + 1, vaccf0123, 1);
            } else {
              vst1q_lane_f32(out_ptr, vaccf0123, 0);
              vst1q_lane_f32(out_ptr + 1, vaccf0123, 1);
              vst1q_lane_f32(out_ptr + 2, vaccf0123, 2);
            }
          }
          LITE_PARALLEL_COMMON_END();
        }
    }
}

#define SPARSE_INT8_INT8_W48_v7_KERNEL \
  "veor   q4,   q0,  q0\n"             \
  "veor   q5,   q1,  q1\n"             \
  "veor   q6,   q2,  q2\n"             \
  "veor   q7,   q3,  q3\n"             \
  "pld  [%[a_ptr], #32]    \n"         \
  "veor   q8,   q0,  q0\n"             \
  "veor   q9,   q1,  q1\n"             \
  "pld  [%[widx_dmap], #32]    \n"     \
  "veor   q10,  q2,  q2\n"             \
  "veor   q11,  q3,  q3\n"             \
  "veor   q12,  q0,  q0\n"             \
  "pld  [%[b_ptr], #64]    \n"         \
  "veor   q13,  q1,  q1\n"             \
  "veor   q14,  q2,  q2\n"             \
  "veor   q15,  q3,  q3\n"             \
  "cmp    %[k],    #0\n"               \
  "beq    1f\n" /* main loop*/         \
  "0:\n"                               \
  "ldrsb   r0, [%[a_ptr]], #1\n"       \
  "subs    %[k],   %[k],   #1\n"       \
  "mov   r1,   %[b_ptr]\n"             \
  "vld1.8  {d2-d5}, [r1]\n"            \
  "vdup.8    d0,   r0\n"               \
  "vmull.s8    q3,    d2,  d0\n"       \
  "vaddw.s16   q4,    q4,  d6\n"       \
  "vaddw.s16   q5,    q5,  d7\n"       \
  "vmull.s8    q3,    d3,  d0\n"       \
  "pld  [%[widx_dmap], #32]    \n"     \
  "vaddw.s16   q6,    q6,  d6\n"       \
  "vaddw.s16   q7,    q7,  d7\n"       \
  "vmull.s8    q3,    d4,  d0\n"       \
  "vaddw.s16   q8,    q8, d6\n"        \
  "vaddw.s16   q9,    q9, d7\n"        \
  "vmull.s8    q3,    d5,  d0\n"       \
  "ldr     r0, [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], r0\n"    \
  "vaddw.s16   q10,   q10, d6\n"       \
  "vaddw.s16   q11,   q11, d7\n"       \
  "pld  [%[b_ptr], #64]    \n"         \
  "add  r1,  r1,   #32\n"              \
  "vld1.8  {d2-d3}, [r1]\n"            \
  "vmull.s8    q3,    d2,  d0\n"       \
  "vaddw.s16   q12,   q12,  d6\n"      \
  "pld  [%[a_ptr], #32]    \n"         \
  "vaddw.s16   q13,   q13,  d7\n"      \
  "vmull.s8    q3,    d3,  d0\n"       \
  "vaddw.s16   q14,   q14,  d6\n"      \
  "vaddw.s16   q15,   q15,  d7\n"      \
  "bne     0b\n"                       \
  "1:\n"

#define SPARSE_INT8_TRANS_INT32_TO_INT8_W48_v7      \
  /* write output */                                \
  "vdup.32    q0,   %[vscale]\n"                    \
  "vdup.32    q1,   %[vbias]\n"                     \
  "vcvt.f32.s32   q2, q4\n" /* cvt int32 to fp32*/  \
  "vcvt.f32.s32   q3, q5\n" /* cvt int32 to fp32*/  \
  "vdup.32    q4,   d2[0]\n"                        \
  "vdup.32    q5,   d2[0]\n"                        \
  "vmla.f32   q4,  q2,  q0\n"                       \
  "vmla.f32   q5,  q3,  q0\n"                       \
  "vcvt.f32.s32   q2, q6\n" /* cvt int32 to fp32*/  \
  "vcvt.f32.s32   q3, q7\n" /* cvt int32 to fp32*/  \
  "vdup.32    q6,   d2[0]\n"                        \
  "vdup.32    q7,   d2[0]\n"                        \
  "vmla.f32   q6,  q2,  q0\n"                       \
  "vmla.f32   q7,  q3,  q0\n"                       \
  "vcvt.f32.s32   q2, q8\n" /* cvt int32 to fp32*/  \
  "vcvt.f32.s32   q3, q9\n" /* cvt int32 to fp32*/  \
  "vdup.32    q8,   d2[0]\n"                        \
  "vdup.32    q9,   d2[0]\n"                        \
  "vmla.f32   q8,  q2,  q0\n"                       \
  "vmla.f32   q9,  q3,  q0\n"                       \
  "vcvt.f32.s32   q2, q10\n" /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q3, q11\n" /* cvt int32 to fp32*/ \
  "vdup.32    q10,   d2[0]\n"                       \
  "vdup.32    q11,   d2[0]\n"                       \
  "vmla.f32   q10,  q2,  q0\n"                      \
  "vmla.f32   q11,  q3,  q0\n"                      \
  "vcvt.f32.s32   q2, q12\n" /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q3, q13\n" /* cvt int32 to fp32*/ \
  "vdup.32    q12,   d2[0]\n"                       \
  "vdup.32    q13,   d2[0]\n"                       \
  "vmla.f32   q12,  q2,  q0\n"                      \
  "vmla.f32   q13,  q3,  q0\n"                      \
  "vcvt.f32.s32   q2, q14\n" /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q3, q15\n" /* cvt int32 to fp32*/ \
  "vdup.32    q14,   d2[0]\n"                       \
  "vdup.32    q15,   d2[0]\n"                       \
  "vmla.f32   q14,  q2,  q0\n"                      \
  "vmla.f32   q15,  q3,  q0\n"

#define SPARSE_INT8_INT8_W48_v7_RELU                  \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q0, #0\n"              /* for relu */   \
  "vmax.f32   q4,   q4,   q0\n"      /* relu */       \
  "vmax.f32   q5,   q5,   q0\n"      /* relu */       \
  "vmax.f32   q6,   q6,   q0\n"      /* relu */       \
  "vmax.f32   q7,   q7,   q0\n"      /* relu */       \
  "vmax.f32   q8,   q8,   q0\n"      /* relu */       \
  "vmax.f32   q9,   q9,   q0\n"      /* relu */       \
  "vmax.f32   q10,  q10,  q0\n"      /* relu */       \
  "vmax.f32   q11,  q11,  q0\n"      /* relu */       \
  "vmax.f32   q12,  q12,  q0\n"      /* relu */       \
  "vmax.f32   q13,  q13,  q0\n"      /* relu */       \
  "vmax.f32   q14,  q14,  q0\n"      /* relu */       \
  "vmax.f32   q15,  q15,  q0\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W48_v7_RELU6                  \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q4,   q4,   q0\n"      /* relu6 */       \
  "vmax.f32   q5,   q5,   q0\n"      /* relu6 */       \
  "vmax.f32   q6,   q6,   q0\n"      /* relu6 */       \
  "vmax.f32   q7,   q7,   q0\n"      /* relu6 */       \
  "vmax.f32   q8,   q8,   q0\n"      /* relu6 */       \
  "vmax.f32   q9,   q9,   q0\n"      /* relu6 */       \
  "vmax.f32   q10,  q10,  q0\n"      /* relu6 */       \
  "vmax.f32   q11,  q11,  q0\n"      /* relu6 */       \
  "vmax.f32   q12,  q12,  q0\n"      /* relu6 */       \
  "vmax.f32   q13,  q13,  q0\n"      /* relu6 */       \
  "vmax.f32   q14,  q14,  q0\n"      /* relu6 */       \
  "vmax.f32   q15,  q15,  q0\n"      /* relu6 */       \
  "vmin.f32   q4,   q4,   q1\n"      /* relu6 */       \
  "vmin.f32   q5,   q5,   q1\n"      /* relu6 */       \
  "vmin.f32   q6,   q6,   q1\n"      /* relu6 */       \
  "vmin.f32   q7,   q7,   q1\n"      /* relu6 */       \
  "vmin.f32   q8,   q8,   q1\n"      /* relu6 */       \
  "vmin.f32   q9,   q9,   q1\n"      /* relu6 */       \
  "vmin.f32   q10,  q10,  q1\n"      /* relu6 */       \
  "vmin.f32   q11,  q11,  q1\n"      /* relu6 */       \
  "vmin.f32   q12,  q12,  q1\n"      /* relu6 */       \
  "vmin.f32   q13,  q13,  q1\n"      /* relu6 */       \
  "vmin.f32   q14,  q14,  q1\n"      /* relu6 */       \
  "vmin.f32   q15,  q15,  q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W48_v7_LEAKY_RELU                     \
  /* do relu */                                                \
  "11: \n"                                                     \
  "cmp   %[vflag_act],  #3       \n"   /* check leakey relu */ \
  "bne   12f                     \n"   /* no act end */        \
  "vmov.i32   q0, #0\n"                /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"        /* leakey relu alpha */ \
  "vcge.f32   q2,    q4,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q4,    q1     \n" /* vmulq_f32 */         \
  "vbif       q4,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q5,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q5,    q1     \n" /* vmulq_f32 */         \
  "vbif       q5,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q6,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q6,    q1     \n" /* vmulq_f32 */         \
  "vbif       q6,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q7,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q7,    q1     \n" /* vmulq_f32 */         \
  "vbif       q7,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q8,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q8,    q1     \n" /* vmulq_f32 */         \
  "vbif       q8,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q9,    q0     \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q9,    q1     \n" /* vmulq_f32 */         \
  "vbif       q9,    q3,    q2     \n"                         \
  "vcge.f32   q2,    q10,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q10,    q1    \n" /* vmulq_f32 */         \
  "vbif       q10,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q11,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q11,    q1    \n" /* vmulq_f32 */         \
  "vbif       q11,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q12,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q12,    q1    \n" /* vmulq_f32 */         \
  "vbif       q12,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q13,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q13,    q1    \n" /* vmulq_f32 */         \
  "vbif       q13,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q14,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q14,    q1    \n" /* vmulq_f32 */         \
  "vbif       q14,    q3,    q2    \n"                         \
  "vcge.f32   q2,    q15,    q0    \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q15,    q1    \n" /* vmulq_f32 */         \
  "vbif       q15,    q3,    q2    \n"                         \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W48_v7_HARD_SWISH                            \
  /* do relu */                                                       \
  "12: \n"                                                            \
  "vld1.f32   {d0-d3}, [%[hs_param]]!      @ load hard swish alpha\n" \
  "vadd.f32   q3, q4, q0                \n"                           \
  "vmul.f32   q4, q4, q1                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]       \n"                        \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q4, q4, q3                \n"                           \
  "vadd.f32   q3, q5, q0                \n"                           \
  "vmul.f32   q5, q5, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]       \n"                        \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q5, q5, q3                \n"                           \
  "vadd.f32   q3, q6, q0                \n"                           \
  "vmul.f32   q6, q6, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]       \n"                        \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q6, q6, q3                \n"                           \
  "vadd.f32   q3, q7, q0                \n"                           \
  "vmul.f32   q7, q7, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q7, q7, q3                \n"                           \
  "vadd.f32   q3, q8, q0                \n"                           \
  "vmul.f32   q8, q8, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q8, q8, q3                \n"                           \
  "vadd.f32   q3, q9, q0                \n"                           \
  "vmul.f32   q9, q9, q1                \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q9, q9, q3                \n"                           \
  "vadd.f32   q3, q10, q0               \n"                           \
  "vmul.f32   q10, q10, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q10, q10, q3              \n"                           \
  "vadd.f32   q3, q11, q0               \n"                           \
  "vmul.f32   q11, q11, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q11, q11, q3              \n"                           \
  "vadd.f32   q3, q12, q0               \n"                           \
  "vmul.f32   q12, q12, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q12, q12, q3              \n"                           \
  "vadd.f32   q3, q13, q0               \n"                           \
  "vmul.f32   q13, q13, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q13, q13, q3              \n"                           \
  "vadd.f32   q3, q14, q0               \n"                           \
  "vmul.f32   q14, q14, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q14, q14, q3              \n"                           \
  "vadd.f32   q3, q15, q0               \n"                           \
  "vmul.f32   q15, q15, q1              \n"                           \
  "vmax.f32   q3, q3, q2                \n"                           \
  "vld1.f32   {d4-d5}, [%[hs_param]]    \n"                           \
  "vmin.f32   q3, q3, q2                \n"                           \
  "vmov.u32   q2,   #0                  @ for hardswish \n"           \
  "vmul.f32   q15, q15, q3              \n"                           \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx48, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx48, and the required data is
 * MxKxKx48.
 */
#define SPARSE_INT8_INT8_W48_v7_OUT                                     \
  SPARSE_INT8_INT8_W48_v7_KERNEL SPARSE_INT8_TRANS_INT32_TO_INT8_W48_v7 \
      SPARSE_INT8_INT8_W48_v7_RELU SPARSE_INT8_INT8_W48_v7_RELU6        \
          SPARSE_INT8_INT8_W48_v7_LEAKY_RELU                            \
              SPARSE_INT8_INT8_W48_v7_HARD_SWISH                        \
      "vmov.f32  q0, #-0.5\n"    /* neg offset */                       \
      "vmov.f32  q1, #0.5\n"     /* pos offset */                       \
      "vcgt.f32  q2, q4, #0\n"   /* get pos mask */                     \
      "vbif.f32  q1, q0, q2\n"   /* get right offset */                 \
      "vadd.f32  q4, q1, q4\n"   /* add offset */                       \
      "vmov.f32  q2, #0.5\n"     /* pos offset */                       \
      "vcgt.f32  q3, q5, #0\n"   /* get pos mask */                     \
      "vbif.f32  q2, q0, q3\n"   /* get right offset */                 \
      "vadd.f32  q5, q2, q5\n"   /* add offset */                       \
      "vmov.f32  q1, #0.5\n"     /* pos offset */                       \
      "vcgt.f32  q2, q6, #0\n"   /* get pos mask */                     \
      "vbif.f32  q1, q0, q2\n"   /* get right offset */                 \
      "vadd.f32  q6, q1, q6\n"   /* add offset */                       \
      "vmov.f32  q2, #0.5\n"     /* pos offset */                       \
      "vcgt.f32  q3, q7, #0\n"   /* get pos mask */                     \
      "vbif.f32  q2, q0, q3\n"   /* get right offset */                 \
      "vadd.f32  q7, q2, q7\n"   /* add offset */                       \
      "vmov.f32  q1, #0.5\n"     /* pos offset */                       \
      "vcgt.f32  q2, q8, #0\n"   /* get pos mask */                     \
      "vbif.f32  q1, q0, q2\n"   /* get right offset */                 \
      "vadd.f32  q8, q1, q8\n"   /* add offset */                       \
      "vmov.f32  q2, #0.5\n"     /* pos offset */                       \
      "vcgt.f32  q3, q9, #0\n"   /* get pos mask */                     \
      "vbif.f32  q2, q0, q3\n"   /* get right offset */                 \
      "vadd.f32  q9, q2, q9\n"   /* add offset */                       \
      "vmov.f32  q1, #0.5\n"     /* pos offset */                       \
      "vcgt.f32  q2, q10, #0\n"  /* get pos mask */                     \
      "vbif.f32  q1, q0, q2\n"   /* get right offset */                 \
      "vadd.f32  q10, q1, q10\n" /* add offset */                       \
      "vmov.f32  q2, #0.5\n"     /* pos offset */                       \
      "vcgt.f32  q3, q11, #0\n"  /* get pos mask */                     \
      "vbif.f32  q2, q0, q3\n"   /* get right offset */                 \
      "vadd.f32  q11, q2, q11\n" /* add offset */                       \
      "vmov.f32  q1, #0.5\n"     /* pos offset */                       \
      "vcgt.f32  q2, q12, #0\n"  /* get pos mask */                     \
      "vbif.f32  q1, q0, q2\n"   /* get right offset */                 \
      "vadd.f32  q12, q1, q12\n" /* add offset */                       \
      "vmov.f32  q2, #0.5\n"     /* pos offset */                       \
      "vcgt.f32  q3, q13, #0\n"  /* get pos mask */                     \
      "vbif.f32  q2, q0, q3\n"   /* get right offset */                 \
      "vadd.f32  q13, q2, q13\n" /* add offset */                       \
      "vmov.f32  q1, #0.5\n"     /* pos offset */                       \
      "vcgt.f32  q2, q14, #0\n"  /* get pos mask */                     \
      "vbif.f32  q1, q0, q2\n"   /* get right offset */                 \
      "vadd.f32  q14, q1, q14\n" /* add offset */                       \
      "vmov.f32  q2, #0.5\n"     /* pos offset */                       \
      "vcgt.f32  q3, q15, #0\n"  /* get pos mask */                     \
      "vbif.f32  q2, q0, q3\n"   /* get right offset */                 \
      "vadd.f32  q15, q2, q15\n" /* add offset */                       \
      "vld1.f32 {d0-d1}, [%[vmax]] \n"                                  \
      "vcge.f32 q1,  q4, q0\n"                                          \
      "vcge.f32 q2,  q5, q0\n"                                          \
      "vcge.f32 q3,  q6, q0\n"                                          \
      "vbif q4, q0, q1\n"                                               \
      "vbif q5, q0, q2\n"                                               \
      "vbif q6, q0, q3\n"                                               \
      "vcge.f32 q1,  q7, q0\n"                                          \
      "vcge.f32 q2,  q8, q0\n"                                          \
      "vcge.f32 q3,  q9, q0\n"                                          \
      "vbif q7, q0, q1\n"                                               \
      "vbif q8, q0, q2\n"                                               \
      "vbif q9, q0, q3\n"                                               \
      "vcge.f32 q1,  q10, q0\n"                                         \
      "vcge.f32 q2,  q11, q0\n"                                         \
      "vcge.f32 q3,  q12, q0\n"                                         \
      "vbif q10, q0, q1\n"                                              \
      "vbif q11, q0, q2\n"                                              \
      "vbif q12, q0, q3\n"                                              \
      "vcge.f32 q1,  q13, q0\n"                                         \
      "vcge.f32 q2,  q14, q0\n"                                         \
      "vcge.f32 q3,  q15, q0\n"                                         \
      "vbif q13, q0, q1\n"                                              \
      "vbif q14, q0, q2\n"                                              \
      "vbif q15, q0, q3\n"                                              \
      "vcvt.s32.f32   q0, q4\n"  /*     fp32->int32 */                  \
      "vcvt.s32.f32   q1, q5\n"  /*      fp32->int32 */                 \
      "vcvt.s32.f32   q2, q6\n"  /*      fp32->int32 */                 \
      "vcvt.s32.f32   q3, q7\n"  /*      fp32->int32 */                 \
      "vqmovn.s32 d8, q0\n"      /*     int32 -> int16 */               \
      "vqmovn.s32 d9, q1\n"      /*      int32 -> int16 */              \
      "vqmovn.s32 d10, q2\n"     /*      int32 -> int16 */              \
      "vqmovn.s32 d11, q3\n"     /*      int32 -> int16 */              \
      "vqmovn.s16 d12, q4\n"     /* 0, int16 -> int8 */                 \
      "vqmovn.s16 d13, q5\n"     /* 1, int16 -> int8 */                 \
      "vcvt.s32.f32   q0, q8\n"  /*     fp32->int32 */                  \
      "vcvt.s32.f32   q1, q9\n"  /*      fp32->int32 */                 \
      "vcvt.s32.f32   q2, q10\n" /*      fp32->int32 */                 \
      "vcvt.s32.f32   q3, q11\n" /*      fp32->int32 */                 \
      "vqmovn.s32 d8, q0\n"      /*     int32 -> int16 */               \
      "vqmovn.s32 d9, q1\n"      /*      int32 -> int16 */              \
      "vqmovn.s32 d10, q2\n"     /*      int32 -> int16 */              \
      "vqmovn.s32 d11, q3\n"     /*      int32 -> int16 */              \
      "vqmovn.s16 d14, q4\n"     /* 0, int16 -> int8 */                 \
      "vqmovn.s16 d15, q5\n"     /* 1, int16 -> int8 */                 \
      "vcvt.s32.f32   q0, q12\n" /*     fp32->int32 */                  \
      "vcvt.s32.f32   q1, q13\n" /*      fp32->int32 */                 \
      "vcvt.s32.f32   q2, q14\n" /*      fp32->int32 */                 \
      "vcvt.s32.f32   q3, q15\n" /*      fp32->int32 */                 \
      "vqmovn.s32 d8, q0\n"      /*     int32 -> int16 */               \
      "vqmovn.s32 d9, q1\n"      /*      int32 -> int16 */              \
      "vqmovn.s32 d10, q2\n"     /*      int32 -> int16 */              \
      "vqmovn.s32 d11, q3\n"     /*      int32 -> int16 */              \
      "vqmovn.s16 d16, q4\n"     /* 0, int16 -> int8 */                 \
      "vqmovn.s16 d17, q5\n"     /* 1, int16 -> int8 */                 \
      "mov   r0,   %[c_ptr]\n"   /* store result */                     \
      "vst1.32   {d12-d13},  [r0]\n"                                    \
      "add  r0,  r0,   #16\n"                                           \
      "vst1.32   {d14-d15},  [r0]\n"                                    \
      "add  r0,  r0,   #16\n"                                           \
      "vst1.32   {d16-d17},  [r0]\n"

#define SPARSE_INT8_INT8_W32_v7_KERNEL \
  "veor   q8,   q0,  q0\n"             \
  "veor   q9,   q1,  q1\n"             \
  "pld  [%[a_ptr], #32]    \n"         \
  "veor   q10,  q2,  q2\n"             \
  "veor   q11,  q3,  q3\n"             \
  "pld  [%[widx_dmap], #32]    \n"     \
  "veor   q12,  q4,  q4\n"             \
  "veor   q13,  q5,  q5\n"             \
  "pld  [%[b_ptr], #64]    \n"         \
  "veor   q14,  q6,  q6\n"             \
  "veor   q15,  q7,  q7\n"             \
  "cmp    %[k],    #0\n"               \
  "beq    1f\n" /* main loop*/         \
  "0:\n"                               \
  "ldrsb   r0, [%[a_ptr]], #1\n"       \
  "subs    %[k],   %[k],   #1\n"       \
  "vld1.8  {d2-d5}, [%[b_ptr]]\n"      \
  "vdup.8    d0,   r0\n"               \
  "vmull.s8    q3,    d2,  d0\n"       \
  "vmull.s8    q4,    d3,  d0\n"       \
  "pld  [%[widx_dmap], #32]    \n"     \
  "vmull.s8    q5,    d4,  d0\n"       \
  "vmull.s8    q6,    d5,  d0\n"       \
  "ldr     r1, [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], r1\n"    \
  "vaddw.s16   q8,     q8,  d6\n"      \
  "vaddw.s16   q9,     q9,  d7\n"      \
  "pld  [%[a_ptr], #32]    \n"         \
  "vaddw.s16   q10,    q10,  d8\n"     \
  "vaddw.s16   q11,    q11,  d9\n"     \
  "vaddw.s16   q12,    q12,  d10\n"    \
  "pld  [%[b_ptr], #64]    \n"         \
  "vaddw.s16   q13,    q13,  d11\n"    \
  "vaddw.s16   q14,    q14,  d12\n"    \
  "vaddw.s16   q15,    q15,  d13\n"    \
  "bne     0b\n"                       \
  "1:\n"

#define SPARSE_INT8_TRANS_INT32_TO_INT8_W32_v7       \
  /* write output */                                 \
  "vcvt.f32.s32   q0, q8\n"  /* cvt int32 to fp32*/  \
  "vcvt.f32.s32   q1, q9\n"  /* cvt int32 to fp32*/  \
  "vcvt.f32.s32   q2, q10\n" /* cvt int32 to fp32*/  \
  "vcvt.f32.s32   q3, q11\n" /* cvt int32 to fp32*/  \
  "vdup.32    q8,   %[vscale]\n"                     \
  "vdup.32    q4,   %[vbias]\n"                      \
  "vdup.32    q5,   d8[0]\n"                         \
  "vdup.32    q6,   d8[0]\n"                         \
  "vdup.32    q7,   d8[0]\n"                         \
  "3:\n"                                             \
  "vmla.f32  q4,  q0,  q8\n"                         \
  "vmla.f32  q5,  q1,  q8\n"                         \
  "vmla.f32  q6,  q2,  q8\n"                         \
  "vmla.f32  q7,  q3,  q8\n"                         \
  "4:\n"                                             \
  "vcvt.f32.s32   q8, q12\n"  /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q9, q13\n"  /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q10, q14\n" /* cvt int32 to fp32*/ \
  "vcvt.f32.s32   q11, q15\n" /* cvt int32 to fp32*/ \
  "vdup.32    q12,   %[vscale]\n"                    \
  "vdup.32    q0,   %[vbias]\n"                      \
  "vdup.32    q1,   d0[0]\n"                         \
  "vdup.32    q2,   d0[0]\n"                         \
  "vdup.32    q3,   d0[0]\n"                         \
  "6:\n"                                             \
  "vmla.f32  q0,  q8,  q12\n"                        \
  "vmla.f32  q1,  q9,  q12\n"                        \
  "vmla.f32  q2,  q10,  q12\n"                       \
  "vmla.f32  q3,  q11,  q12\n"

#define SPARSE_INT8_INT8_W32_v7_RELU                  \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q8, #0\n"              /* for relu */   \
  "vmax.f32   q0,   q0,   q8\n"      /* relu */       \
  "vmax.f32   q1,   q1,   q8\n"      /* relu */       \
  "vmax.f32   q2,   q2,   q8\n"      /* relu */       \
  "vmax.f32   q3,   q3,   q8\n"      /* relu */       \
  "vmax.f32   q4,   q4,   q8\n"      /* relu */       \
  "vmax.f32   q5,   q5,   q8\n"      /* relu */       \
  "vmax.f32   q6,   q6,   q8\n"      /* relu */       \
  "vmax.f32   q7,   q7,   q8\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W32_v7_RELU6                  \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q8,   #0\n"            /* for relu6 */   \
  "vdup.32    q9,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q0,   q0,   q8\n"      /* relu6 */       \
  "vmax.f32   q1,   q1,   q8\n"      /* relu6 */       \
  "vmax.f32   q2,   q2,   q8\n"      /* relu6 */       \
  "vmax.f32   q3,   q3,   q8\n"      /* relu6 */       \
  "vmax.f32   q4,   q4,   q8\n"      /* relu6 */       \
  "vmax.f32   q5,   q5,   q8\n"      /* relu6 */       \
  "vmax.f32   q6,   q6,   q8\n"      /* relu6 */       \
  "vmax.f32   q7,   q7,   q8\n"      /* relu6 */       \
  "vmin.f32   q0,   q0,   q9\n"      /* relu6 */       \
  "vmin.f32   q1,   q1,   q9\n"      /* relu6 */       \
  "vmin.f32   q2,   q2,   q9\n"      /* relu6 */       \
  "vmin.f32   q3,   q3,   q9\n"      /* relu6 */       \
  "vmin.f32   q4,   q4,   q9\n"      /* relu6 */       \
  "vmin.f32   q5,   q5,   q9\n"      /* relu6 */       \
  "vmin.f32   q6,   q6,   q9\n"      /* relu6 */       \
  "vmin.f32   q7,   q7,   q9\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W32_v7_LEAKY_RELU                    \
  /* do relu */                                               \
  "11: \n"                                                    \
  "cmp   %[vflag_act],  #3       \n"  /* check leakey relu */ \
  "bne   12f                     \n"  /* no act end */        \
  "vmov.i32   q8, #0\n"               /* for relu */          \
  "vdup.32    q9,  %[valpha]\n"       /* leakey relu alpha */ \
  "vcge.f32   q10,    q0,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q11,    q0,    q9   \n" /* vmulq_f32 */         \
  "vcge.f32   q12,    q1,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q13,    q1,    q9   \n" /* vmulq_f32 */         \
  "vcge.f32   q14,    q2,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q15,    q2,    q9   \n" /* vmulq_f32 */         \
  "vbif       q0,    q11,    q10   \n"                        \
  "vbif       q1,    q13,    q12   \n"                        \
  "vbif       q2,    q15,    q14   \n"                        \
  "vcge.f32   q10,    q3,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q11,    q3,    q9   \n" /* vmulq_f32 */         \
  "vcge.f32   q12,    q4,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q13,    q4,    q9   \n" /* vmulq_f32 */         \
  "vcge.f32   q14,    q5,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q15,    q5,    q9   \n" /* vmulq_f32 */         \
  "vbif       q3,    q11,    q10   \n"                        \
  "vbif       q4,    q13,    q12   \n"                        \
  "vbif       q5,    q15,    q14   \n"                        \
  "vcge.f32   q10,    q6,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q11,    q6,    q9   \n" /* vmulq_f32 */         \
  "vcge.f32   q12,    q7,    q8   \n" /* vcgeq_f32 */         \
  "vmul.f32   q13,    q7,    q9   \n" /* vmulq_f32 */         \
  "vbif       q6,    q11,    q10   \n"                        \
  "vbif       q7,    q13,    q12   \n"                        \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W32_v7_HARD_SWISH       \
  /* do relu */                                  \
  "12: \n"                                       \
  "vmov.u32   q8,   #0                     \n"   \
  "vld1.f32   {d18-d21}, [%[hs_param]]!      \n" \
  "vld1.f32   {d22-d23}, [%[hs_param]]       \n" \
  "vadd.f32   q12, q0, q9                 \n"    \
  "vadd.f32   q14, q1, q9                 \n"    \
  "vmul.f32   q13, q0, q10                \n"    \
  "vmul.f32   q14, q1, q10                \n"    \
  "vmax.f32   q12, q12, q8                \n"    \
  "vmax.f32   q14, q14, q8                \n"    \
  "vmin.f32   q12, q12, q11               \n"    \
  "vmin.f32   q14, q14, q11               \n"    \
  "vmul.f32   q0,  q13, q12               \n"    \
  "vmul.f32   q1,  q15, q14               \n"    \
  "vadd.f32   q12, q2, q9                 \n"    \
  "vadd.f32   q14, q3, q9                 \n"    \
  "vmul.f32   q13, q2, q10                \n"    \
  "vmul.f32   q14, q3, q10                \n"    \
  "vmax.f32   q12, q12, q8                \n"    \
  "vmax.f32   q14, q14, q8                \n"    \
  "vmin.f32   q12, q12, q11               \n"    \
  "vmin.f32   q14, q14, q11               \n"    \
  "vmul.f32   q2,  q13, q12               \n"    \
  "vmul.f32   q3,  q15, q14               \n"    \
  "vadd.f32   q12, q4, q9                 \n"    \
  "vadd.f32   q14, q5, q9                 \n"    \
  "vmul.f32   q13, q4, q10                \n"    \
  "vmul.f32   q14, q5, q10                \n"    \
  "vmax.f32   q12, q12, q8                \n"    \
  "vmax.f32   q14, q14, q8                \n"    \
  "vmin.f32   q12, q12, q11               \n"    \
  "vmin.f32   q14, q14, q11               \n"    \
  "vmul.f32   q4,  q13, q12               \n"    \
  "vmul.f32   q5,  q15, q14               \n"    \
  "vadd.f32   q12, q6, q9                 \n"    \
  "vadd.f32   q14, q7, q9                 \n"    \
  "vmul.f32   q13, q6, q10                \n"    \
  "vmul.f32   q14, q7, q10                \n"    \
  "vmax.f32   q12, q12, q8                \n"    \
  "vmax.f32   q14, q14, q8                \n"    \
  "vmin.f32   q12, q12, q11               \n"    \
  "vmin.f32   q14, q14, q11               \n"    \
  "vmul.f32   q6,  q13, q12               \n"    \
  "vmul.f32   q7,  q15, q14               \n"    \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx32, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx32, and the required data is
 * MxKxKx32.
 */
#define SPARSE_INT8_INT8_W32_v7_OUT                                            \
  SPARSE_INT8_INT8_W32_v7_KERNEL SPARSE_INT8_TRANS_INT32_TO_INT8_W32_v7        \
      SPARSE_INT8_INT8_W32_v7_RELU SPARSE_INT8_INT8_W32_v7_RELU6               \
          SPARSE_INT8_INT8_W32_v7_LEAKY_RELU                                   \
              SPARSE_INT8_INT8_W32_v7_HARD_SWISH                               \
      "vmov.f32  q8, #-0.5\n"    /* neg offset */                              \
      "vmov.f32  q10, #0.5\n"    /* pos offset */                              \
      "vmov.f32  q11, #0.5\n"    /* pos offset */                              \
      "vmov.f32  q12, #0.5\n"    /* pos offset */                              \
      "vmov.f32  q13, #0.5\n"    /* pos offset */                              \
      "vcgt.f32  q14, q0, #0\n"  /* get pos mask */                            \
      "vcgt.f32  q15, q1, #0\n"  /* get pos mask */                            \
      "vbif.f32  q10, q8, q14\n" /* get right offset */                        \
      "vbif.f32  q11, q8, q15\n" /* get right offset */                        \
      "vcgt.f32  q14, q2, #0\n"  /* get pos mask */                            \
      "vcgt.f32  q15, q3, #0\n"  /* get pos mask */                            \
      "vbif.f32  q12, q8, q14\n" /* get right offset */                        \
      "vbif.f32  q13, q8, q15\n" /* get right offset */                        \
      "vadd.f32  q0, q10, q0\n"  /* add offset */                              \
      "vadd.f32  q1, q11, q1\n"  /* add offset */                              \
      "vadd.f32  q2, q12, q2\n"  /* add offset */                              \
      "vadd.f32  q3, q13, q3\n"  /* add offset */                              \
      "vmov.f32  q10, #0.5\n"    /* pos offset */                              \
      "vmov.f32  q11, #0.5\n"    /* pos offset */                              \
      "vmov.f32  q12, #0.5\n"    /* pos offset */                              \
      "vmov.f32  q13, #0.5\n"    /* pos offset */                              \
      "vcgt.f32  q14, q4, #0\n"  /* get pos mask */                            \
      "vcgt.f32  q15, q5, #0\n"  /* get pos mask */                            \
      "vbif.f32  q10, q8, q14\n" /* get right offset */                        \
      "vbif.f32  q11, q8, q15\n" /* get right offset */                        \
      "vcgt.f32  q14, q6, #0\n"  /* get pos mask */                            \
      "vcgt.f32  q15, q7, #0\n"  /* get pos mask */                            \
      "vbif.f32  q12, q8, q14\n" /* get right offset */                        \
      "vbif.f32  q13, q8, q15\n" /* get right offset */                        \
      "vadd.f32  q4, q10, q4\n"  /*      add offset */                         \
      "vadd.f32  q5, q11, q5\n"  /*      add offset */                         \
      "vadd.f32  q6, q12, q6\n"  /*      add offset */                         \
      "vadd.f32  q7, q13, q7\n"  /*      add offset */                         \
      "vld1.f32 {d16-d17}, [%[vmax]] \n"                                       \
      "vcge.f32 q9,  q0, q8\n"                       /* @ q8 >= -127 \n */     \
      "vcge.f32 q10, q1, q8\n"                       /* @ q9 >= -127 \n */     \
      "vcge.f32 q11, q2, q8\n"                       /* @ q0 >= -127 \n */     \
      "vcge.f32 q12, q3, q8\n"                       /* @ q1 >= -127 \n */     \
      "vcge.f32 q13, q4, q8\n"                       /* @ q2 >= -127 \n */     \
      "vcge.f32 q14, q5, q8\n"                       /* @ q3 >= -127 \n */     \
      "vcge.f32 q15, q6, q8\n" /* @ q4 >= -127 \n */ /* choose data */         \
      "vbif q0, q8, q9\n"                            /* @ choose */            \
      "vcge.f32 q9,  q7, q8\n"                       /* @ q5 >= -127 \n */     \
      "vbif q1, q8, q10\n"                           /* @ choose */            \
      "vbif q2, q8, q11\n"                           /* @ choose */            \
      "vbif q3, q8, q12\n"                           /* @ choose */            \
      "vbif q4, q8, q13\n"                           /* @ choose */            \
      "vbif q5, q8, q14\n"                           /* @ choose */            \
      "vbif q6, q8, q15\n"                           /* @ choose */            \
      "vbif q7, q8, q9\n"                            /* @ choose */            \
      "vcvt.s32.f32   q8, q0\n"                      /*     fp32->int32 */     \
      "vcvt.s32.f32   q9, q1\n"                      /*      fp32->int32 */    \
      "vcvt.s32.f32   q10, q2\n"                     /*      fp32->int32 */    \
      "vcvt.s32.f32   q11, q3\n"                     /*      fp32->int32 */    \
      "vcvt.s32.f32   q12, q4\n"                     /*      fp32->int32 */    \
      "vcvt.s32.f32   q13, q5\n"                     /*      fp32->int32 */    \
      "vcvt.s32.f32   q14, q6\n"                     /*      fp32->int32 */    \
      "vcvt.s32.f32   q15, q7\n"                     /*      fp32->int32 */    \
      "vqmovn.s32 d0, q8\n"                          /*     int32 -> int16 */  \
      "vqmovn.s32 d1, q9\n"                          /*      int32 -> int16 */ \
      "vqmovn.s32 d2, q10\n"                         /*      int32 -> int16 */ \
      "vqmovn.s32 d3, q11\n"                         /*      int32 -> int16 */ \
      "vqmovn.s32 d4, q12\n"                         /*     int32 -> int16 */  \
      "vqmovn.s32 d5, q13\n"                         /*      int32 -> int16 */ \
      "vqmovn.s32 d6, q14\n"                         /*      int32 -> int16 */ \
      "vqmovn.s32 d7, q15\n"                         /*      int32 -> int16 */ \
      "vqmovn.s16 d8, q0\n"                          /* 0, int16 -> int8 */    \
      "vqmovn.s16 d9, q1\n"                          /* 1, int16 -> int8 */    \
      "vqmovn.s16 d10, q2\n"                         /* 2, int16 -> int8 */    \
      "vqmovn.s16 d11, q3\n"                         /* 3, int16 -> int8 */    \
      "mov   r0,   %[c_ptr]\n"                       /* store result */        \
      "vst1.32   {d10-d11},  [r0]\n"                                           \
      "add  r0,  r0,   #16\n"                                                  \
      "vst1.32   {d8-d9},  [r0]\n"

#define SPARSE_INT8_INT8_W16_v7_KERNEL \
  "veor   q4,  q0,  q0\n"              \
  "pld  [%[a_ptr], #32]    \n"         \
  "veor   q5,  q1,  q1\n"              \
  "pld  [%[widx_dmap], #32]    \n"     \
  "veor   q6,  q2,  q2\n"              \
  "pld  [%[b_ptr], #32]    \n"         \
  "veor   q7,  q3,  q3\n"              \
  "cmp    %[k],    #0\n"               \
  "beq    1f\n" /* main loop*/         \
  "0:\n"                               \
  "ldrsb   r0, [%[a_ptr]], #1\n"       \
  "subs    %[k],   %[k],   #1\n"       \
  "vld1.8  {d2-d3}, [%[b_ptr]]\n"      \
  "vdup.8    d0,   r0\n"               \
  "vmull.s8    q2,    d2,  d0\n"       \
  "vmull.s8    q3,    d3,  d0\n"       \
  "ldr     r1, [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], r1\n"    \
  "vaddw.s16   q4,    q4,  d4\n"       \
  "vaddw.s16   q5,    q5,  d5\n"       \
  "pld  [%[b_ptr], #32]    \n"         \
  "vaddw.s16   q6,    q6,  d6\n"       \
  "vaddw.s16   q7,    q7,  d7\n"       \
  "bne     0b\n"                       \
  "1:\n"

#define SPARSE_INT8_TRANS_INT32_TO_INT8_W16_v7          \
  /* write output */                                    \
  "vcvt.f32.s32   q0, q4\n" /*     cvt int32 to fp32*/  \
  "vcvt.f32.s32   q1, q5\n" /*      cvt int32 to fp32*/ \
  "vcvt.f32.s32   q2, q6\n" /*     cvt int32 to fp32*/  \
  "vcvt.f32.s32   q3, q7\n" /*      cvt int32 to fp32*/ \
  "vdup.32    q4,   %[vscale]\n"                        \
  "vdup.32    q8,   %[vbias]\n"                         \
  "vdup.32    q9,   d16[0]\n"                           \
  "vdup.32    q10,  d16[0]\n"                           \
  "vdup.32    q11,  d16[0]\n"                           \
  "3:\n"                                                \
  "vmla.f32  q8,   q0,  q4\n"                           \
  "vmla.f32  q9,   q1,  q4\n"                           \
  "vmla.f32  q10,  q2,  q4\n"                           \
  "vmla.f32  q11,  q3,  q4\n"

#define SPARSE_INT8_INT8_W16_v7_RELU                  \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q0, #0\n"              /* for relu */   \
  "vmax.f32   q8,   q8,   q0\n"      /* relu */       \
  "vmax.f32   q9,   q9,   q0\n"      /* relu */       \
  "vmax.f32   q10,  q10,  q0\n"      /* relu */       \
  "vmax.f32   q11,  q11,  q0\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W16_v7_RELU6                  \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q8,   q8,   q0\n"      /* relu6 */       \
  "vmax.f32   q9,   q9,   q0\n"      /* relu6 */       \
  "vmax.f32   q10,  q10,  q0\n"      /* relu6 */       \
  "vmax.f32   q11,  q11,  q0\n"      /* relu6 */       \
  "vmin.f32   q8,   q8,   q1\n"      /* relu6 */       \
  "vmin.f32   q9,   q9,   q1\n"      /* relu6 */       \
  "vmin.f32   q10,  q10,  q1\n"      /* relu6 */       \
  "vmin.f32   q11,  q11,  q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W16_v7_LEAKY_RELU                   \
  /* do relu */                                              \
  "11: \n"                                                   \
  "cmp   %[vflag_act],  #3       \n" /* check leakey relu */ \
  "bne   12f                     \n" /* no act end */        \
  "vmov.i32   q0, #0\n"              /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"      /* leakey relu alpha */ \
  "vcge.f32   q2,    q8,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q8,    q1   \n" /* vmulq_f32 */         \
  "vcge.f32   q4,    q9,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q5,    q9,    q1   \n" /* vmulq_f32 */         \
  "vcge.f32   q6,    q10,   q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q7,    q10,   q1   \n" /* vmulq_f32 */         \
  "vbif       q8,    q3,    q2   \n"                         \
  "vbif       q9,    q5,    q4   \n"                         \
  "vbif       q10,   q7,    q6   \n"                         \
  "vcge.f32   q2,    q11,    q0   \n" /* vcgeq_f32 */        \
  "vmul.f32   q3,    q11,    q1   \n" /* vmulq_f32 */        \
  "vbif       q11,   q3,    q2   \n"                         \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W16_v7_HARD_SWISH     \
  /* do relu */                                \
  "12: \n"                                     \
  "vmov.u32   q0,   #0                     \n" \
  "vld1.f32   {d2-d5}, [%[hs_param]]!      \n" \
  "vld1.f32   {d6-d7}, [%[hs_param]]       \n" \
  "vadd.f32   q4, q8, q1                \n"    \
  "vadd.f32   q6, q9, q1                \n"    \
  "vmul.f32   q5, q8, q2                \n"    \
  "vmul.f32   q7, q9, q2                \n"    \
  "vmax.f32   q4, q4, q0                \n"    \
  "vmax.f32   q6, q6, q0                \n"    \
  "vmin.f32   q4, q4, q3                \n"    \
  "vmin.f32   q6, q6, q3                \n"    \
  "vmul.f32   q8, q5, q4                \n"    \
  "vmul.f32   q9, q7, q6                \n"    \
  "vadd.f32   q4, q10, q1                \n"   \
  "vadd.f32   q6, q11, q1                \n"   \
  "vmul.f32   q5, q10, q2                \n"   \
  "vmul.f32   q7, q11, q2                \n"   \
  "vmax.f32   q4, q4, q0                \n"    \
  "vmax.f32   q6, q6, q0                \n"    \
  "vmin.f32   q4, q4, q3                \n"    \
  "vmin.f32   q6, q6, q3                \n"    \
  "vmul.f32   q10, q5, q4                \n"   \
  "vmul.f32   q11, q7, q6                \n"   \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx16, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx16, and the required data is
 * MxKxKx16.
 */
#define SPARSE_INT8_INT8_W16_v7_OUT                                          \
  SPARSE_INT8_INT8_W16_v7_KERNEL SPARSE_INT8_TRANS_INT32_TO_INT8_W16_v7      \
      SPARSE_INT8_INT8_W16_v7_RELU SPARSE_INT8_INT8_W16_v7_RELU6             \
          SPARSE_INT8_INT8_W16_v7_LEAKY_RELU                                 \
              SPARSE_INT8_INT8_W16_v7_HARD_SWISH                             \
      "vmov.f32  q0, #-0.5\n"    /* neg offset */                            \
      "vmov.f32  q1, #0.5\n"     /* pos offset */                            \
      "vmov.f32  q2, #0.5\n"     /* pos offset */                            \
      "vmov.f32  q3, #0.5\n"     /* pos offset */                            \
      "vmov.f32  q4, #0.5\n"     /* pos offset */                            \
      "vcgt.f32  q5, q8, #0\n"   /* get pos mask */                          \
      "vcgt.f32  q6, q9, #0\n"   /* get pos mask */                          \
      "vbif.f32  q1, q0, q5\n"   /* get right offset */                      \
      "vbif.f32  q2, q0, q6\n"   /* get right offset */                      \
      "vcgt.f32  q12, q10, #0\n" /* get pos mask */                          \
      "vcgt.f32  q13, q11, #0\n" /* get pos mask */                          \
      "vbif.f32  q3, q0, q12\n"  /* get right offset */                      \
      "vbif.f32  q4, q0, q13\n"  /* get right offset */                      \
      "vadd.f32  q8, q1, q8\n"   /*     add offset */                        \
      "vadd.f32  q9, q2, q9\n"   /*      add offset */                       \
      "vadd.f32  q10, q3, q10\n" /*      add offset */                       \
      "vadd.f32  q11, q4, q11\n" /*      add offset */                       \
      "vld1.f32 {d14-d15}, [%[vmax]] \n"                                     \
      "vcge.f32 q0,  q8,  q7\n"                    /* @ q8 >= -127 \n */     \
      "vcge.f32 q1,  q9,  q7\n"                    /* @ q9 >= -127 \n */     \
      "vcge.f32 q2,  q10, q7\n"                    /* @ q0 >= -127 \n */     \
      "vcge.f32 q3,  q11, q7\n"                    /* @ q1 >= -127 \n */     \
      "vbif q8, q7, q0\n"                          /* @ choose */            \
      "vbif q9, q7, q1\n"                          /* @ choose */            \
      "vbif q10, q7, q2\n"                         /* @ choose */            \
      "vbif q11, q7, q3\n"                         /* @ choose */            \
      "vcvt.s32.f32   q0, q8\n"                    /*     fp32->int32 */     \
      "vcvt.s32.f32   q1, q9\n"                    /*      fp32->int32 */    \
      "vcvt.s32.f32   q2, q10\n"                   /*      fp32->int32 */    \
      "vcvt.s32.f32   q3, q11\n"                   /*      fp32->int32 */    \
      "vqmovn.s32 d8, q0\n"                        /*     int32 -> int16 */  \
      "vqmovn.s32 d9, q1\n"                        /*      int32 -> int16 */ \
      "vqmovn.s32 d10, q2\n"                       /*      int32 -> int16 */ \
      "vqmovn.s32 d11, q3\n"                       /*      int32 -> int16 */ \
      "vqmovn.s16 d0, q4\n"                        /* 0, int16 -> int8 */    \
      "vqmovn.s16 d1, q5\n" /* 1, int16 -> int8 */ /* store result */        \
      "vst1.32   {d0-d1},  [%[c_ptr]]\n"

#define SPARSE_INT8_INT8_W8_v7_KERNEL  \
  "veor   q4,  q0,  q0\n"              \
  "veor   q5,  q1,  q1\n"              \
  "cmp    %[k],    #0\n"               \
  "beq    1f\n" /* main loop*/         \
  "0:\n"                               \
  "ldrsb   r0, [%[a_ptr]], #1\n"       \
  "subs    %[k],   %[k],   #1\n"       \
  "vld1.8  d2, [%[b_ptr]]\n"           \
  "vdup.8    d0,   r0\n"               \
  "vmull.s8    q2,    d2,  d0\n"       \
  "ldr     r1, [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], r1\n"    \
  "vaddw.s16   q4,    q4,  d4\n"       \
  "vaddw.s16   q5,    q5,  d5\n"       \
  "bne     0b\n"                       \
  "1:\n"

#define SPARSE_INT8_TRANS_INT32_TO_INT8_W8_v7           \
  /* write output */                                    \
  "vcvt.f32.s32   q0, q4\n" /*     cvt int32 to fp32*/  \
  "vcvt.f32.s32   q1, q5\n" /*      cvt int32 to fp32*/ \
  "vdup.32    q2,   %[vscale]\n"                        \
  "vdup.32    q6,   %[vbias]\n"                         \
  "vdup.32    q7,   d12[0]\n"                           \
  "3:\n"                                                \
  "vmla.f32  q6,   q0,  q2\n"                           \
  "vmla.f32  q7,   q1,  q2\n"

#define SPARSE_INT8_INT8_W8_v7_RELU                   \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q0, #0\n"              /* for relu */   \
  "vmax.f32   q6,   q6,   q0\n"      /* relu */       \
  "vmax.f32   q7,   q7,   q0\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W8_v7_RELU6                   \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q6,   q6,   q0\n"      /* relu6 */       \
  "vmax.f32   q7,   q7,   q0\n"      /* relu6 */       \
  "vmin.f32   q6,   q6,   q1\n"      /* relu6 */       \
  "vmin.f32   q7,   q7,   q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W8_v7_LEAKY_RELU                    \
  /* do relu */                                              \
  "11: \n"                                                   \
  "cmp   %[vflag_act],  #3       \n" /* check leakey relu */ \
  "bne   12f                     \n" /* no act end */        \
  "vmov.i32   q0, #0\n"              /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"      /* leakey relu alpha */ \
  "vcge.f32   q2,    q6,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q6,    q1   \n" /* vmulq_f32 */         \
  "vcge.f32   q4,    q7,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q5,    q7,    q1   \n" /* vmulq_f32 */         \
  "vbif       q6,    q3,    q2   \n"                         \
  "vbif       q7,    q5,    q4   \n"                         \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W8_v7_HARD_SWISH      \
  /* do relu */                                \
  "12: \n"                                     \
  "vmov.u32   q0,   #0                     \n" \
  "vld1.f32   {d2-d5}, [%[hs_param]]!      \n" \
  "vld1.f32   {d6-d7}, [%[hs_param]]       \n" \
  "vadd.f32   q4, q6, q1                \n"    \
  "vadd.f32   q8, q7, q1                \n"    \
  "vmul.f32   q5, q6, q2                \n"    \
  "vmul.f32   q9, q7, q2                \n"    \
  "vmax.f32   q4, q4, q0                \n"    \
  "vmax.f32   q8, q8, q0                \n"    \
  "vmin.f32   q4, q4, q3                \n"    \
  "vmin.f32   q8, q8, q3                \n"    \
  "vmul.f32   q6, q5, q4                \n"    \
  "vmul.f32   q7, q9, q8                \n"    \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx8, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx8, and the required data is
 * MxKxKx8.
 */
#define SPARSE_INT8_INT8_W8_v7_OUT                                            \
  SPARSE_INT8_INT8_W8_v7_KERNEL SPARSE_INT8_TRANS_INT32_TO_INT8_W8_v7         \
      SPARSE_INT8_INT8_W8_v7_RELU SPARSE_INT8_INT8_W8_v7_RELU6                \
          SPARSE_INT8_INT8_W8_v7_LEAKY_RELU SPARSE_INT8_INT8_W8_v7_HARD_SWISH \
      "vmov.f32  q8, #-0.5\n"  /* neg offset */                               \
      "vmov.f32  q0, #0.5\n"   /* pos offset */                               \
      "vmov.f32  q1, #0.5\n"   /* pos offset */                               \
      "vmov.f32  q2, #0.5\n"   /* pos offset */                               \
      "vmov.f32  q3, #0.5\n"   /* pos offset */                               \
      "vcgt.f32  q4, q6, #0\n" /* get pos mask */                             \
      "vcgt.f32  q5, q7, #0\n" /* get pos mask */                             \
      "vbif.f32  q0, q8, q4\n" /* get right offset */                         \
      "vbif.f32  q1, q8, q5\n" /* get right offset */                         \
      "vadd.f32  q6, q0, q6\n" /*     add offset */                           \
      "vadd.f32  q7, q1, q7\n" /*      add offset */                          \
      "vld1.f32 {d0-d1}, [%[vmax]] \n"                                        \
      "vcge.f32 q1,  q6,  q0\n"                    /* @ q8 >= -127 \n */      \
      "vcge.f32 q2,  q7,  q0\n"                    /* @ q9 >= -127 \n */      \
      "vbif q6, q0, q1\n"                          /* @ choose */             \
      "vbif q7, q0, q2\n"                          /* @ choose */             \
      "vcvt.s32.f32   q0, q6\n"                    /*     fp32->int32 */      \
      "vcvt.s32.f32   q1, q7\n"                    /*      fp32->int32 */     \
      "vqmovn.s32 d8, q0\n"                        /*     int32 -> int16 */   \
      "vqmovn.s32 d9, q1\n"                        /*      int32 -> int16 */  \
      "vqmovn.s16 d0, q4\n" /* 0, int16 -> int8 */ /* store result */         \
      "vst1.32   {d0},  [%[c_ptr]]\n"

#define SPARSE_INT8_INT8_W4_v7_KERNEL  \
  "veor   q3,  q0,  q0\n"              \
  "cmp    %[k],    #0\n"               \
  "beq    1f\n" /* main loop*/         \
  "0:\n"                               \
  "ldrsb   r0, [%[a_ptr]], #1\n"       \
  "vdup.8     d0,    r0\n"             \
  "subs    %[k],   %[k],   #1\n"       \
  "ldr     r0, [%[b_ptr]]\n"           \
  "vdup.32    d2,   r0\n"              \
  "vmull.s8    q2,    d2,  d0\n"       \
  "ldr     r1, [%[widx_dmap]],   #4\n" \
  "add   %[b_ptr],  %[b_ptr], r1\n"    \
  "vaddw.s16   q3,    q3,  d4\n"       \
  "bne     0b\n"                       \
  "1:\n"

#define SPARSE_INT8_TRANS_INT32_TO_INT8_W4_v7          \
  /* write output */                                   \
  "vcvt.f32.s32   q0, q3\n" /*     cvt int32 to fp32*/ \
  "vdup.32    q1,   %[vscale]\n"                       \
  "vdup.32    q4,   %[vbias]\n"                        \
  "3:\n"                                               \
  "vmla.f32  q4,   q0,  q1\n"

#define SPARSE_INT8_INT8_W4_v7_RELU                   \
  /* do relu */                                       \
  "cmp    %[vflag_act],    #0\n"     /* skip relu */  \
  "beq   9f                     \n"  /* no act end */ \
  "cmp    %[vflag_act],    #1\n"     /* skip relu */  \
  "bne   10f                     \n" /* other act */  \
  "vmov.i32   q0, #0\n"              /* for relu */   \
  "vmax.f32   q4,   q4,   q0\n"      /* relu */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W4_v7_RELU6                   \
  /* do relu6 */                                       \
  "10: \n"                                             \
  "cmp   %[vflag_act],  #2       \n" /* check relu6 */ \
  "bne   11f                     \n" /* no act end */  \
  "vmov.i32   q0,   #0\n"            /* for relu6 */   \
  "vdup.32    q1,   %[valpha]\n"     /* relu6 alpha */ \
  "vmax.f32   q4,   q4,   q0\n"      /* relu6 */       \
  "vmin.f32   q4,   q4,   q1\n"      /* relu6 */       \
  "b      9f                    \n"  /* relu end */

#define SPARSE_INT8_INT8_W4_v7_LEAKY_RELU                    \
  /* do relu */                                              \
  "11: \n"                                                   \
  "cmp   %[vflag_act],  #3       \n" /* check leakey relu */ \
  "bne   12f                     \n" /* no act end */        \
  "vmov.i32   q0, #0\n"              /* for relu */          \
  "vdup.32    q1,  %[valpha]\n"      /* leakey relu alpha */ \
  "vcge.f32   q2,    q4,    q0   \n" /* vcgeq_f32 */         \
  "vmul.f32   q3,    q4,    q1   \n" /* vmulq_f32 */         \
  "vbif       q4,    q3,    q2   \n"                         \
  "b      9f                    \n"

#define SPARSE_INT8_INT8_W4_v7_HARD_SWISH      \
  /* do relu */                                \
  "12: \n"                                     \
  "vmov.u32   q0,   #0                     \n" \
  "vld1.f32   {d2-d5}, [%[hs_param]]!      \n" \
  "vld1.f32   {d6-d7}, [%[hs_param]]       \n" \
  "vadd.f32   q5, q4, q1                \n"    \
  "vmul.f32   q6, q4, q2                \n"    \
  "vmax.f32   q5, q5, q0                \n"    \
  "vmin.f32   q5, q5, q3                \n"    \
  "vmul.f32   q4, q6, q5                \n"    \
  "9:\n"

/**
 * The data block size for sparse matrix calculation is Mx4, that is, the
 * parameter
 * matrix size is MxK, the activation matrix is Kx4, and the required data is
 * MxKxKx4.
 */
#define SPARSE_INT8_INT8_W4_v7_OUT                                            \
  SPARSE_INT8_INT8_W4_v7_KERNEL SPARSE_INT8_TRANS_INT32_TO_INT8_W4_v7         \
      SPARSE_INT8_INT8_W4_v7_RELU SPARSE_INT8_INT8_W4_v7_RELU6                \
          SPARSE_INT8_INT8_W4_v7_LEAKY_RELU SPARSE_INT8_INT8_W4_v7_HARD_SWISH \
      "vmov.f32  q1, #-0.5\n"  /* neg offset */                               \
      "vmov.f32  q0, #0.5\n"   /* pos offset */                               \
      "vcgt.f32  q2, q4, #0\n" /* get pos mask */                             \
      "vbif.f32  q0, q1, q2\n" /* get right offset */                         \
      "vadd.f32  q4, q0, q4\n" /*     add offset */                           \
      "vld1.f32 {d0-d1}, [%[vmax]] \n"                                        \
      "vcge.f32 q1,  q4,  q0\n"                    /* @ q8 >= -127 \n */      \
      "vbif q4, q0, q1\n"                          /* @ choose */             \
      "vcvt.s32.f32   q0, q4\n"                    /*     fp32->int32 */      \
      "vqmovn.s32 d2, q0\n"                        /*     int32 -> int16 */   \
      "vqmovn.s16 d0, q1\n" /* 0, int16 -> int8 */ /* store result */         \
      "vst1.32   d0[0],  [%[c_ptr]]\n"

/**
 * \brief Sparse calculation implementation of 1x1 convolution, both input and
 * output are int8.
 * Sparse matrix multiplication is calculated in blocks, the block size is Mx48,
 * that is,
 * the parameter matrix is MxK, and the activation matrix is Kx48; when N is
 * less than 48,
 * it is calculated in blocks of Mx32, Mx16, Mx8, and Mx4 in turn;
 * \brief the input type is int8, and the output type is also int8.
 * @param A sparse weight data
 * @param B dense input data
 * @param widx_dmap An array of int32_t values storing scaled [by sizeof(input
 * element)] difference
 * between input channels corresponding to successive non-zero element
 * @param nidx_nnzmap the number of non-zero kernel elements per each output
 * channel
 * @param bias
 * @param output
 * @param M
 * @param N
 * @param K
 * @param param
 * @param ctx
 */
void sparse_conv_int8_int8_pipelined(const int8_t* A,
                                     const int8_t* B,
                                     const int32_t* widx_dmap,
                                     const uint32_t* nidx_nnzmap,
                                     const float* bias,
                                     const float* scale,
                                     int8_t* output,
                                     int M,
                                     int K,
                                     int N,
                                     const operators::SparseConvParam& param,
                                     ARMContext* ctx) {
  auto act_param = param.activation_param;
  auto act_type = act_param.active_type;
  volatile float alpha = 0.f;
  float vhs_param[12] = {0.f};
  int flag_act = 0x00;  // relu: 1, relu6: 2, leakey: 3
  if (act_param.has_active) {
    if (act_type == lite_api::ActivationType::kRelu) {
      flag_act = 0x01;
    } else if (act_type == lite_api::ActivationType::kRelu6) {
      flag_act = 0x02;
      alpha = act_param.Relu_clipped_coef;
    } else if (act_type == lite_api::ActivationType::kLeakyRelu) {
      flag_act = 0x03;
      alpha = act_param.Leaky_relu_alpha;
    } else if (act_type == lite_api::ActivationType::kHardSwish) {
      flag_act = 0x04;
      for (int i = 0; i < 4; i++) {
        vhs_param[i] = act_param.hard_swish_offset / param.output_scale;
        vhs_param[i + 4] = 1.0 / act_param.hard_swish_scale;
        vhs_param[i + 8] = act_param.hard_swish_threshold / param.output_scale;
      }
    }
  }
  int flag_bias = (bias != nullptr) ? 1 : 0;
  size_t mc = N * sizeof(int8_t);
  size_t nc = M;
  size_t output_stride = N * sizeof(int8_t);
  size_t output_decrement = output_stride * nc - 48 * sizeof(int8_t);
  float vmax[4] = {-127.0, -127.0, -127.0, -127.0};
  while
    SPARSE_LIKELY(mc >= 48 * sizeof(int8_t)) {
      LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
        int8_t* cur_output =
            reinterpret_cast<int8_t*>((uintptr_t)output + output_stride * i);
        const int8_t* cur_w = A;
        uint32_t nnz = nidx_nnzmap[i];
        const int8_t* cur_b = B;
        const int32_t* dmap = widx_dmap;
        if (i != 0) {
          cur_w = A + nidx_nnzmap[i - 1];
          nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];
          cur_b +=
              ((nidx_nnzmap[i - 1] == 0) ? 0
                                         : widx_dmap[nidx_nnzmap[i - 1] - 1]);
          dmap = widx_dmap + nidx_nnzmap[i - 1];
        }
        float vsclae = scale[i];
        float vbias = (bias != nullptr) ? bias[i] : 0.0;
        float* hs_param = vhs_param;
        // clang-format off
          asm volatile(SPARSE_INT8_INT8_W48_v7_OUT
            : [a_ptr] "+r"(cur_w),
              [b_ptr] "+r"(cur_b),
              [c_ptr] "+r"(cur_output),
              [k] "+r"(nnz),
              [widx_dmap] "+r"(dmap),
              [hs_param] "+r"(hs_param)
            : [vscale] "r"(vsclae),
              [vbias] "r"(vbias),
              [vflag_act] "r"(flag_act),
              [valpha] "r"(alpha),
              [vmax] "r"(vmax)
            : "q0",
              "q1",
              "q2",
              "q3",
              "q4",
              "q5",
              "q6",
              "q7",
              "q8",
              "q9",
              "q10",
              "q11",
              "q12",
              "q13",
              "q14",
              "q15",
              "r0",
              "r1",
              "cc",
              "memory");
        // clang-format on
      }
      LITE_PARALLEL_COMMON_END();
      output =
          reinterpret_cast<int8_t*>((uintptr_t)output + 48 * sizeof(int8_t));
      B += 48;
      mc -= 48 * sizeof(int8_t);
    }
  if
    SPARSE_UNLIKELY(mc != 0) {
      output_decrement += 16 * sizeof(int8_t);
      if (mc & (32 * sizeof(int8_t))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          int8_t* cur_output =
              reinterpret_cast<int8_t*>((uintptr_t)output + output_stride * i);
          const int8_t* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const int8_t* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            cur_w = A + nidx_nnzmap[i - 1];
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];
            cur_b +=
                ((nidx_nnzmap[i - 1] == 0) ? 0
                                           : widx_dmap[nidx_nnzmap[i - 1] - 1]);
            dmap = widx_dmap + nidx_nnzmap[i - 1];
          }
          float vsclae = scale[i];
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float* hs_param = vhs_param;
          // clang-format off
            asm volatile(SPARSE_INT8_INT8_W32_v7_OUT
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vscale] "r"(vsclae),
                [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha),
                [vmax] "r"(vmax)
              : "q0",
                "q1",
                "q2",
                "q3",
                "q4",
                "q5",
                "q6",
                "q7",
                "q8",
                "q9",
                "q10",
                "q11",
                "q12",
                "q13",
                "q14",
                "q15",
                "r0",
                "r1",
                "cc",
                "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<int8_t*>((uintptr_t)output + 32 * sizeof(int8_t));
        B += 32;
        mc -= 32 * sizeof(int8_t);
      }
      output_decrement += 16 * sizeof(int8_t);
      if (mc & (16 * sizeof(int8_t))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          int8_t* cur_output =
              reinterpret_cast<int8_t*>((uintptr_t)output + output_stride * i);
          const int8_t* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const int8_t* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            cur_w = A + nidx_nnzmap[i - 1];
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];
            cur_b +=
                ((nidx_nnzmap[i - 1] == 0) ? 0
                                           : widx_dmap[nidx_nnzmap[i - 1] - 1]);
            dmap = widx_dmap + nidx_nnzmap[i - 1];
          }
          float vsclae = scale[i];
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float* hs_param = vhs_param;
          // clang-format off
            asm volatile(SPARSE_INT8_INT8_W16_v7_OUT
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vscale] "r"(vsclae),
                [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha),
                [vmax] "r"(vmax)
              : "q0",
                "q1",
                "q2",
                "q3",
                "q4",
                "q5",
                "q6",
                "q7",
                "q8",
                "q9",
                "q10",
                "q11",
                "q12",
                "q13",
                "r0",
                "r1",
                "cc",
                "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<int8_t*>((uintptr_t)output + 16 * sizeof(int8_t));
        B += 16;
        mc -= 16 * sizeof(int8_t);
      }
      output_decrement += 8 * sizeof(int8_t);
      if (mc & (8 * sizeof(int8_t))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          int8_t* cur_output =
              reinterpret_cast<int8_t*>((uintptr_t)output + output_stride * i);
          const int8_t* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const int8_t* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            cur_w = A + nidx_nnzmap[i - 1];
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];
            cur_b +=
                ((nidx_nnzmap[i - 1] == 0) ? 0
                                           : widx_dmap[nidx_nnzmap[i - 1] - 1]);
            dmap = widx_dmap + nidx_nnzmap[i - 1];
          }
          float vsclae = scale[i];
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float* hs_param = vhs_param;
          // clang-format off
            asm volatile(SPARSE_INT8_INT8_W8_v7_OUT
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vscale] "r"(vsclae),
                [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha),
                [vmax] "r"(vmax)
              : "q0",
                "q1",
                "q2",
                "q3",
                "q4",
                "q5",
                "q6",
                "q7",
                "q8",
                "q9",
                "r0",
                "r1",
                "cc",
                "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<int8_t*>((uintptr_t)output + 8 * sizeof(int8_t));
        B += 8;
        mc -= 8 * sizeof(int8_t);
      }
      output_decrement += 4 * sizeof(int8_t);
      if (mc & (4 * sizeof(int8_t))) {
        LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
          int8_t* cur_output =
              reinterpret_cast<int8_t*>((uintptr_t)output + output_stride * i);
          const int8_t* cur_w = A;
          uint32_t nnz = nidx_nnzmap[i];
          const int8_t* cur_b = B;
          const int32_t* dmap = widx_dmap;
          if (i != 0) {
            cur_w = A + nidx_nnzmap[i - 1];
            nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];
            cur_b +=
                ((nidx_nnzmap[i - 1] == 0) ? 0
                                           : widx_dmap[nidx_nnzmap[i - 1] - 1]);
            dmap = widx_dmap + nidx_nnzmap[i - 1];
          }
          float vsclae = scale[i];
          float vbias = (bias != nullptr) ? bias[i] : 0.0;
          float* hs_param = vhs_param;
          // clang-format off
            asm volatile(SPARSE_INT8_INT8_W4_v7_OUT
              : [a_ptr] "+r"(cur_w),
                [b_ptr] "+r"(cur_b),
                [c_ptr] "+r"(cur_output),
                [k] "+r"(nnz),
                [widx_dmap] "+r"(dmap),
                [hs_param] "+r"(hs_param)
              : [vscale] "r"(vsclae),
                [vbias] "r"(vbias),
                [vflag_act] "r"(flag_act),
                [valpha] "r"(alpha),
                [vmax] "r"(vmax)
              : "q0",
                "q1",
                "q2",
                "q3",
                "q4",
                "q5",
                "q6",
                "r0",
                "r1",
                "cc",
                "memory");
          // clang-format on
        }
        LITE_PARALLEL_COMMON_END();
        output =
            reinterpret_cast<int8_t*>((uintptr_t)output + 4 * sizeof(int8_t));
        B += 4;
        mc -= 4 * sizeof(int8_t);
      }

      if
        SPARSE_UNLIKELY(mc != 0 && mc < 4 * sizeof(int8_t)) {
          LITE_PARALLEL_COMMON_BEGIN(i, tid, nc, 0, 1) {
            int8_t* out_ptr = reinterpret_cast<int8_t*>((uintptr_t)output +
                                                        output_stride * i);
            const int8_t* cur_w = A;
            uint32_t nnz = nidx_nnzmap[i];
            const int8_t* cur_b = B;
            const int32_t* dmap = widx_dmap;
            if (i != 0) {
              cur_w = A + nidx_nnzmap[i - 1];
              nnz = nidx_nnzmap[i] - nidx_nnzmap[i - 1];
              cur_b += ((nidx_nnzmap[i - 1] == 0)
                            ? 0
                            : widx_dmap[nidx_nnzmap[i - 1] - 1]);
              dmap = widx_dmap + nidx_nnzmap[i - 1];
            }
            float32x4_t vbias =
                (bias != nullptr) ? vdupq_n_f32(bias[i]) : vdupq_n_f32(0);
            float vscale = scale[i];
            int32x4_t vacc0123 = vdupq_n_s32(0);
            for (int j = 0; j < nnz; j++) {
              int8x8_t vi0123 = vdup_n_s8(0);
              if (mc == 1) {
                vi0123 = vld1_lane_s8(cur_b, vi0123, 0);
              } else if (mc == 2) {
                vi0123 = vld1_lane_s8(cur_b, vi0123, 0);
                vi0123 = vld1_lane_s8(cur_b + 1, vi0123, 1);
              } else {
                vi0123 = vld1_lane_s8(cur_b, vi0123, 0);
                vi0123 = vld1_lane_s8(cur_b + 1, vi0123, 1);
                vi0123 = vld1_lane_s8(cur_b + 2, vi0123, 2);
              }
              int8x8_t vw = vld1_dup_s8(cur_w);
              cur_w += 1;
              intptr_t diff = *dmap++;
              cur_b = (const int8_t*)((uintptr_t)cur_b + (uintptr_t)diff);
              int16x8_t vo0123 = vmull_s8(vi0123, vw);
              vacc0123 = vaddw_s16(vacc0123, vget_low_s16(vo0123));
            }

            float32x4_t vaccf0123 = vcvtq_f32_s32(vacc0123);
            vaccf0123 = vmlaq_n_f32(vbias, vaccf0123, vscale);
            float32x4_t vzero = vdupq_n_f32(0);
            if (flag_act == 1) {
              vaccf0123 = vmaxq_f32(vaccf0123, vzero);
            } else if (flag_act == 0) {
            } else if (flag_act == 2) {
              float32x4_t aph = vdupq_n_f32(alpha);
              vaccf0123 = vmaxq_f32(vaccf0123, vzero);
              vaccf0123 = vminq_f32(vaccf0123, aph);
            } else if (flag_act == 3) {
              float32x4_t aph = vdupq_n_f32(alpha);
              uint32x4_t vflag0123 = vcgeq_f32(vaccf0123, vzero);
              float32x4_t v0123 = vmulq_f32(vaccf0123, aph);
              vaccf0123 = vbslq_f32(vflag0123, vaccf0123, v0123);
            } else {
              float32x4_t offset =
                  vdupq_n_f32(act_param.hard_swish_offset / param.output_scale);
              float32x4_t hs_scale =
                  vdupq_n_f32(1.0 / act_param.hard_swish_scale);
              float32x4_t thre = vdupq_n_f32(act_param.hard_swish_threshold /
                                             param.output_scale);
              float32x4_t vset0123 = vaddq_f32(vaccf0123, offset);
              float32x4_t vscale0123 = vmulq_f32(vaccf0123, hs_scale);
              vset0123 = vmaxq_f32(vset0123, vzero);
              vset0123 = vminq_f32(vset0123, thre);
              vaccf0123 = vmulq_f32(vscale0123, vset0123);
            }

            vaccf0123 = vbslq_f32(vcgeq_f32(vaccf0123, vdupq_n_f32(-127.0)),
                                  vaccf0123,
                                  vdupq_n_f32(-127.0));
            float32x4_t vpos = vdupq_n_f32(0.5);
            float32x4_t vneg = vdupq_n_f32(-0.5);
            vaccf0123 = vbslq_f32(vcgeq_f32(vaccf0123, vzero),
                                  vaddq_f32(vaccf0123, vpos),
                                  vaddq_f32(vaccf0123, vneg));

            int32x4_t vacci0123 = vcvtq_s32_f32(vaccf0123);

            int16x4_t v16i0123 = vqmovn_s32(vacci0123);
            int16x4_t v16i4567 = vdup_n_s16(0);
            int8x8_t v8i01234567 = vqmovn_s16(vcombine_s16(v16i0123, v16i4567));

            if (mc == 1) {
              vst1_lane_s8(out_ptr, v8i01234567, 0);
            } else if (mc == 2) {
              vst1_lane_s8(out_ptr, v8i01234567, 0);
              vst1_lane_s8(out_ptr + 1, v8i01234567, 1);
            } else {
              vst1_lane_s8(out_ptr, v8i01234567, 0);
              vst1_lane_s8(out_ptr + 1, v8i01234567, 1);
              vst1_lane_s8(out_ptr + 2, v8i01234567, 2);
            }
          }
          LITE_PARALLEL_COMMON_END();
        }
    }
}

#endif

}  // namespace math
}  // namespace arm
}  // namespace lite
}  // namespace paddle
