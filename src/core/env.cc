//
// Modified for jumanpp-lib: RNN code removed
//

#include "core/env.h"

#include "core_version.h"

namespace jumanpp {
namespace core {

Status JumanppEnv::loadModel(StringPiece filename) {
  JPP_RETURN_IF_ERROR(modelFile_.open(filename));
  JPP_RETURN_IF_ERROR(modelFile_.load(&modelInfo_));
  JPP_RETURN_IF_ERROR(dicBldr_.restoreDictionary(modelInfo_));
  JPP_RETURN_IF_ERROR(dicHolder_.load(dicBldr_));

  if (hasPerceptronModel()) {
    JPP_RETURN_IF_ERROR(perceptron_.load(modelInfo_));
    scorers_.feature = &perceptron_;
    scorers_.scoreWeights.push_back(1);
    scoringConf_.numScorers += 1;
  }

  core_.reset(new CoreHolder{dicBldr_.spec, dicHolder_});

  return Status::Ok();
}

bool JumanppEnv::hasPerceptronModel() const {
  for (auto& x : modelInfo_.parts) {
    if (x.kind == core::model::ModelPartKind::Perceprton) {
      return true;
    }
  }
  return false;
}

void JumanppEnv::setBeamSize(u32 size) { scoringConf_.beamSize = size; }

Status JumanppEnv::initFeatures(const features::StaticFeatureFactory* sff) {
  return core_->initialize(sff);
}

void JumanppEnv::setGlobalBeam(i32 globalBeam, i32 rightCheck, i32 rightBeam) {
  analyzerConfig_.globalBeamSize = globalBeam;
  analyzerConfig_.rightGbeamCheck = rightCheck;
  analyzerConfig_.rightGbeamSize = rightBeam;
}

void JumanppEnv::setAutoBeam(i32 base, i32 step, i32 max) {
  analyzerConfig_.autoBeamBase = base;
  analyzerConfig_.autoBeamStep = step;
  analyzerConfig_.autoBeamMax = max;
}

void JumanppEnv::fillVersion(VersionInfo* result) const {
  result->binary = JPP_VERSION_STRING.str();
  using model::ModelPartKind;
  auto dic = modelInfo_.firstPartOf(ModelPartKind::Dictionary);
  result->dictionary.clear();
  if (dic) {
    result->dictionary = dic->comment;
  }
  auto model = modelInfo_.firstPartOf(ModelPartKind::Perceprton);
  result->model.clear();
  if (model) {
    result->model = model->comment;
  }
  result->rnn.clear();
}

}  // namespace core
}  // namespace jumanpp
