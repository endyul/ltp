#ifndef __LTP_SEGMENTOR_EXTRACTOR_H__
#define __LTP_SEGMENTOR_EXTRACTOR_H__

#include <vector>
#include "segmentor/instance.h"
#include "utils/new_template.hpp"
#include <functional>

namespace ltp {
namespace segmentor {

/**
 * A singleton for extracting features
 *
 */
class Extractor {
public:
  static Extractor& extractor();
  static size_t num_templates();

  /**
   * Extract first-order features and store the vector of features vector
   *
   *  @param[in]  inst    The pointer to the instance.
   *  @param[out] vector of features vectors   Features of the instance.
   */
  static std::vector<std::vector<std::string>> extract1o_new(const Instance& inst);
protected:
  Extractor();
  ~Extractor();
private:
  static std::vector< utility::NewTemplate* > templates;

  typedef std::unordered_map<std::string, std::function<std::string(const Instance&, int)>> ft_funcs_type;
  static ft_funcs_type ft_funcs;

  static utility::NewTemplate::Data data;

};

}     //  end for namespace segmentor
}     //  end for namespace ltp 

#endif  //  end for __LTP_SEGMENTOR_EXTRACTOR_H__
