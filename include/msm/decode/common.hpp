#pragma once
#include <msm/common.hpp>
#include <msm/decode/clean.hpp>
namespace impl {
    template <bool big_endian, typename ...Args>
    auto readTXT2(Args... args) {
        struct signature {
            static constexpr bool clean() {return !syntax();}
            static constexpr bool syntax() {
                return ktu::variadic::types<Args...>::template is_same<const std::vector<std::string>&, 1>::value;
            }
        };

        std::string &text(ktu::variadic::get<0>(args...));
        const auto &labels(ktu::variadic::get<1>(args...));
        auto &binary(ktu::variadic::get<1>(args...));
        ktu::reader &reader(ktu::variadic::get<2>(args...));
        const uint8_t *sectionEnd = (reader.cur() - 4) + ktu::align2(reader.read_big_endian<big_endian, uint32_t>() + 0x10, 4);
        const title::id& id(ktu::variadic::get<3>(args...));
        typename ktu::discard_if<signature::syntax(), width::view&>::type width(ktu::variadic::get<4>(args...));
        typename ktu::discard_if<signature::syntax(), maxBounds_t>::type maxBounds(0,0);
        
        reader.seek(reader.cur()+8);
        const uint8_t *cur = reader.cur();


        if constexpr (signature::syntax()) {
            
            if (labels.size()) {
                int i = 0;
                for (
                    const uint8_t *stringsTableEnd = reader.read_big_endian<big_endian, uint32_t>()*sizeof(uint32_t) + reader.cur();
                    reader.cur() != stringsTableEnd;
                ) {
                    text += fmt::format("\n" R"(\string{{{}}})" "\n", (i < labels.size()) ? labels[i].c_str() : ""); i++;
                    ::decodeString<big_endian>(text, ktu::reader((const uint16_t*)(cur + reader.read_big_endian<big_endian, uint32_t>()), (const uint16_t*)(sectionEnd)), id);
                    text += '\n';
                }
                goto pastElse;
            }
        } /*else*/ {
            for (
                const uint8_t *stringsTableEnd = reader.read_big_endian<big_endian, uint32_t>()*sizeof(uint32_t) + reader.cur();
                reader.cur() != stringsTableEnd;
            ) {
                if constexpr (signature::syntax()) {
                    text += "\n" R"(\string)" "\n";
                    ::decodeString<big_endian>(text, ktu::reader((const uint16_t*)(cur + reader.read_big_endian<big_endian, uint32_t>()), (const uint16_t*)(sectionEnd)), id);
                    text += '\n';
                } else if constexpr (signature::clean()) {
                    maxBounds.setMax(
                        ::decodeString<big_endian>(
                            text,
                            binary,
                            ktu::reader(
                                (const uint16_t*)(cur + reader.read_big_endian<big_endian, uint32_t>()),
                                (const uint16_t*)(sectionEnd)
                            ),
                            id,
                            width
                        )
                    );
                    
                }
                
            }
        }
            

        pastElse:
        reader.seek(sectionEnd);

        if constexpr (signature::clean()) {
            return maxBounds;
        }
    }
};