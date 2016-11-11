#ifndef __LTP_FRAMEWORK_MODEL_H__
#define __LTP_FRAMEWORK_MODEL_H__

#include "framework/serializable.h"
#include "framework/parameter.h"
#include "framework/featurespace.h"
#include "framework/base_model.h"
#include "utils/smartmap.hpp"
#include "config.h"

namespace ltp {
namespace framework {

class Model: public BaseModel {
public:
  Model(const size_t& nr_feature_types): space(nr_feature_types) {}
  Model(const char* _header,
        const size_t& nr_feature_types): space(nr_feature_types) {
    strcpy(header, _header);
  }

  /*
   * non-para constructor
   * begin at v3.4.0
   * we should call `load` function then immediately to construct `space`
   * we just set nr_feature_types = 0
   * space with nr_feature_types is dynamically malloc in `space.load` function
   *
   */
  Model(): space(0) {}


  ~Model() {}


  /**
   * get number of labels;
   *
   *  @return   int   the number of labels
   */
  uint32_t num_labels(void) const { return labels.size(); }


  /**
   * save the model to a output stream
   *
   *  @param[out] ofs         The output stream.
   *  @param[in]  opt         The Parameters::DumpOption opt
   */
  void save(std::ostream & ofs,
            const Parameters::DumpOption& opt = Parameters::DumpOption::kDumpAveraged) const {

    ofs.write(header, HEADER_CHUNK_SIZE);
    ofs.write(version, VERSION_CHUNK_SIZE);
    ofs.write(reinterpret_cast<const char *>(&type), TYPE_CHUNK_SIZE);
    ofs.write(reserved_field, RESERVED_CHUNK_SIZE);

    labels.dump(ofs);
    space.dump(ofs);
    param.dump(ofs, opt);
  }

  void save(std::ostream & ofs,
            const std::string & _header ,
            const Parameters::DumpOption& opt = Parameters::DumpOption::kDumpAveraged) {
    save(ofs, _header.c_str(), opt);
  }

  void save(std::ostream & ofs,
            const char * _header ,
            const Parameters::DumpOption& opt = Parameters::DumpOption::kDumpAveraged) {
    strcpy(header, _header);
    save(ofs, opt);
  }


  /**
   * load the model from an input stream
   *
   *  @param[in]  ifs   the input stream
   */
  bool load(std::istream& ifs) {

    ifs.read(header, HEADER_CHUNK_SIZE);
    ifs.read(version, VERSION_CHUNK_SIZE);
    ifs.read(reinterpret_cast<char *>(&type), TYPE_CHUNK_SIZE);
    ifs.read(reserved_field, RESERVED_CHUNK_SIZE);

    if (!labels.load(ifs)) {
      return false;
    }
    if (!space.load(ifs)) {
      return false;
    }
    space.set_num_labels(labels.size());
    if (!param.load(ifs)) {
      return false;
    }

    return true;
  }

public:
  utility::IndexableSmartMap labels;  //! The labels.
  ViterbiFeatureSpace space;          //! The feature space.
  Parameters param;                   //! The parameters.
};


} //  namespace framework
} //  namespace ltp

#endif  //  end for __LTP_FRAMEWORK_MODEL_H__
