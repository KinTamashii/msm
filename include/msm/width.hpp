#pragma once


#include <map>
#include <ktu/memory.hpp>
#include <fmt/format.h>

#include <msm/titleID.hpp>


class width;

namespace impl {

    struct width_base {
            friend fmt::formatter<width>;
        protected:
            uint16_t base = 100, size = 100;
            const uint8_t *currentWidths = nullptr;
            const uint16_t *endOfRange = nullptr;
            void _set(uint16_t size, const void *data);
        public:
            width_base() {}
            width_base(uint16_t base, uint16_t size, const uint8_t * currentWidths, const uint16_t *endOfRange) :
                base(base), size(size), currentWidths(currentWidths), endOfRange(endOfRange) {}
            inline uint16_t operator[](uint16_t codepoint) const {
                return currentWidths ?
                    (size_t)currentWidths[codepoint]*(size_t)size/(size_t)base :
                    8 * size / 100;
            }
            inline bool is(uint16_t size) {
                return this->size == size;
            }
    };
};





class width : public impl::width_base {
        friend fmt::formatter<width>;
    public:
        class view : public impl::width_base {
            public:
                view(uint16_t base, uint16_t size, const uint8_t * currentWidths, const uint16_t *endOfRange, const void *data) :
                    width_base(base, size, currentWidths, endOfRange), data(data) {}
                inline void set(uint16_t size) {_set(size, data);}
            private:
                const void *data;
        };
        operator view() {
            return view(base, size, currentWidths, endOfRange, data.data());
        }
        width() {}
        width(const char *s, std::map<std::string, std::string> &vars) {
            assign(s, vars);
        }
        void assign(const char *s, std::map<std::string, std::string> &vars);
        inline void set(uint16_t size) {_set(size, data.data());}
        
    private:
        ktu::buffer data;
};


template <> class fmt::formatter<width> {
public:
    constexpr auto parse (format_parse_context& ctx) { return ctx.begin(); }
    template <typename Context>
    constexpr auto format (const width& obj, Context& ctx) const {
        return format_to(ctx.out(), "(base={}, size={}, currentWidths={}, endOfRange={}, data.data()={}, data.size()={})", obj.base, obj.size, (void*)obj.currentWidths, (void*)obj.endOfRange, (void*)obj.data.data(), obj.data.size());
    }
};
