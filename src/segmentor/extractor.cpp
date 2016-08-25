#include "segmentor/extractor.h"
#include "segmentor/settings.h"
#include "utils/strutils.hpp"
#include "utils/chartypes.hpp"
#include <vector>

namespace ltp {
namespace segmentor {

using utility::Template;
using utility::StringVec;

std::vector<Template *> Extractor::templates;

Extractor& Extractor::extractor() {
  static Extractor instance_;
  return instance_;
}

int Extractor::num_templates() {
  extractor();
  return templates.size();
}

Extractor::Extractor() {
  // delimit feature templates
  templates.push_back(new Template("1={c-2}"));
  templates.push_back(new Template("2={c-1}"));
  templates.push_back(new Template("3={c-0}"));
  templates.push_back(new Template("4={c+1}"));
  templates.push_back(new Template("5={c+2}"));
  templates.push_back(new Template("6={c-2}-{c-1}"));
  templates.push_back(new Template("7={c-1}-{c-0}"));
  templates.push_back(new Template("8={c-0}-{c+1}"));
  templates.push_back(new Template("9={c+1}-{c+2}"));
  templates.push_back(new Template("14={ct-1}"));
  templates.push_back(new Template("15={ct-0}"));
  templates.push_back(new Template("16={ct+1}"));
  templates.push_back(new Template("17={lex1}"));
  templates.push_back(new Template("18={lex2}"));
  templates.push_back(new Template("19={lex3}"));
}

Extractor::~Extractor() {
  for (size_t i = 0; i < templates.size(); ++ i) {
    delete templates[i];
  }
}

std::vector<std::vector<std::string>> Extractor::extract1o_new(const Instance &inst) {
  std::vector<std::vector<std::string>> inst_features_mat;

#define TYPE(x) (strutils::to_str(inst.chartypes[(x)]&0x07))
  std::vector<std::vector<std::string>> data_mat;
  for(auto idx = 0; idx < inst.size(); ++idx) {
    std::vector<std::string> datas;
    datas.push_back(inst.forms[idx]);
    datas.push_back(TYPE(idx));
    datas.push_back(strutils::to_str(inst.lexicon_match_state[idx] & 0x0f));
    datas.push_back(strutils::to_str((inst.lexicon_match_state[idx]>>4) & 0x0f));
    datas.push_back(strutils::to_str((inst.lexicon_match_state[idx]>>8) & 0x0f));
    data_mat.push_back(datas);
  }
#undef TYPE

  auto len = inst.size();
  for(auto idx = 0; idx < inst.size(); ++idx) {
    std::vector<std::string> features;
    auto& c_2 = idx-2 < 0 ? BOS : data_mat[idx-2][0];
    auto& c_1 = idx-1 < 0 ? BOS : data_mat[idx-1][0];
    auto& c_0 = data_mat[idx][0];
    auto& c__1 = idx+1 >= len ? EOS : data_mat[idx+1][0];
    auto& c__2 = idx+2 >= len ? EOS : data_mat[idx+2][0];
    features.push_back("1="+c_2);
    features.push_back("2="+c_1);
    features.push_back("3="+c_0);
    features.push_back("4="+c__1);
    features.push_back("5="+c__2);
    features.push_back("6="+c_2+"-"+c_1);
    features.push_back("7="+c_1+"-"+c_0);
    features.push_back("8="+c_0+"-"+c__1);
    features.push_back("9="+c__1+"-"+c__2);
    features.push_back("14="+(idx-1<0?BOT:data_mat[idx-1][1]));
    features.push_back("15="+data_mat[idx][1]);
    features.push_back("16="+(idx+1>=len?EOT:data_mat[idx+1][1]));
    features.push_back("17="+data_mat[idx][2]);
    features.push_back("18="+data_mat[idx][3]);
    features.push_back("19="+data_mat[idx][4]);
    inst_features_mat.push_back(std::move(features));
  }

  return inst_features_mat;
}


int Extractor::extract1o(const Instance& inst, int idx,
    std::vector<StringVec>& cache) {

  size_t len = inst.size();
  Template::Data data;

#define EQU(x, y) (inst.forms[(x)] == inst.forms[(y)])
#define TYPE(x) (strutils::to_str(inst.chartypes[(x)]&0x07))
  data.set( "c-2", (idx-2 < 0 ? BOS : inst.forms[idx-2]) );
  data.set( "c-1", (idx-1 < 0 ? BOS : inst.forms[idx-1]) );
  data.set( "c-0", inst.forms[idx] );
  data.set( "c+1", (idx+1 >= len ? EOS : inst.forms[idx+1]) );
  data.set( "c+2", (idx+2 >= len ? EOS : inst.forms[idx+2]) );
  data.set( "ct-1", (idx-1 < 0 ? BOT : TYPE(idx-1)) );
  data.set( "ct-0", TYPE(idx) );
  data.set( "ct+1", (idx+1 >= len ? EOT : TYPE(idx+1)) );
  data.set( "lex1", strutils::to_str(inst.lexicon_match_state[idx] & 0x0f));
  data.set( "lex2", strutils::to_str((inst.lexicon_match_state[idx]>>4) & 0x0f));
  data.set( "lex3", strutils::to_str((inst.lexicon_match_state[idx]>>8) & 0x0f));
#undef TYPE
#undef EQU

  std::string feat;
  feat.reserve(1024);
  // render features
  for (size_t i = 0; i < templates.size(); ++ i) {
    templates[i]->render(data, feat);
    cache[i].push_back(feat);
  }
  return 0;
}

}     //  end for namespace segmentor
}     //  end for namespace ltp
