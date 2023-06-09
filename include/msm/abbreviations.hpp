#pragma once
#include <string>
#include <msm/languages.hpp>
/* maximum key range = 2008, duplicates = 0 */

class abbreviations
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static const char *find (const char *str, size_t len);

  private:
    class word {
          
    public:
      static inline void set(language lang, const char *src, size_t size) {
        buf[0] = (char)lang;
        memcpy(&buf[1], src, size);
        word::_size = size + 1;
        buf[word::_size] = 0;
      }
      static inline const char *get() {
          return &buf[0];
      }
      static inline size_t size() {
        return word::_size;
      }
      static constexpr size_t capacity = 8;
    private:
        static char buf[capacity];
        static size_t _size;
  };
  public:
  static inline bool match(const char * first, const char *cur, language lang) {
    const char *loc = cur, *beg = std::max(first, cur - word::capacity);
    while (true) {
        cur--;
        if (cur < beg) {
            return false;
        }
        if (*cur == ' ' || *cur == '\n' || *cur == '\r' || *cur == '\t') {
            break;
        }
        if (*cur == -0x80 && (cur > first + 1)) {
          const char *tmp = cur - 1;
          if (*tmp-- == -0x80 && *tmp == -29)
            break;
        }
    }
    cur++;
    word::set(lang, cur, loc-cur);
    
    return bool(abbreviations::find(word::get(), word::size()));
  }
  
  static inline bool match(const uint8_t *first, const uint8_t *cur, language lang) {
    return match((const char*)first, (const char*)cur, lang);
  }
};