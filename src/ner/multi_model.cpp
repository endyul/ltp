//
// Created by Zixiang Xu on 2016/11/9.
//

#include "ner/multi_model.h"
#include "utils/model_helper.hpp"
#include "ner/extractor.h"

namespace ltp {
  namespace ner {

    const std::string MultiModel::model_header = "otner";

    MultiModel::MultiModel() {}
    MultiModel::~MultiModel() {}

    void MultiModel::save(std::ostream& ofs, const framework::Parameters::DumpOption& opt) {
      framework::MultiModel::save(ofs, model_header);
    }

    bool MultiModel::load(std::istream& ifs) {
      BaseModel baseModel;
      baseModel.soft_load(ifs);
      if (!ltp::utility::ModelHelper::check_version(baseModel)) {
        return false;
      }

      if (!framework::MultiModel::load(ifs)) {
        return false;
      }

      if (!check_header(Model::model_header)) {
        return false;
      }

      return true;
    }
  }
}
