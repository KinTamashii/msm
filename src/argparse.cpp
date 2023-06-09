#include <msm/argparse.hpp>
#include <string>
#include <ktu/algorithm.hpp>



language parseLanguage(const char *s) {
    switch (ktu::hash(s)) {
        case ktu::hash("danish"):
        case ktu::hash("da"):
            return language::Danish;

        case ktu::hash("dutch"):
        case ktu::hash("nl"):
            return language::Dutch;

        case ktu::hash("english"):
        case ktu::hash("en"):
            return language::English;

        case ktu::hash("finnish"):
        case ktu::hash("fi"):
            return language::Finnish;

        case ktu::hash("french"):
        case ktu::hash("fr"):
            return language::French;

        case ktu::hash("german"):
        case ktu::hash("de"):
            return language::German;

        case ktu::hash("greek"):
        case ktu::hash("el"):
            return language::Greek;

        case ktu::hash("hungarian"):
        case ktu::hash("hu"):
            return language::Hungarian;

        case ktu::hash("italian"):
        case ktu::hash("it"):
            return language::Italian;

        case ktu::hash("norwegian"):
        case ktu::hash("no"):
            return language::Norwegian;

        case ktu::hash("romanian"):
        case ktu::hash("ro"):
            return language::Romanian;

        case ktu::hash("russian"):
        case ktu::hash("ru"):
            return language::Russian;

        case ktu::hash("spanish"):
        case ktu::hash("es"):
            return language::Spanish;

        case ktu::hash("swedish"):
        case ktu::hash("sv"):
            return language::Swedish;
        
        default:
        case ktu::hash("unknown"):
        case ktu::hash("unk"):
            return language::Unknown;
    }
}
#include <ktu/memory/buffer.hpp>

unsigned long mstou(const char *parameter) {
    
    if (*parameter == '0') {
        ++parameter;
        
        if (*parameter == 'x') {
            ++parameter;
            return ktu::readHex(parameter);
        }
        if (*parameter == 'b') {
            ++parameter;
            return ktu::readBin(parameter);
        }
        return ktu::readOct(parameter);
    }
    char c = *parameter;
    if ('0' < c && c < ':') {
        return ktu::readDec(parameter);
    }
    return -1;
}


const char *quote(std::string &str, const char* s) {
    char qchar = *s++;
    while (*s) {
        if (*s == qchar) {
            ++s;
            break;
        }
        if (*s == '\\') {
            ++s;
            if (!*s) break;
        }
        str += *s;
        ++s;
    }
    return s;
}

char *macro(char *s) {
    ++s;
    if (*s != '{') {
        return s;
    }
    ++s;
    while (true) {
        switch (*s) {
            case '"':
            case '\'':
                s = quote(s);
                break;
            case '#':
            case '@':
                s = macro(s);
                break;
            case '\\':
                ++s;
                if (!*s) return s;
                ++s;
                break;
            case '}':
                ++s;
            case '\0':
                return s;
            default:
                ++s;
                break;
        }
    }
}

const char *macro(std::string &str, const char *s, std::map <std::string, std::string> &vars) {
    char c = *s;
    std::string key, item;
    std::string *cur = &key;
    ++s;
    if (*s != '{') {
        str += c;
        return s;
    }
    ++s;
    while (true) {
        switch (*s) {
            case '"':
            case '\'':
                s = quote(*cur, s);
                break;
            case '#':
            case '@':
                s = macro(str, s, vars);
                break;
            case '\\':
                ++s;
                if (!*s) goto out;
                *cur += *s++;
                break;
            case ' ':
                ++s;
                break;
            case '=':
                ++s;
                cur = &item;
                break;
            case '}':
                ++s;
            case '\0':
                goto out;
            default:
                *cur += *s++;
                break;
        }
    }
    out:;
    if (item.size())
        vars[key] = item;
    if (c == '@' && key.size() && vars.count(key)) {
        str += vars[key];
    }
    return s;
}



char *splitArgAssignment(char *arg) {
    char *s = arg;
    while (true) {
        switch (*s) {
            case '"':
            case '\'':
                s = quote(s);
                break;
            case '#':
            case '@':
                s = macro(s);
                break;
            case '=':
                *s++ = '\0';
            case '\0':
                return s;
            default:
                ++s;
                break;
        }
    }
}


multiSplitParameter_t multiSplitArgNextParameter(char *arg) {

    char *s = arg, c;
    while (true) {
        switch (c = *s) {
            case '"':
            case '\'':
                s = quote(s);
                break;
            case '#':
            case '@':
                s = macro(s);
                break;
            case ':':
            case ',':
                *s++ = '\0';
            case '\0':
                return {s, c};
            default:
                ++s;
                break;
        }
    }
}

char *splitArgNextParameter(char *arg) {
    char *s = arg;
    while (true) {
        switch (*s) {
            case '"':
            case '\'':
                s = quote(s);
                break;
            case '#':
            case '@':
                s = macro(s);
                break;
            case ',':
                *s++ = '\0';
            case '\0':
                return s;
            default:
                ++s;
                break;
        }
    }
}


char *quote(char *s) {
    char qchar = *s++;
    while (*s) {
        if (*s == qchar) {
            ++s;
            break;
        }
        if (*s == '\\') {
            ++s;
            if (!*s) break;
        }
        ++s;
    }
    return s;

}

std::string parse(const char *s, std::map<std::string, std::string> &vars) {
    std::string str;
    while (true) {
        switch (*s) {
            case '"':
            case '\'':
                s = quote(str, s);
                break;
            case '#':
            case '@':
                s = macro(str, s, vars);
                break;
            case '\0':
                return str;
            default:
                str += *s++;
                break;
        }
    }
    
}


