#include <iostream>
#include "config.h"
#include "boost/program_options.hpp"
#include "utils/logging.hpp"
#include "postagger/postagger_frontend.h"
#include "postagger/options.h"

#define DESCRIPTION "Part of Speech Tagging"
#define EXECUTABLE "otpos"

using boost::program_options::options_description;
using boost::program_options::value;
using boost::program_options::variables_map;
using boost::program_options::store;
using boost::program_options::parse_command_line;
using boost::program_options::notify;
using ltp::postagger::PostaggerFrontend;
using ltp::postagger::TrainOptions;
using ltp::postagger::TestOptions;
using ltp::postagger::DumpOptions;

int learn(int argc, const char* argv[]) {
  std::string usage = EXECUTABLE "(learn) in LTP " LTP_VERSION " - (C) 2012-2015 HIT-SCIR\n";
  usage += "Training suite for " DESCRIPTION "\n";
  usage += "usage: ./" EXECUTABLE " learn <options>\n\n";
  usage += "options:";

  TrainOptions opt;
  options_description optparser(usage);
  optparser.add_options()
    ("model", value<std::string>(&opt.model_name)->required(),
     "The prefix of the model file, model will be stored as model.$iter.")
    ("reference", value<std::string>(&opt.train_file)->required(), "The path to the reference file.")
    ("development", value<std::string>(&opt.holdout_file)->default_value(""), "The path to the development file.")
    ("algorithm", value<std::string>(&opt.algorithm)->default_value("pa"), "The learning algorithm\n"
                                          " - ap: averaged perceptron\n"
                                          " - pa: passive aggressive [default]")
    ("max-iter", value<size_t>(&opt.max_iter)->default_value(10), "The number of iteration [default=10].")
    ("rare-feature-threshold", value<size_t>(&opt.rare_feature_threshold)->default_value(0),
     "The threshold for rare feature, used in model truncation. [default=0]")
    ("help,h", "Show help information");

  if (argc == 1) { std::cerr << optparser << std::endl; return 1; }

  variables_map vm;
  try{
    store(parse_command_line(argc, argv, optparser), vm);
    if (vm.count("help")) {
      std::cerr << optparser << std::endl;
      return 0;
    }
    notify(vm);
  } catch(const boost::program_options::error &e) {
    std::cerr << optparser << std::endl;
    std::cerr << e.what() << std::endl;
    return 1;
  }

  if (opt.holdout_file.empty()) {
    WARNING_LOG("development file is not configed, evaluation will not be performed.");
  }

  if (opt.algorithm != "pa" && opt.algorithm != "ap") {
    WARNING_LOG("algorithm should either be ap or pa, set as default [pa].");
    opt.algorithm = "pa";
  }


  PostaggerFrontend frontend(opt);
  frontend.train();
  return 0;
}

int test(int argc, const char* argv[]) {
  std::string usage = EXECUTABLE "(test) in LTP " LTP_VERSION " - (C) 2012-2015 HIT-SCIR\n";
  usage += "Testing suite for " DESCRIPTION "\n\n";
  usage += "usage: ./" EXECUTABLE " test <options>\n\n";
  usage += "options:";

  TestOptions opt;
  options_description optparser(usage);
  optparser.add_options()
    ("model", value<std::string>(&opt.model_file)->required(), "The path to the model file.")
    ("lexicon", value<std::string>(&opt.lexicon_file)->default_value(""),
     "The lexicon file, (optional, if configured, constrained decoding will be performed).")
    ("input", value<std::string>(&opt.test_file), "The path to the reference file.")
    ("evaluate", value<bool>(&opt.evaluate)->default_value(false),
     "if configured, perform evaluation, input should contain '_' concatenated tag")
    ("sequence", value<bool>(&opt.sequence_prob)->default_value(false), "Output the probability of the label sequences")
    ("marginal", value<bool>(&opt.marginal_prob)->default_value(false), "Output the marginal probabilities of tags")
    ("help,h", "Show help information");

  if (argc == 1) { std::cerr << optparser << std::endl; return 1; }

  variables_map vm;
  try {
    store(parse_command_line(argc, argv, optparser), vm);
    if (vm.count("help")) {
      std::cerr << optparser << std::endl;
      return 1;
    }
    notify(vm);
  } catch(const boost::program_options::error &e) {
    std::cerr << optparser << std::endl;
    std::cerr << e.what() << std::endl;
    return 1;
  }

  PostaggerFrontend frontend(opt);
  frontend.test();
  return 0;
}

int dump(int argc, const char* argv[]) {
  std::string usage = EXECUTABLE "(dump) in LTP " LTP_VERSION " - (C) 2012-2015 HIT-SCIR\n";
  usage += "Model visualization suite for " DESCRIPTION "\n";
  usage += "usage: ./" EXECUTABLE " dump <options>\n\n";
  usage += "options:";

  DumpOptions opt;
  options_description optparser(usage);
  optparser.add_options()
    ("model", value<std::string>(&opt.model_file)->required(), "The path to the model file.")
    ("help,h", "Show help information");

  if (argc == 1) { std::cerr << optparser << std::endl; return 1; }

  variables_map vm;

  try {
    store(parse_command_line(argc, argv, optparser), vm);
    if (vm.count("help")) {
      std::cerr << optparser << std::endl;
      return 0;
    }
    notify(vm);
  } catch(const boost::program_options::error &e) {
    std::cerr << optparser << std::endl;
    std::cerr << e.what() << std::endl;
    return 1;
  }

  PostaggerFrontend frontend(opt);
  frontend.dump();
  return 0;
}

int main(int argc, const char* argv[]) {
  std::string usage = EXECUTABLE " in LTP " LTP_VERSION " - (C) 2012-2015 HIT-SCIR\n";
  usage += "Training and testing suite for " DESCRIPTION "\n\n";
  usage += "usage: ./" EXECUTABLE " [learn|test|dump] <options>";

  if (argc == 1) {
    std::cerr << usage << std::endl;
    return 1;
  } else if (std::string(argv[1]) == "learn") {
    return learn(argc- 1, argv+ 1);
  } else if (std::string(argv[1]) == "test") {
    return test(argc- 1, argv+ 1);
  } else if (std::string(argv[1]) == "dump") {
    return dump(argc- 1, argv+ 1);
  } else {
    std::cerr << "unknown mode: " << argv[1] << std::endl;
    std::cerr << usage << std::endl;
    return 1;
  }
  return 0;
}
