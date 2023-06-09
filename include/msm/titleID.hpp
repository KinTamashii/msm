#pragma once
#include <ktu/string.hpp>
#include <ktu/algorithm.hpp>
#include <msm/decode/codes.hpp>
#include <msm/common.hpp>
#include <map>
struct title {
    enum id : uint64_t {
        null = 0,
        ML4_US = 0x00040000000D5A00,
        ML4_EU = 0x00040000000D9000,
        ML4_JP = 0x0004000000060600,
        ML5_US = 0x0004000000132700,
        ML5_EU = 0x0004000000132800,
        ML5_JP = 0x0004000000132600,
        ML1_REMAKE_US = 0x00040000001B8F00,
        ML1_REMAKE_EU = 0x00040000001B9000,
        ML1_REMAKE_JP = 0x0004000000194B00,
        ML3_REMAKE_US = 0x00040000001D1400,
        ML3_REMAKE_EU = 0x0004000E001D1500,
        ML3_REMAKE_JP = 0x00040000001CA900,
        ML3_REMAKE_JP_VER_1_2 = 0x0004000E001CA900
    };
    struct is {
        static inline constexpr bool ML4 (title::id id) {
            return id == ML4_US || id == ML4_EU || id == ML4_JP;
        };
        static inline constexpr bool ML5 (title::id id) {
            return id == ML5_US || id == ML5_EU || id == ML5_JP;
        };
        static inline constexpr bool ML1_REMAKE (title::id id) {
            return id == ML1_REMAKE_US || id == ML1_REMAKE_EU || id == ML1_REMAKE_JP;
        };
        static inline constexpr bool ML3_REMAKE (title::id id) {
            return id == ML3_REMAKE_US || id == ML3_REMAKE_EU || id == ML3_REMAKE_JP || id == ML3_REMAKE_JP_VER_1_2;
        };
    };
    static constexpr id get (const char* str) {
        char c = *str;
        if (('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f'))
            return (title::id)ktu::ston64(str);
        
        switch (ktu::hash(str)) {
            case ktu::hash("ML4"):
            case ktu::hash("ML4_US"):
                return id::ML4_US;
            case ktu::hash("ML4_EU"):
                return id::ML4_EU;
            case ktu::hash("ML4_JP"):
                return id::ML4_JP;
            case ktu::hash("ML5"):
            case ktu::hash("ML5_US"):
                return id::ML5_US;
            case ktu::hash("ML5_EU"):
                return id::ML5_EU;
            case ktu::hash("ML5_JP"):
                return id::ML5_JP;
            case ktu::hash("ML1_REMAKE"):
            case ktu::hash("ML1_REMAKE_US"):
                return id::ML1_REMAKE_US;
            case ktu::hash("ML1_REMAKE_EU"):
                return id::ML1_REMAKE_EU;
            case ktu::hash("ML1_REMAKE_JP"):
                return id::ML1_REMAKE_JP;
            case ktu::hash("ML3_REMAKE"):
            case ktu::hash("ML3_REMAKE_US"):
                return id::ML3_REMAKE_US;
            case ktu::hash("ML3_REMAKE_EU"):
                return id::ML3_REMAKE_EU;
            case ktu::hash("ML3_REMAKE_JP"):
                return id::ML3_REMAKE_JP;
            case ktu::hash("ML3_REMAKE_JP_VER_1_2"):
                return id::ML3_REMAKE_JP_VER_1_2;
            default:
                return id::null;
        };
    };
    static inline constexpr bool valid (id id) {
        switch (id) {
            case id::ML4_US:
            case id::ML4_EU:
            case id::ML4_JP:
            case id::ML5_US:
            case id::ML5_EU:
            case id::ML5_JP:
            case id::ML1_REMAKE_US:
            case id::ML1_REMAKE_EU:
            case id::ML1_REMAKE_JP:
            case id::ML3_REMAKE_US:
            case id::ML3_REMAKE_EU:
            case id::ML3_REMAKE_JP:
            case id::ML3_REMAKE_JP_VER_1_2:
                return true;
            default:
                return false;
        };
    };
    private:
        class codeOverride_t {
            private:
                struct indecies {enum {hset, hspace, option, wait};};
            public:
                void set(char *codeName, char *postAssign);
                KTU_INLINE code_t hset()    const {return codes[indecies::hset];}
                KTU_INLINE code_t hspace()  const {return codes[indecies::hspace];}
                KTU_INLINE code_t option()  const {return codes[indecies::option];}
                KTU_INLINE code_t wait()    const {return codes[indecies::wait];}
                KTU_INLINE const char *operator[](code_t code) const {
                    auto it = codeOverrideMap.find(code);
                    if (it == codeOverrideMap.end())
                        return nullptr;
                    return it->second;
                }
            private:
                std::map<code_t, const char*> codeOverrideMap;
                code_t codes[4] {{0,0},{0,0},{0,0},{0,0}};
        };
    public:
        static codeOverride_t codeOverride;
    static constexpr id find(const char *path) {
        char c;
        bool dec, high;
        while ((c = *path)) {
            if ((dec = ('0' <= c && c <= '9')) || (high = ('A' <= c && c <= 'F')) || ('a' <= c && c <= 'f')) {
                uint64_t value = 0;
                do {
                    value <<= 4;
                    value |= c - ((dec) ? '0' : (high) ? 0x37 : 0x57);
                    
                    c = *++path;
                } while ((dec = ('0' <= c && c <= '9')) || (high = ('A' <= c && c <= 'F')) || ('a' <= c && c <= 'f'));
                
                if (valid((id)value)) return (id)value;
            }
            ++path;
            while (*path && *path++ != std::filesystem::path::preferred_separator);
        }
        return id::null;
    }
    template <bool big_endian>
    static bool select (id id, code_t code, std::string& str, ktu::reader& reader, std::stack<ruby_t>& rubies) {
        const char *identifier = codeOverride[code];
        if (!identifier) {
            switch (id) {
                case id::ML4_US:
                case id::ML4_EU:
                case id::ML4_JP:
                    switch (code) {
                        case code_t(3, 1):
                            identifier = "wait";
                            break;
                        case code_t(5, 1):
                            identifier = "option";
                            break;
                    };
                    break;
                case id::ML5_US:
                case id::ML5_EU:
                case id::ML5_JP:
                case id::ML1_REMAKE_US:
                case id::ML1_REMAKE_EU:
                case id::ML1_REMAKE_JP:
                case id::ML3_REMAKE_US:
                case id::ML3_REMAKE_EU:
                case id::ML3_REMAKE_JP:
                case id::ML3_REMAKE_JP_VER_1_2:
                    switch (code) {
                        case code_t(2, 1):
                            identifier = "wait";
                            break;
                        case code_t(4, 0):
                        case code_t(4, 1):
                            identifier = "option";
                            break;
                        case code_t(6, 0):
                            identifier = "hspace";
                            break;
                        case code_t(6, 1):
                            identifier = "hset";
                            break;
                    };
                    break;
                default:
                    break;
            };
        }
        if (!identifier) return false;
        
        uint16_t size = reader.read_big_endian<big_endian, uint16_t>() / 2;
        str += '\\';
        str += identifier;
        variadicCodeParameters<big_endian, false>(str, " ", size, reader);
        return true;
    };
    static bool option (id id, code_t code);
    static bool hspace (id id, code_t code);
    static bool hset (id id, code_t code);
    static code_t getWaitGroupType (id id);
    
};
