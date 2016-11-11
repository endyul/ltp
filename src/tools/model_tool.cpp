//
// Created by Zixiang Xu on 2016/10/25.
//

#include "model_tool.h"
#include <vector>
#include "config.h"
#include "boost/program_options.hpp"
#include "utils/logging.hpp"
#include <iostream>
#include <fstream>
#include "framework/multi_model.h"
#include "framework/base_model.h"


#define EXECUTABLE "model_tool"

using boost::program_options::options_description;
using boost::program_options::value;
using boost::program_options::variables_map;
using boost::program_options::store;
using boost::program_options::parse_command_line;
using boost::program_options::command_line_parser;
using boost::program_options::positional_options_description;

using ltp::framework::MultiModel;

bool merge_models(const std::vector<MultiModel*>& multi_models, std::ofstream& ofs) {
  if (multi_models.size() == 0) return false;
  auto base_model = new ltp::framework::BaseModel();

  uint32_t ct = 0;
  auto header = multi_models[0]->get(0)->get_header();
  for(const auto& multi_model : multi_models) {
    for(int i = 0; i < multi_model -> num_models(); ++i) {
      ct++;
      if (strcmp(header, multi_model->get(i)->get_header()) != 0) {
        std::cerr << "invalid models!" << std::endl;
        return false;
      }
    }
  }

  base_model->set_header(header);
  base_model->set_type(ltp::framework::BaseModel::TYPE_HITSCIR_MERGE_MODEL);
  base_model->save(ofs);
  ofs.write(reinterpret_cast<const char*>(&ct), sizeof(uint32_t));
  for(const auto& multi_model : multi_models) {
    for(int i = 0; i < multi_model -> num_models(); ++i) {
      multi_model->get(i)->save(ofs);
    }
  }
  return true;
}

int merge(int argc, const char* argv[]) {
  std::string usage = EXECUTABLE " in LTP " LTP_VERSION " - " LTP_COPYRIGHT "\n";
  usage += "model merge tool for merge-ner task\n\n";
  usage += "usage: ./" EXECUTABLE " merge <options>\n\n";
  usage += "options";

  options_description optparser(usage);
  optparser.add_options()
      ("input,I", value<std::vector<std::string>>(), "model paths")
      ("output,o", value<std::string>(), "output model")
      ("help,h", "Show help information");

  if (argc == 1) {
    std::cerr << optparser << std::endl;
    return 1;
  }

  variables_map vm;
  store(parse_command_line(argc, argv, optparser), vm);

  if (vm.count("help")) {
    std::cerr << optparser << std::endl;
    return 0;
  }

  std::vector<std::string> input_files;
  if (!vm.count("input")) {
    ERROR_LOG("input model should be specified [--input]");
    return 1;
  } else {
    input_files = vm["input"].as<std::vector<std::string>>();
  }

  std::string output_model_path;
  if (!vm.count("output")) {
    ERROR_LOG("output model name should be specified [--output].");
    return 1;
  } else {
    output_model_path= vm["output"].as<std::string>();
  }

  INFO_LOG("models to be merged:");
  std::for_each(input_files.cbegin(),
                input_files.cend(),
                [](const std::string& val){INFO_LOG("\t%s",val.c_str());});

  std::vector<MultiModel*> multi_models;
  for(auto& file_path : input_files) {
    auto multi_model = new MultiModel;
    std::ifstream mfs(file_path, std::ifstream::binary);
    multi_model->load(mfs);
    multi_models.push_back(multi_model);
  }

  std::ofstream ofs(output_model_path.c_str(), std::ofstream::binary);
  auto r = merge_models(multi_models, ofs);
  return r;
}

int info(int argc, const char* argv[]) {
  return 0;
}

int main(int argc, const char* argv[]) {
  std::string usage = EXECUTABLE " in LTP " LTP_VERSION " - " LTP_COPYRIGHT "\n";
  usage += "model tool for LTP model\n\n";
  usage += "usage: ./" EXECUTABLE " [merge|info] <options>";

  if (argc == 1) {
    std::cerr << usage << std::endl;
    return 1;
  } else if (std::string(argv[1]) == "merge") {
    return merge(argc-1, argv+1);
  } else if (std::string(argv[1]) == "info") {
    return info(argc-1, argv+1);
  } else {
    std::cerr << "unknown mode: " << argv[1] << std::endl;
    std::cerr << usage << std::endl;
    return 1;
  }

  return 0;
}

