//
// Created by Zixiang Xu on 2016/11/9.
//

#include "model.h"
#include "utils/model_helper.hpp"
#include "postagger/extractor.h"

namespace ltp {
namespace postagger {

  const std::string Model::model_header = "otpos";

  Model::Model(): framework::Model(Extractor::num_templates()){}
  Model::~Model() {}

  void Model::save(std::ostream& ofs, const framework::Parameters::DumpOption& opt) {
    framework::Model::save(ofs, model_header, opt);
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

    return true;
  }
}
}
