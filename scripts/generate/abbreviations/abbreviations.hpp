#pragma once
#include <string>
#include <msbt-editor++/languages.hpp>
/* maximum key range = 902, duplicates = 0 */

class abbreviations
{
private:
  static inline unsigned int hash (const char *str, unsigned int len);
public:
  static const char *find (const char *str, unsigned int len);

  inline bool abbreviations::match(const char *str, language lang) {
    std::string s{lang};
    s += str;
    return bool(abbreviations::find(s.c_str(), s.size()));
  }
};