from __init__ import INCLUDE_DIR, SOURCE_DIR, PROJECT_NAME
from languages import languages
from subprocess import run

import os
dir_path = os.path.dirname(os.path.realpath(__file__))




def tokenize(directory : str):
    f = open(directory, 'r')

    language = ''
    abbreviations = []
    abbreviation = ''
    while True:
        c = f.read(1)
        if not len(c):
            if len(language):
                yield language, abbreviations
            return
        match c:
            case '\n' | ' ' | '\r' | '\u3000' | '\t':
                pass
            case ':':
                while True:
                    c = f.read(1)
                    if not len(c):
                        if len(abbreviations):
                            yield language, abbreviations
                        return
                    match c:
                        case '\n' | ' ' | '\r' | '\u3000' | '\t':
                            if len(abbreviation): abbreviations.append(abbreviation)
                            abbreviation = ''
                            pass
                        case ';':
                            if len(abbreviation): abbreviations.append(abbreviation)
                            break
                        case _:
                            abbreviation += c
                yield language, abbreviations
                language = ''
                abbreviations = []
                
            case _:
                language += c

def gperf_create_file(gperf_in_dir : str, data_dir : str) -> int:
    gperf_file = open(gperf_in_dir, 'w')
    text = ""

    
    max_length = 0
    for language, abbreviations in tokenize(data_dir):
        duplicates = set()
        value = languages[language].value
        for abbreviation in abbreviations:
            max_length = max(len(abbreviation), max_length)
            if abbreviation in duplicates: continue
            duplicates.add(abbreviation)
            if value == ord('"'):
                text += f"\"\\\"{abbreviation}\"\n"
                pass
            elif value < ord('!'):
                tmp = ''.join([f"\\x{c:02X}" for c in bytes(abbreviation, 'utf-8')])
                text += f"\"\\x{value:02X}{tmp}\"\n"
            else:
                text += f"{chr(value)}{abbreviation}\n"
    gperf_file.write(text)
    gperf_file.close()

    print(max_length)
    return max_length


def gperf_process_files(name : str, max_size : int, gperf_out_dir : str, gperf_out_include_dir : str):
    gperf_out_code = open(gperf_out_dir, 'r').read()
    i : int = gperf_out_code.find("/* m")
    j : int = gperf_out_code.find("};",i)
    #print()
    
    gperf_out_include_file = open(gperf_out_include_dir, 'w')
    gperf_out_include_file.write(f"""\
#pragma once
#include <string>
#include <{PROJECT_NAME}/languages.hpp>
{gperf_out_code[i:j]}
  private:
    class word {{
          
    public:
      static inline void set(language lang, const char *src, size_t size) {{
        buf[0] = (char)lang;
        memcpy(&buf[1], src, size);
        word::_size = size + 1;
        buf[word::_size] = 0;
      }}
      static inline const char *get() {{
          return &buf[0];
      }}
      static inline size_t size() {{
        return word::_size;
      }}
      static constexpr size_t capacity = {max_size+2};
    private:
        static char buf[capacity];
        static size_t _size;
  }};
  public:
  static inline bool match(const char * first, const char *cur, language lang) {{
    const char *loc = cur, *beg = std::max(first, cur - word::capacity);
    while (true) {{
        cur--;
        if (cur < beg) {{
            return false;
        }}
        if (*cur == ' ' || *cur == '\\n' || *cur == '\\r' || *cur == '\\t') {{
            break;
        }}
        if (*cur == -0x80 && (cur > first + 1)) {{
          const char *tmp = cur - 1;
          if (*tmp-- == -0x80 && *tmp == -29)
            break;
        }}
    }}
    cur++;
    word::set(lang, cur, loc-cur);
    
    return bool({name}::find(word::get(), word::size()));
  }}
  
  static inline bool match(const uint8_t *first, const uint8_t *cur, language lang) {{
    return match((const char*)first, (const char*)cur, lang);
  }}
{'};'}""")
    gperf_out_include_file.close()

    gperf_out_file = open(gperf_out_dir, 'w')

    gperf_out_file.write(f"""\
#include <{PROJECT_NAME}/{name}.hpp>
{gperf_out_code[:i]}
{gperf_out_code[j+2:].replace('register ','')}

char {name}::word::buf[{name}::word::capacity];
size_t {name}::word::_size;
"""
    )
    gperf_out_file.close()




def gperf_execute(name : str, data_dir : str):
    gperf_in_dir = f"{dir_path}/{name}.gperf"
    max_size = gperf_create_file(gperf_in_dir, data_dir)
    gperf_out_dir = f"{SOURCE_DIR}/{name}.cpp"
    gperf_out_include_dir = f"{INCLUDE_DIR}/{name}.hpp"
    
    run(
        [
        "gperf",
            f"{gperf_in_dir}",
            "--language=C++",
            f"--class-name={name}",
            "--lookup-function-name=find",
            "--includes",
            f"--output-file={gperf_out_dir}"
        ]
    )
    gperf_process_files(name, max_size, gperf_out_dir, gperf_out_include_dir)

def main():
    gperf_execute("abbreviations", f"{dir_path}/data.txt")
    

    


if __name__ == '__main__':
    main()