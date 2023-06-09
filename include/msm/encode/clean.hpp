#pragma once
#include <string>
#include <ktu/unicode.hpp>
#include <ktu/memory.hpp>
#include <iostream>
#include <map>
#include <array>
#include <stack>
#include <vector>
#include <ktu/algorithm.hpp>
#include <fmt/format.h>

#include <msm/encode/common.hpp>
#include <msm/width.hpp>
#include <msm/languages.hpp>

#include <msm/argparse.hpp>

struct encodeCleanFlags_t;
struct encodeCleanFlags_t {
    constexpr encodeCleanFlags_t(
        bool autopage = false,
        bool limitlines = true,
        bool maxlines = false,
        bool maxwidth = false
    ) :
        autopage(autopage), limitlines(limitlines), maxlines(maxlines), maxwidth(maxwidth) {}

    bool autopage, limitlines, maxlines, maxwidth;
    void assign(const char *s, std::map<std::string, std::string> &vars);
};

template <> class fmt::formatter<encodeCleanFlags_t> {
public:
    constexpr auto parse (format_parse_context& ctx) { return ctx.begin(); }
    template <typename Context>
    constexpr auto format (encodeCleanFlags_t obj, Context& ctx) const {
        return fmt::format_to(ctx.out(), "(autopage={}, limitlines={}, maxlines={}, maxwidth={})", obj.autopage, obj.limitlines, obj.maxlines, obj.maxwidth);
    }
};


/* Encode a txt file with a msf format file (and optional cwdh file). */
void encodeClean(
    const std::filesystem::path &input,
    const std::filesystem::path &output,
    const std::filesystem::path &format,
    const std::filesystem::path &external,
    title::id id,
    width::view width,
    language language,
    encodeCleanFlags_t flags
);

