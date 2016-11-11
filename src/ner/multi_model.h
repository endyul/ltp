//
// Created by Zixiang Xu on 2016/11/9.
//

#ifndef LTP_LANGUAGE_TECHNOLOGY_PLATFORM_MULTI_MODEL_H
#define LTP_LANGUAGE_TECHNOLOGY_PLATFORM_MULTI_MODEL_H


#include "framework/multi_model.h"

namespace ltp {
  namespace ner {

    class MultiModel : public framework::MultiModel {
    public:
      const static std::string model_header;

    public:
      MultiModel();
      ~MultiModel();
      void save(std::ostream& ofs, const framework::Parameters::DumpOption& opt);
      bool load(std::istream& ifs);

    };

  }
}


#endif //LTP_LANGUAGE_TECHNOLOGY_PLATFORM_MULTI_MODEL_H
