//
// Created by Zixiang Xu on 2016/10/27.
//

#include "multi_ner.h"

namespace ltp{
namespace ner{

  MultiNamedEntityRecognizer::MultiNamedEntityRecognizer(): model(0) {}
  MultiNamedEntityRecognizer::~MultiNamedEntityRecognizer() {if (model) {delete model; model=0;}}

  size_t MultiNamedEntityRecognizer::num_models() const {
    return model->num_models();
  }

}
}

