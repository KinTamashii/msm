#pragma once
#include <cstdint>
#include <compare>
#include <ktu/macros/inline.hpp>
#include <bit>
#include <type_traits>
#include <ktu/memory.hpp>

union code_t {
    constexpr code_t () {}
    constexpr code_t (uint32_t grouptype) : grouptype(grouptype) {}
    constexpr code_t (uint16_t group, uint16_t type) {
        if (std::is_constant_evaluated()) {
            if constexpr (std::endian::native == std::endian::big) {
                grouptype = group << 16 | type;
            } else {
                grouptype = type << 16 | group;
            }
        } else {
            this->group = group;
            this->type = type;
        }
    }
    uint32_t grouptype;
    struct {
        uint16_t group, type;
    };
    KTU_INLINE constexpr bool valid() const {return grouptype;}
    KTU_INLINE constexpr operator uint32_t() const {
        return grouptype;
    }
    KTU_INLINE constexpr std::weak_ordering operator<=>(code_t other) const {
        return grouptype <=> other.grouptype;
    }
};



class maxBounds_t {
    public:
        maxBounds_t(uint16_t width = 0, uint8_t lines = 0) :
            priv(width, lines) {}
        maxBounds_t(ktu::reader &reader) {
            priv.width = reader.read_little_endian<uint16_t>();
            priv.lines = reader.read<uint8_t>();
        }
        KTU_INLINE void write(uint8_t *&ptr) {
            ktu::write_little_endian<uint16_t>(ptr, priv.width);
            *ptr++ = priv.lines;
        }
        KTU_INLINE uint16_t width() {return priv.width;}
        KTU_INLINE uint16_t lines() {return priv.lines;}

        KTU_INLINE void addWidth(uint16_t width) {priv.width += width;}
        KTU_INLINE void setWidth(uint16_t width) {priv.width = width;}
        template <bool assignWidth = true>
        KTU_INLINE void addLine() {
            if constexpr (assignWidth) {
                priv.width = 1;
            }
            ++(priv.lines);
        }
        KTU_INLINE void resetLines() {priv.lines = 0;}

        KTU_INLINE void setMaxWidth(maxBounds_t other) {
            priv.width = maxWidth(*this, other);
        }
        KTU_INLINE void setMaxLines(maxBounds_t other) {
            priv.lines = maxLines(*this, other);
        }
        KTU_INLINE void setMax(maxBounds_t other) {
            *this = maxBounds_t::max(*this, other);
        }
        KTU_INLINE static uint16_t maxWidth(maxBounds_t othis, maxBounds_t other) {
            return std::max(othis.priv.width, other.priv.width);
        }
        KTU_INLINE static uint8_t maxLines(maxBounds_t othis, maxBounds_t other) {
            return std::max(othis.priv.lines, other.priv.lines);
        }
        KTU_INLINE static maxBounds_t max(maxBounds_t othis, maxBounds_t other) {
            return maxBounds_t(
                maxWidth(othis, other),
                maxLines(othis, other)
            );
        }
    private:
        struct priv_t {
            priv_t(uint16_t width = 0, uint8_t lines = 0) :
                width(width), lines(lines) {}
            uint16_t width;
            uint8_t lines;
        } priv;
        
};

#include <fmt/format.h>

template <> class fmt::formatter<code_t> {
public:
    constexpr auto parse (format_parse_context& ctx) { return ctx.begin(); }
    template <typename Context>
    constexpr auto format (code_t obj, Context& ctx) const {
        return fmt::format_to(ctx.out(), obj ? "(group=0x{:04X}, type=0x{:04X})" : "None", obj.group, obj.type);
    }
};