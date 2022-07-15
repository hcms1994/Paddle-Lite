// Copyright (c) 2022 PaddlePaddle Authors. All Rights Reserved.
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

#pragma once

#include <limits>
#include <memory>
#include <string>
#include <vector>
#include "lite/core/optimizer/mir/pass.h"
#include "lite/core/optimizer/mir/pass_registry.h"
#include "lite/core/tensor.h"
#include "lite/core/types.h"

namespace paddle {
namespace lite {
namespace mir {

class ReshapeCalcOfflinePass : public mir::StmtPass {
 public:
  void Apply(const std::unique_ptr<SSAGraph>& graph) override;
  void RemoveReshapePattern(const std::unique_ptr<SSAGraph>& graph);
};

}  // namespace mir
}  // namespace lite
}  // namespace paddle
