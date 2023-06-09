#pragma once
#include <msm/common.hpp>
#include <ktu/memory.hpp>
#include <msm/titleID.hpp>
class txt2 {
    public:
        txt2() {}
        template <bool big_endian>
        inline void push(uint32_t codepoint) {
            ktu::u16::push_back<big_endian>(stringData, codepoint);
        }
        template <bool big_endian>
        inline void push_short(uint16_t codepoint) {
            stringData.push_back_big_endian<big_endian, uint16_t>(codepoint);
        }
        template <size_t N, typename T>
        inline void push_array(T(&ptr)[N]) {
            stringData.push_back(ptr, N*sizeof(T));
        }
        inline void insert(auto *pos, auto *first, auto *last) {
            stringData.insert(pos, first, last);
        }
        inline void insert(auto *pos, auto item) {
            stringData.insert(pos, item);
        }

        size_t size8() {
            return stringData.size();
        }
        inline const uint8_t *begp() {
            return stringData.data();
        }
        inline const uint8_t *endp() {
            return stringData.data()+stringData.size();
        }
        template <bool big_endian>
        inline void push_string() {
            
            stringData.push_back_big_endian<big_endian, uint16_t>(0);
            header.push_back(lastSize);
            lastSize = stringData.size();
        }
        template <bool big_endian>
        static void addBuffer(txt2 *pthis, ktu::buffer &buf, uint8_t paddingChar) {
            size_t pos = buf.size<uint32_t>() + 0x1;
            uint32_t add = pthis->header.size() - 0x10, numStrings = (add - 0x4) >> 2;

            buf.insert(buf.end(), pthis->header.begin(), pthis->header.end());
            buf.insert(buf.end(), pthis->stringData.begin(), pthis->stringData.end());

            auto ptr = buf.data<uint32_t>() + pos;
            *ptr = ktu::big_endian<big_endian, uint32_t>(pthis->header.size() + pthis->stringData.size() - 0x10);
            ptr += 3;
            *ptr++ = ktu::big_endian<big_endian>(numStrings);
            
            
            for (auto end = ptr + numStrings; ptr != end; ptr++) {
                *ptr = ktu::big_endian<big_endian>(*ptr + add);
            }
            

            buf.resize(ktu::align2(buf.size(), 4), paddingChar);
        }
        inline size_t bytesize() {return stringData.size();};
        inline size_t size() {return stringData.size<uint16_t>();}
        inline void resize(size_t size){stringData.resize<uint16_t>(size);}
        uint16_t *offset(size_t index){return &stringData.operator[]<uint16_t>(index);}
        template <bool big_endian>
        void codeVariadicParams(std::vector<uint32_t> &valueParameters, uint16_t group, uint16_t type);
        template <bool big_endian>
        inline void code(uint16_t group, uint16_t type) {
            stringData.push_back<uint16_t>(ktu::big_endian<big_endian>((uint16_t)0xE));
            stringData.push_back(ktu::big_endian<big_endian>(group));
            stringData.push_back(ktu::big_endian<big_endian>(type));
        }
        template <bool big_endian>
        void endcode(uint16_t group, uint16_t type) {
            stringData.push_back<uint16_t>(ktu::big_endian<big_endian>((uint16_t)0xF));
            stringData.push_back(ktu::big_endian<big_endian>(group));
            stringData.push_back(ktu::big_endian<big_endian>(type));
        }
        inline void clear() {
            stringData.clear();
            header.resize(0x14);
            lastSize = 0;
        }
    private:
        ktu::buffer header{(uint8_t)0x54, (uint8_t)0x58, (uint8_t)0x54, (uint8_t)0x32, (uint8_t)0x00, (uint8_t)0x01, (uint8_t)0x8A, (uint8_t)0xB0, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00};
        ktu::buffer stringData;
        uint32_t lastSize = 0;
};

struct fileData_t {
    uint32_t offset;
    uint32_t size;
};

void archive_ml4(const std::vector<fileData_t> &fileTable, const std::filesystem::path &external, uint32_t ml4unknown);


struct bg4_t {
    uint16_t version = 0x105;
    uint16_t fileEntryCountMultiplier = 0x0;
    std::vector<std::string> fileNames;
    size_t headerSize = 0x10;
};
void archive_bg4(ktu::buffer &buf, const std::vector<fileData_t> &fileTable, const bg4_t &bg4);

inline bool isWhitespaceNoNewline(uint32_t codepoint) {
    return (codepoint == '\r') || (codepoint == '\t') || (codepoint == ' ') || (codepoint == 0x3000);
}

inline bool isWhitespace(uint32_t codepoint) {
    return (codepoint == '\r') || (codepoint == '\t') || (codepoint == '\n') || (codepoint == ' ') || (codepoint == 0x3000);
}

#include <msm/mode.hpp>