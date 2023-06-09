#pragma once

#include <string>
#include <ktu/memory.hpp>
#include <fmt/format.h>
#include <msm/common.hpp>
#include <stack>

struct ruby_t {
    union {
        struct {
            const uint8_t *left, *right;
        };
        const uint8_t *access[2];
    };
    uint8_t index = 0;
};


template <bool big_endian>
void ruby(std::string &str, ktu::reader &reader, std::stack<ruby_t> &rubies) {
    uint16_t size = (reader.read_big_endian<big_endian, uint16_t>() / 2);
    
    if (!size) str += R"(\code[0,0])";
    if (size < 2) {
        str += fmt::format(R"(\code[0,0,{}])", reader.read_big_endian<big_endian, uint16_t>());
    }
    str += R"(\ruby{)";
    uint16_t
        rbSize = reader.read_big_endian<big_endian, uint16_t>(),
        rtSize = reader.read_big_endian<big_endian, uint16_t>();

    rubies.push({});
    rubies.top().left = reader.cur() + rtSize;
    rubies.top().right = reader.cur() + rtSize + rbSize;
}



template <bool big_endian, bool parametersOpen>
inline void variadicCodeParameters(std::string &str, const char * noParametersPostFix, uint16_t size, ktu::reader &reader) {
    if (!size) {
        if constexpr (parametersOpen) str += ']';
        else str += noParametersPostFix;
        return;
    }
    if constexpr (parametersOpen) str += ',';
    else str += '[';
    for (unsigned i = 0; i < size - 1; i++) {
        str += fmt::format("{},", reader.read_big_endian<big_endian, uint16_t>());
    }
    str += fmt::format("{}]", reader.read_big_endian<big_endian, uint16_t>());
}


template <bool big_endian>
void generic(std::string &str, code_t code, uint16_t size, ktu::reader &reader) {
    str += fmt::format(R"(\code[{},{})", code.group, code.type);
    variadicCodeParameters<big_endian, true>(str, " ", size, reader);
}





template <bool big_endian>
void size(std::string &str, ktu::reader &reader) {
    uint16_t size = (reader.read_big_endian<big_endian, uint16_t>() / 2);
    str += R"(\size)";
    variadicCodeParameters<big_endian, false>(str, " ", size, reader);
    
}

template <bool big_endian>
void color(std::string &str, ktu::reader &reader) {
    uint16_t size = (reader.read_big_endian<big_endian, uint16_t>() / 2);
    if (size != 2) {
        generic<big_endian>(str, {0,3}, size, reader);
        return;
    }
    
    str += fmt::format(R"(\color[{},{},{},{}])",
        reader.read_big_endian<big_endian, uint8_t>(),
        reader.read_big_endian<big_endian, uint8_t>(),
        reader.read_big_endian<big_endian, uint8_t>(),
        reader.read_big_endian<big_endian, uint8_t>()
    );
}


template <bool big_endian>
void pagebreak(std::string &str, ktu::reader &reader, bool endspace) {
    uint16_t size = (reader.read_big_endian<big_endian, uint16_t>() / 2);
    str += R"(\pagebreak)";
    variadicCodeParameters<big_endian, false>(str, (endspace) ? " " : "\n", size, reader);
}