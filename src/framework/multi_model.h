//
// Created by Zixiang Xu on 2016/10/25.
//

#ifndef LTP_LANGUAGE_TECHNOLOGY_PLATFORM_MERGE_MODEL_H
#define LTP_LANGUAGE_TECHNOLOGY_PLATFORM_MERGE_MODEL_H

#include <vector>
#include <ner/extractor.h>
#include <stdlib.h>
#include "ner/model.h"


namespace ltp {
namespace framework {

class MultiModel: public BaseModel {
public:
  MultiModel() {}

  MultiModel(const char* _header) {
    strcpy(header, _header);
  }

  ~MultiModel() {
    clear();
  }


  Model* get(int i) {
    return models.at(i);
  }

  void save(std::ostream & ofs) {
    if (models.size() == 0) {
      ERROR_LOG("input model should be specified.");
      return;
    }
    save(ofs, models[0] -> get_header());
  }

  void save(std::ostream & ofs,
            const std::string & _header) {
    save(ofs, _header.c_str());
  }

  void save(std::ostream & ofs,
            const char * _header) {

    if (models.size() == 0) {
      ERROR_LOG("input model should be specified.");
      return;
    }

    if (!valid()) {
      ERROR_LOG("input models contains invalid model.");
      return;
    }

    strcpy(header, _header);
    strcpy(version, LTP_MODEL_VERSION);
    type = TYPE_HITSCIR_MERGE_MODEL;
    strcpy(reserved_field, "");

    ofs.write(header, HEADER_CHUNK_SIZE);
    ofs.write(version, VERSION_CHUNK_SIZE);
    ofs.write(reinterpret_cast<const char*>(&type), TYPE_CHUNK_SIZE);
    ofs.write(reserved_field, RESERVED_CHUNK_SIZE);

    uint32_t sz = static_cast<uint32_t>(models.size());
    ofs.write(reinterpret_cast<const char*>(&sz), sizeof(uint32_t));

    for(auto& m : models) {
      m -> save(ofs);
    }
    return;
  }



  bool load(std::istream & ifs) {
    clear();

    ifs.read(header, HEADER_CHUNK_SIZE);
    ifs.read(version, VERSION_CHUNK_SIZE);
    ifs.read(reinterpret_cast<char*>(&type), TYPE_CHUNK_SIZE);
    ifs.read(reserved_field, RESERVED_CHUNK_SIZE); //unused


    /*
     * compatible for loading uni model
     */
    if (type == TYPE_HITSCIR_NORMAL_MODEL) {
      ifs.clear();
      ifs.seekg(0, std::ios::beg);
      return load_as_uni_model(ifs);
    }

    if (type != TYPE_HITSCIR_MERGE_MODEL) {
      return false;
    }

    /*
     * in multi model, reserved_field is the #num of uni_models it has;
     */
    uint32_t num_models = 0;
    ifs.read(reinterpret_cast<char*>(&num_models), sizeof(uint32_t));
    if (num_models == 0) return false;

    /*
     * load uni models
     */
    for(int i = 0; i < num_models; ++i) {
      Model* model = new Model;
      if (!model->load(ifs)) {
        delete model;
        model = nullptr;
        clear();
        return false;
      }
      models.push_back(model);
    }

    return true;
  }

  void add(Model* model) {
    models.push_back(model);
  }

  size_t num_models() const {
    return models.size();
  }

  bool valid() {
    if (models.size() <= 1) {
      return true;
    }

    for (const auto& m : models) {
      if(strcmp(m->get_header(), models[0] -> get_header()) != 0) {
        return false;
      }
    }
    return true;
  }

protected:
  std::vector<Model*> models;


  bool load_as_uni_model(std::istream& ifs) {
    Model* model = new Model();
    if (!model->load(ifs)) {
      delete model;
      model = nullptr;
      return false;
    }
    models.push_back(model);
    return true;
  }

  void clear() {
    for(auto& m : models) {
      if (m) {
        delete m;
        m = nullptr;
      }
    }
    models.clear();
  }

};

}
}

#endif //LTP_LANGUAGE_TECHNOLOGY_PLATFORM_MERGE_MODEL_H
