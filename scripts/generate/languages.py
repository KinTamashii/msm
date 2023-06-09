from __init__ import INCLUDE_DIR

from enum import IntEnum



languages = IntEnum(
    'languages',
    [
        'Danish',
        'Dutch',
        'English',
        'Finnish',
        'French',
        'German',
        'Greek',
        'Hungarian',
        'Italian',
        'Norwegian',
        'Romanian',
        'Russian',
        'Spanish',
        'Swedish',
        'Unknown'
    ],
    start=0
)

def main():

    txt = open(f"{INCLUDE_DIR}/languages.hpp", 'w')

    
    txt.write(f"""
#pragma once
#include <fmt/format.h>
enum struct language {{
    {f',{chr(0xA)}    '.join([language.name for language in languages])}
}};

    
template <> class fmt::formatter<language> {{
public:
    constexpr auto parse (format_parse_context& ctx) {{ return ctx.begin(); }}
    template <typename Context>
    constexpr auto format (language obj, Context& ctx) const {{
        const char *language_strings[] {{
            {f",{chr(0xA)}            ".join(['"'+language.name+'"' for language in languages])}
        }};
        return format_to(ctx.out(), "{{}}", language_strings[(size_t)obj]);  // --== KEY LINE ==--
    }}
}};



    """
    )

    txt.close()
