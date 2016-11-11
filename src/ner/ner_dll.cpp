#include "ner/ner_dll.h"
#include "ner/multi_ner.h"
#include "ner/extractor.h"
#include "ner/settings.h"
#include "utils/logging.hpp"
#include "utils/codecs.hpp"
#include "utils/sbcdbc.hpp"
#include "utils/unordered_set.hpp"
#include "ner/decoder.h"
#include <iostream>
#include <fstream>
#include <memory>
#include "ner/settings.h"
#include "utils/model_helper.hpp"
#include "ner/multi_model.h"


class __ltp_dll_ner_wrapper : public ltp::ner::MultiNamedEntityRecognizer {
public:
  __ltp_dll_ner_wrapper() {}
  ~__ltp_dll_ner_wrapper() {
    clear();
  }


  void clear() {
    for(auto& ner : ners) {
      if (ner) {
        delete ner;
        ner = nullptr;
      }
      ners.clear();
    }
  }


  bool load(const char* model_file) {
    std::ifstream mfs(model_file, std::ifstream::binary);

    if (!mfs) {
      return false;
    }

    model = new ltp::ner::MultiModel;
    if (!model->load(mfs)) {
      delete model;
      model = nullptr;
      return false;
    }

    for (auto i = 0; i < model->num_models(); ++i) {
      auto uni_model = model->get(i);
      auto ner = new ltp::ner::NamedEntityRecognizer((ltp::ner::Model*)uni_model);
      ners.push_back(ner);
    }

    return true;
  }

  int recognize(const std::vector<std::string> & words,
      const std::vector<std::string> & postags,
      std::vector<std::string> & tags) {

    if (words.size() == 0) {
      return 0;
    }

    if (words.size() != postags.size()) {
      return 0;
    }

    ltp::framework::ViterbiFeatureContext ctx;
    ltp::framework::ViterbiScoreMatrix scm;
    ltp::ner::NERViterbiDecoderWithMarginal decoder;
    ltp::ner::Instance inst;

    for (size_t i = 0; i < words.size(); ++ i) {
      inst.forms.push_back(ltp::strutils::chartypes::sbc2dbc_x(words[i]));
      inst.postags.push_back(postags[i]);
    }

    /*
     * if we only have ONE ne model, we can directly recognize
     * else we should resolve conflict results between the different ner
     */
    if (num_models() == 1) {
      auto ner = ners.at(0);
      ner->extract_features(inst, &ctx, false);
      ner->calculate_scores(inst, ctx, true, &scm);
      decoder.decode(scm, *(ner->get_glob_con()), inst.predict_tagsidx);

      for (size_t i = 0; i < words.size(); ++i) {
        tags.push_back(ner->get_model()->labels.at((size_t)inst.predict_tagsidx[i]));
      }

      return (int)tags.size();
    }

    /*
     * store results from each ne recognizer
     * output: label, represented by idx
     * partial_prob: prob of each ne (including 'O')
     * partial_idx: start idx of each ne
     */
    std::vector<std::vector<int>> outputs;
    std::vector<std::vector<double>> partial_probs;
    std::vector<std::vector<int>> partial_idxs;

    /*
     * we begin using each ne recognizer here
     */
    decoder.set_marginal_prob(true);
    decoder.set_sequence_prob(true);
    for(auto& ner : ners) {
      ner->extract_features(inst, &ctx, false);
      ner->calculate_scores(inst, ctx, true, &scm);

      std::vector<int> output;
      double sequence_prob;
      std::vector<double> point_prob;
      std::vector<double> partial_prob;
      std::vector<int> partial_idx;

      decoder.decode(scm,
                     *(ner->get_glob_con()),
                     output,
                     sequence_prob,
                     point_prob,
                     partial_prob,
                     partial_idx,
                     true,
                     ner->get_model()->param._last_timestamp);

      outputs.emplace_back(output);
      partial_probs.emplace_back(partial_prob);
      partial_idxs.emplace_back(partial_idx);
    }

    /*
     * then we should resolve conflict labels between the outputs from different ner
     */
    for (int i = 0; i < words.size();) {
      /*
       * if all the result is not B or S, then we just tag O
       *
       *   *-O-O
       *   *-I-E
       *   *-E-O
       *     ^
       *
       */
      if (all_is_not_begin_tag(outputs, i)) {
        tags.push_back("O");
        i++;
        continue;
      }

      /*
       * find the max-len entity at i,
       * if more than one entity has max-len, we choose the max-prob entity
       * if tag is O, I, E, length is 0
       *
       * e.g.
       *
       *   *-B-I-I-E-    len=4  prob=0.2
       *   *-B-I-I-E-    len=4  prob=0.7  <--key_output
       *   *-B-I-E-O-    len=3
       *   *-S-B-E-O-    len=1
       *   *-I-E-O-O-    len=0
       *   *-E-O-O-O-    len=0
       *   *-O-B-E-O-    len=0
       *     ^
       *     i
       *
       * return:
       *   key_output_idx=1
       *   max_entity_len=4
       *   max_prob=0.7
       */
      int key_output_idx = 0;
      int max_entity_len = 0;
      double max_entity_prob = 0;

      for (int j = 0; j < outputs.size(); ++j) {
        int len = 0;
        double prob = 0;
        if(is_tag_S(outputs[j][i])) {
          len = 1;
          prob = get_entity_prob(partial_probs[j], partial_idxs[j], i);
        } else if (is_tag_B(outputs[j][i])) {
          while ((i + len < words.size())
                 && !is_tag_E(outputs[j][i+len]))
            len ++;
          len++;
          prob = get_entity_prob(partial_probs[j], partial_idxs[j], i);
        }
        if (len != 0
            && (len > max_entity_len
                || (len == max_entity_len && prob > max_entity_prob))) {
          key_output_idx = j;
          max_entity_len = len;
          max_entity_prob = prob;
        }
      }


      /*
       * check if conflict at the tail tag
       *
       * e.g.
       *
       *   B-I-E-O  <-- key
       *   O-O-B-E  <-- conflict=true
       *   O-O-S-B  <-- conflict=false
       *   O-B-I-E  <-- conflict=true
       *   O-S-O-O  <-- conflict=false
       * B-I-I-I-E  <-- conflict=false
       *       ^
       */
      std::vector<int> conflict_output_idxs;
      for (int j = 0; j < outputs.size(); ++j) {
        if (j == key_output_idx) continue;
        int tag = outputs[j][i+max_entity_len-1];
        if (is_tag_B(tag)
            || (is_tag_I(tag)
                && get_entity_begin_idx(outputs[j], i) > i )) {
          conflict_output_idxs.push_back(j);
        }
      }


      bool conflict = !conflict_output_idxs.empty();
      if (!conflict) {
        /*
         * if not conflict at the tail tag,
         * we choose the longest entity tag with max prob previously found
         */
        for(int j = 0; j < max_entity_len; ++j) {
          int tag_idx = outputs[key_output_idx][i+j];
          std::string tag_string = model->get(key_output_idx)->labels.at(tag_idx);
          tags.push_back(tag_string);
        }
        i += max_entity_len;
      } else {
        /*
         * if conflict at the tail tag, we choose the max prob tag
         */
        for(int j = 0; j < conflict_output_idxs.size(); ++j) {
          double entity_prob = get_entity_prob(partial_probs[j],
                                                   partial_idxs[j],
                                                   i + max_entity_len -1);
          if (entity_prob > max_entity_prob) {
            max_entity_prob = entity_prob;
            key_output_idx = j;
          }
        }

        /*
         * copy the result from the outputs with max prob
         */
        for(int j = 0; j < max_entity_len; ++j) {
          int tag_idx = outputs[key_output_idx][i+j];
          std::string tag_string = model->get(key_output_idx)->labels.at(tag_idx);
          tags.push_back(tag_string);
        }
        i += max_entity_len;
        // copy until the end of of the entity
        for (;i < words.size(); ++i) {
          int tag_idx = outputs[key_output_idx][i];
          if (is_tag_O(tag_idx) || is_tag_S(tag_idx) || is_tag_B(tag_idx)) {
            break;
          }
          std::string tag_string = model->get(key_output_idx)->labels.at(tag_idx);
          tags.push_back(tag_string);
        }
      }
    }
    return (int)tags.size();
  }

protected:
  bool all_is_tag_O(const std::vector<std::vector<int>>& outputs, size_t idx) const {
    for(size_t i = 0; i < outputs.size(); ++i) {
      if (!is_tag_O(outputs[i][idx])) {
        return false;
      }
    }
    return true;
  }

