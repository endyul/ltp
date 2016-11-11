#include "segmentor/extractor.h"
#include "segmentor/settings.h"
#include "utils/strutils.hpp"
#include "utils/chartypes.hpp"

namespace ltp {
namespace segmentor {

using utility::NewTemplate;

std::vector<NewTemplate *> Extractor::templates;
utility::NewTemplate::Data Extractor::data;
Extractor::ft_funcs_type Extractor::ft_funcs;

Extractor& Extractor::extractor() {
  static Extractor instance_;
  return instance_;
}

size_t Extractor::num_templates() {
  extractor();
  return templates.size();
}

Extractor::Extractor() {
  // delimit feature templates
  templates.push_back(new NewTemplate("1={c-2}"));
  templates.push_back(new NewTemplate("2={c-1}"));
  templates.push_back(new NewTemplate("3={c-0}"));
  templates.push_back(new NewTemplate("4={c+1}"));
  templates.push_back(new NewTemplate("5={c+2}"));
  templates.push_back(new NewTemplate("6={c-2}-{c-1}"));
  templates.push_back(new NewTemplate("7={c-1}-{c-0}"));
  templates.push_back(new NewTemplate("8={c-0}-{c+1}"));
  templates.push_back(new NewTemplate("9={c+1}-{c+2}"));
  templates.push_back(new NewTemplate("14={ct-1}"));
  templates.push_back(new NewTemplate("15={ct-0}"));
  templates.push_back(new NewTemplate("16={ct+1}"));
  templates.push_back(new NewTemplate("17={lex1}"));
  templates.push_back(new NewTemplate("18={lex2}"));
  templates.push_back(new NewTemplate("19={lex3}"));

  ft_funcs["c"] = [](const Instance& inst, int idx) -> std::string {
    if (idx < 0) return BOS;
    if (idx >= inst.size()) return EOS;
    return inst.forms[idx];
  };

  ft_funcs["ct"] = [](const Instance& inst, int idx) -> std::string {
    if (idx < 0) return BOT;
    if (idx >= inst.size()) return EOT;
    return strutils::to_str(inst.chartypes[idx]&0x07);
  };

  ft_funcs["lex1"] = [](const Instance& inst, int idx) -> std::string {
    return strutils::to_str(inst.lexicon_match_state[idx] & 0x0f);
  };

  ft_funcs["lex2"] = [](const Instance& inst, int idx) -> std::string {
    return strutils::to_str((inst.lexicon_match_state[idx] >> 4) & 0x0f);
  };

  ft_funcs["lex3"] = [](const Instance& inst, int idx) -> std::string {
    return strutils::to_str((inst.lexicon_match_state[idx] >> 8) & 0x0f);
  };
}

Extractor::~Extractor() {
  for (size_t i = 0; i < templates.size(); ++ i) {
    delete templates[i];
  }
}


std::vector<std::vector<std::string>> Extractor::extract1o_new(const Instance &inst) {

  std::vector<std::vector<std::string>> inst_features_mat;

  data.clear();
  for (const auto& item: ft_funcs) {
    data.bind(item.first, std::bind(item.second, inst, std::placeholders::_1));
  }

  for (auto idx = 0; idx < inst.size(); ++idx) {
    std::vector<std::string> features;
    for(const auto& templ : templates) {
      auto feature = templ->render(data, idx);
      features.emplace_back(feature);
    }
    inst_features_mat.push_back(features);
  }

  return inst_features_mat;
}



}     //  end for namespace segmentor
}     //  end for namespace ltp
