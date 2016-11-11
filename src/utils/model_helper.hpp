#ifndef __LTP_UTILS_MODEL_HELPER_HPP__
#define __LTP_UTILS_MODEL_HELPER_HPP__

#include "framework/base_model.h"
#include "config.h"
#include <sstream>

using ltp::framework::BaseModel;


#define UNKNOWN_MODEL_VERSION "unknown"

namespace ltp {
  namespace utility {


    class ModelHelper {
    public:

      static bool check_version(const BaseModel& model) {
        bool r = true;

        if (strcmp(model.get_version(), "") == 0) {
          r = false;
        } else if (ModelHelper::is_version_less_than_030400(model)) {
          /**
           * model version checking starts from v3.4.0,
           * so model less than v3.4.0 should be invalid model
           */
          r = false;
        } else if ( ! ModelHelper::is_version_match(model, LTP_MODEL_VERSION)) {
          r = false;
        }

        return r;
      }

      static std::string get_version_expr(const BaseModel& model) {
        if (strcmp(model.get_version(), "") == 0) {
          return UNKNOWN_MODEL_VERSION;
        }

        return model.get_version();

      }

      static std::string get_target_version() {
        return std::string(LTP_MODEL_VERSION);
      }

      static std::string get_version_error_msg(const BaseModel& model) {
        std::stringstream ss;
        ss << "Unsupported model version: "
           << ModelHelper::get_version_expr(model)
           << " , Expected model version: "
           << ModelHelper::get_target_version()
           <<"\n"
           << "If you are trying to load your customized model, please retrain it with current LTP.";
        return ss.str();
      }

    protected:
      static bool is_version_less_than_030400(const BaseModel& model) {
        return (strcmp(model.get_version(), "") == 0
                && model.get_type() == 0
                && strcmp(model.get_reserved_field(), "") == 0);
      }

      static bool is_version_match(const BaseModel& model, const char* target_version) {
        return strcmp(model.get_version(), target_version) == 0;
      }

    };

  }
}

#endif