  bool all_is_not_begin_tag(const std::vector<std::vector<int>>& outputs, size_t idx) const {
    for(size_t i = 0; i < outputs.size(); ++i) {
      if (is_tag_B(outputs[i][idx]) || is_tag_S(outputs[i][idx])) {
        return false;
      }
    }
    return true;
  }

  double get_entity_prob(const std::vector<double>& partial_prob,
                      const std::vector<int>& partial_idx,
                      int idx) const {
    for(int i = 0; i < partial_idx.size(); ++i) {
      if (partial_idx[i] > idx) {
        return partial_prob[i-1];
      }
    }
    return 0;
  }

  int get_entity_begin_idx(const std::vector<int>& output, int idx) const {
    if (is_tag_O(output[idx]) || is_tag_S(output[idx])) {
      return idx;
    }

    while(!is_tag_B(output[idx])) idx--;
    return idx;
  }


  bool is_tag_O(int tag_idx) const {
    return tag_idx == 0;
  }

  bool is_tag_B(int tag_idx) const {
    if (tag_idx <= 0) return false;
    return ((tag_idx-1) % ltp::ner::__num_pos_types__) == 0;
  }

  bool is_tag_I(int tag_idx) const {
    if (tag_idx <= 0) return false;
    return ((tag_idx-1) % ltp::ner::__num_pos_types__) == 1;
  }

  bool is_tag_E(int tag_idx) const {
    if (tag_idx <= 0) return false;
    return ((tag_idx-1) % ltp::ner::__num_pos_types__) == 2;
  }

  bool is_tag_S(int tag_idx) const {
    if (tag_idx <= 0) return false;
    return ((tag_idx-1) % ltp::ner::__num_pos_types__) == 3;
  }

};

void * ner_create_recognizer(const char * path) {
  __ltp_dll_ner_wrapper* wrapper = new __ltp_dll_ner_wrapper();

  if (!wrapper->load(path)) {
    delete wrapper;
    return 0;
  }

  return reinterpret_cast<void *>(wrapper);
}

int ner_release_recognizer(void * ner) {
  if (!ner) {
    return -1;
  }
  delete reinterpret_cast<__ltp_dll_ner_wrapper *>(ner);
  return 0;
}

int ner_recognize(void * ner,
    const std::vector<std::string> & words,
    const std::vector<std::string> & postags,
    std::vector<std::string> & tags) {

  if (words.size() == 0) {
    return 0;
  }

  if (words.size() != postags.size()) {
    return 0;
  }


  for (int i = 0; i < words.size(); ++ i) {
    if (words[i].empty() || postags.empty()) {
      return 0;
    }
  }

  __ltp_dll_ner_wrapper* wrapper = 0;
  wrapper = reinterpret_cast<__ltp_dll_ner_wrapper*>(ner);
  return wrapper->recognize(words, postags, tags);
}
