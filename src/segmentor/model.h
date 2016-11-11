#ifndef __LTP_SEGMENTOR_MODEL_H__
#define __LTP_SEGMENTOR_MODEL_H__

#include "framework/model.h"
#include "utils/smartmap.hpp"

namespace ltp {
namespace segmentor {

class Model: public framework::Model {
public:
  typedef utility::SmartMap<bool> lexicon_t;

  const static std::string model_header;

  //! The internal lexicon use to extract lexicon features.
  lexicon_t internal_lexicon;

  //! The external lexicon use to extract lexicon features.
  lexicon_t external_lexicon;

public:
  Model();
  ~Model();

  /**
   * save the model to a output stream
   *
   *  @param[out] ofs   the output stream
   */
  void save(std::ostream & ofs,
            const framework::Parameters::DumpOption& opt);

  /**
   * load the model from an input stream
   *
   *  @param[in]  ifs   the input stream
   */
  bool load(std::istream& ifs);
};

}     //  end for namespace segmentor
}     //  end for namespace ltp

#endif  //  end for __LTP_SEGMENTOR_MODEL_H__
