//
// Created by Zixiang Xu on 2016/10/27.
//

#ifndef LTP_LANGUAGE_TECHNOLOGY_PLATFORM_BASE_MODEL_H
#define LTP_LANGUAGE_TECHNOLOGY_PLATFORM_BASE_MODEL_H

#include "config.h"
#include <cassert>

namespace ltp{
namespace framework{
  class BaseModel : public Serializable {
  public:
    /**
     * for older model compatibility (older than v3.4.0)
     * older version has a head chunk with 128 bytes stored `model_name`, without model version checking
     * after v3.4.0, we use name(32B), version(32B), type(32B) and a reserved field(32B)
     */

    static const uint32_t HEADER_CHUNK_SIZE = 32;
    static const uint32_t VERSION_CHUNK_SIZE = 16;
    static const uint32_t TYPE_CHUNK_SIZE = 4;
    static const uint32_t RESERVED_CHUNK_SIZE = 76;

    static const uint32_t TYPE_HITSCIR_NORMAL_MODEL = 0;
    static const uint32_t TYPE_HITSCIR_MERGE_MODEL = 1;
    static const uint32_t TYPE_USER_MODEL = 2;
    static const uint32_t TYPE_INCREMENTAL_MODEL = 3;


    BaseModel() {
      strcpy(version, LTP_MODEL_VERSION);
      assert(HEADER_CHUNK_SIZE
                 +VERSION_CHUNK_SIZE
                 +TYPE_CHUNK_SIZE
                 +RESERVED_CHUNK_SIZE == 128);
    }

    BaseModel(const BaseModel& _base_model) {
      strcpy(header, _base_model.header);
      strcpy(version, _base_model.version);
      type = _base_model.type;
      strcpy(reserved_field, _base_model.reserved_field);
    }

    BaseModel(std::istream & ifs) {
      load(ifs);
    }

    void save(std::ostream & ofs) const {
      ofs.write(header, HEADER_CHUNK_SIZE);
      ofs.write(version, VERSION_CHUNK_SIZE);
      ofs.write(reinterpret_cast<const char*>(&type), TYPE_CHUNK_SIZE);
      ofs.write(reserved_field, RESERVED_CHUNK_SIZE);
    }


    bool load(std::istream & ifs) {
      ifs.read(header, HEADER_CHUNK_SIZE);
      ifs.read(version, VERSION_CHUNK_SIZE);
      ifs.read(reinterpret_cast<char *>(&type), TYPE_CHUNK_SIZE);
      ifs.read(reserved_field, RESERVED_CHUNK_SIZE);

      return true;
    }

    /**
     * load model without effect ifs
     * @param ifs
     * @return success or not
     */
    bool soft_load(std::istream & ifs) {
      bool r = load(ifs);
      ifs.clear();
      ifs.seekg(0, std::ios::beg);
      return r;
    }

    void set_header(const char * _header) {
      strcpy(header, _header);
    }

    const char * get_header() const {
      return header;
    }

    void set_type(uint32_t _type) {
      type = _type;
    }

    uint32_t get_type() const {
      return type;
    }

    void set_version(const char * _version) {
      strcpy(version, _version);
    }

    const char * get_version() const {
      return version;
    }

    void set_reserved_field(const char * _other) {
      strcpy(reserved_field, _other);
    }

    const char * get_reserved_field() const {
      return reserved_field;
    }

    bool check_header(const std::string& _header) const {
      return check_header(_header.c_str());
    }

    bool check_header(const char * _header) const {
      return strcmp(header, _header) == 0;
    }

  protected:
    char header[HEADER_CHUNK_SIZE];
    char version[VERSION_CHUNK_SIZE];
    uint32_t type;
    char reserved_field[RESERVED_CHUNK_SIZE];

    bool is_version_less_than_030400() const {
      return (strcmp(version, "") == 0
          && type == 0
          && strcmp(reserved_field, "") == 0) ;
    }
  };
}
}

#endif //LTP_LANGUAGE_TECHNOLOGY_PLATFORM_ABSTRACT_MODEL_H
