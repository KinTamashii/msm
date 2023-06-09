
#pragma once
#include <fmt/format.h>
enum struct language {
    Danish,
    Dutch,
    English,
    Finnish,
    French,
    German,
    Greek,
    Hungarian,
    Italian,
    Norwegian,
    Romanian,
    Russian,
    Spanish,
    Swedish,
    Unknown
};

    
template <> class fmt::formatter<language> {
public:
    constexpr auto parse (format_parse_context& ctx) { return ctx.begin(); }
    template <typename Context>
    constexpr auto format (language obj, Context& ctx) const {
        const char *language_strings[] {
            "Danish",
            "Dutch",
            "English",
            "Finnish",
            "French",
            "German",
            "Greek",
            "Hungarian",
            "Italian",
            "Norwegian",
            "Romanian",
            "Russian",
            "Spanish",
            "Swedish",
            "Unknown"
        };
        return format_to(ctx.out(), "{}", language_strings[(size_t)obj]);  // --== KEY LINE ==--
    }
};



    