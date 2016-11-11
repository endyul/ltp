//
// Created by 徐梓翔 on 2016/10/27.
//

#ifndef LTP_LANGUAGE_TECHNOLOGY_PLATFORM_MULTI_NER_H
#define LTP_LANGUAGE_TECHNOLOGY_PLATFORM_MULTI_NER_H

#include "ner/multi_model.h"
#include "ner.h"
#include <vector>

namespace ltp{
namespace ner{

class MultiNamedEntityRecognizer {
protected:
  ltp::ner::MultiModel *model;  //! The pointer to the model.
  std::vector<NamedEntityRecognizer*> ners;

public:
  MultiNamedEntityRecognizer();
  ~MultiNamedEntityRecognizer();

  size_t num_models() const;

};

}
}

#endif //LTP_LANGUAGE_TECHNOLOGY_PLATFORM_MULTI_NER_H
