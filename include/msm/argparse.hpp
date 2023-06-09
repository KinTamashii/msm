#pragma once
#include <map>
#include <msm/languages.hpp>
#include <ktu/string.hpp>


language parseLanguage(const char *s);

inline bool stobool(const char *s) {
    return !strcmp(s, "true") || ktu::ston(s);
}
unsigned long mstou(const char *parameter);

char *quote(char *s);
char *splitArgAssignment(char *arg);
char *splitArgNextParameter(char *arg);

struct multiSplitParameter_t {
    char *parameter, terminator;
    enum : char {
        base = ',',
        super = ':',
        null = '\0'
    };
};

multiSplitParameter_t multiSplitArgNextParameter(char *arg);




std::string parse(const char *s, std::map<std::string, std::string> &vars);

const char *quote(std::string &str, const char* s);
const char *macro(std::string &str, const char *s, std::map <std::string, std::string> &vars);
