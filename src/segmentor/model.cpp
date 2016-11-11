#include "segmentor/model.h"
#include "segmentor/extractor.h"
#include <cstring>
#include "utils/model_helper.hpp"
#include "segmentor.h"

namespace ltp {
namespace segmentor {

using framework::Parameters;

Model::Model(): framework::Model(Extractor::num_templates()){}
Model::~Model() {}

const std::string Model::model_header = "otcws";

void Model::save(std::ostream& ofs, const Parameters::DumpOption& opt) {
  framework::Model::save(ofs, model_header, opt);
  internal_lexicon.dump(ofs);
}

bool Model::load(std::istream& ifs) {

  BaseModel baseModel;
  baseModel.soft_load(ifs);
  if (!ltp::utility::ModelHelper::check_version(baseModel)) {
    return false;
  }

  if (!framework::Model::load(ifs)) {
    return false;
  }

  if (!check_header(Model::model_header)) {
    return false;
  }

  if (!internal_lexicon.load(ifs)) {
     return false;
  }

  return true;
}

}     //  end for namespace segmentor
}     //  end for namespace ltp
