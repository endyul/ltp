//
// Created by Zixiang Xu on 2016/11/9.
//

#ifndef LTP_LANGUAGE_TECHNOLOGY_PLATFORM_MODEL_H
#define LTP_LANGUAGE_TECHNOLOGY_PLATFORM_MODEL_H

#include "framework/model.h"

namespace ltp {
namespace postagger {

class Model : public framework::Model {
public:
  const static std::string model_header;

public:
  Model();
  ~Model();
  void save(std::ostream& ofs, const framework::Parameters::DumpOption& opt);
  bool load(std::istream& ifs);


};

}
}


#endif //LTP_LANGUAGE_TECHNOLOGY_PLATFORM_MODEL_H
