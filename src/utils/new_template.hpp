#ifndef __NEW_TEMPLATE_HPP__
#define __NEW_TEMPLATE_HPP__

#include "utils/unordered_map.hpp"
#include <cstring>
#include <vector>

namespace ltp {
namespace utility {
  class NewTemplate {
  public:

    class Data {
    public:
      typedef std::unordered_map<std::string, std::function<std::string(int)>> func_map;

      void bind(const std::string& ft_name,
                const std::function<std::string(int)>& ft_func) {
        ft_func_map[ft_name] = ft_func;
      }

      std::string get(const std::string& ft_name, int idx) {
        auto find_cache = cache[ft_name].find(idx);
        if (find_cache != cache[ft_name].end()) {
          return find_cache->second;
        }

        auto ret = ft_func_map[ft_name](idx);
        cache[ft_name][idx] = ret;
        return ret;
      }

      void clear() {
        ft_func_map.clear();
        cache.clear();
      }

    protected:
      func_map ft_func_map;
      std::unordered_map<std::string, std::unordered_map<int, std::string>> cache;
    };

    NewTemplate (const char * tempstr):items(0), buffer(0) {
      int len = strlen(tempstr);
      buffer = new char[len + 1];
      memcpy(buffer, tempstr, len + 1);

      /* get number of tokens in the template
       *  e.g:
       *      1={c-1}-{c-0}
       *
       *      4 tokens: "1=", "c-1", "-", "c-0"
       *
       * loop over all the tokens and push them into the cache.
       *
       * after this, buffer = "1=\0c\0\0\0-\0c\0\0\0" + "\0"
       */

      int right_bracket = -1;
      int left_bracket = -1;
      const char * s = NULL;
      for (s = buffer; *s; ++ s) {
        if ((*s) == '{') {
          left_bracket = s - buffer;
          if (right_bracket + 1 < left_bracket) {
            buffer[left_bracket] = 0;
            auto token = std::make_shared<Token>(0, buffer+right_bracket+1, 0);
            items.push_back(token);
          }
        }

        if ((*s) == '}') {
          right_bracket = s - buffer;
          buffer[right_bracket] = 0;

          char * p = buffer + left_bracket + 1;
          int sign = 1;
          int offset = 0;
          for (char * p = buffer + left_bracket + 1; p != buffer+right_bracket ; ++p){
            if ((*p) == '-' || (*p) == '+') {
              sign = (*p == '+') ? 1 : -1;
              *p++ = 0;
              offset = std::atoi(p);
              break;
            }
          }
          auto token = std::make_shared<Token>(1, buffer+left_bracket+1, offset * sign);
          items.push_back(token);
        }
      }
    }

    std::string render(Data& data, int idx) {
      std::string ret;

      for (const auto& item : items) {
        if (item -> type == 0) {
          ret.append(std::string(item->payload));
        } else if (item -> type == 1)  {
          ret.append(data.get(item->payload, idx+item->offset));
        }
      }
      return ret;
    }

    ~NewTemplate() {
      if (buffer) {
        delete []buffer;
        buffer = 0;
      }
    }

  protected:

    /*
     * e.g. tempstr = "1={c-1}"
     *
     *     2 token: "1=", "c-1"
     *
     *     after init object, buffer = "1=\0c\0\0\0" + "\0"
     *
     *     for token "1=",  type = 0 (no need render), payload = point to it, offset = useless
     *     for token "c-1", type = 1 (need render), payload = point to "c", offset = -1
     */
    struct Token {
      Token(int _type, char * _payload, int _offset): type(_type), payload(_payload), offset(_offset) {}

      int type;
      char * payload;
      int offset;
    };

    std::vector<std::shared_ptr<Token>> items;
    char * buffer;

  };

}
}
#endif
