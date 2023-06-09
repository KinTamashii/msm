#pragma once
#include <cstdint>
#include <cstddef>
#include <msm/titleID.hpp>

namespace code_data_n {
    constexpr size_t size = 63;
    static uint8_t data[size*4] {0x0E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x0E, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x0E, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00};
    constexpr struct access_t {
        uint8_t offset : 6; // 00'10_00'00
        uint8_t size : 4;
        friend inline bool operator==(access_t self, access_t other) {
            return *(uint16_t*)&self == *(uint16_t*)&other;
        };
    } page{0, 4}, page_wait{4, 8}, ruby{12, 6}, page_ellipses{18, 10}, page_wait_ellipses{28, 14}, option_override{42, 4}, newline{46, 1};
    inline bool setWaitCode(title::id id) {
        code_t result = title::getWaitGroupType(id);
        if (!result) return false;
        uint16_t *dataOffset = (uint16_t*)((void*)&data[0]);
        uint16_t
            groupLittleEndian = ktu::little_endian(result.group),
            typeLittleEndian = ktu::little_endian(result.type),
            groupBigEndian = ktu::big_endian(result.group),
            typeBigEndian = ktu::big_endian(result.type);
        dataOffset[5] = groupLittleEndian;
        dataOffset[6] = typeLittleEndian;
        dataOffset[32] = groupLittleEndian;
        dataOffset[33] = typeLittleEndian;
        dataOffset[size+5] = groupBigEndian;
        dataOffset[size+6] = typeBigEndian;
        dataOffset[size+32] = groupBigEndian;
        dataOffset[size+33] = typeBigEndian;
        return true;
    };
    inline void setOptionOverrideCode(code_t code) {
        uint16_t *dataOffset = (uint16_t*)((void*)&data[0]);
        dataOffset[43] = ktu::little_endian(code.group);
        dataOffset[44] = ktu::little_endian(code.type);
        dataOffset[size+43] = ktu::big_endian(code.group);
        dataOffset[size+44] = ktu::big_endian(code.type);
    };
    const uint8_t *begin(bool bigEndian, access_t access) {
        return &data[((size_t)(access.offset) + bigEndian * size) * 2];
    };
    const uint8_t *end(bool bigEndian, access_t access) {
        return &data[(((size_t)access.offset+(size_t)access.size) + bigEndian * size) * 2];
    };
    inline void push(txt2 &textSection, bool bigEndian, access_t access) {
        textSection.insert(textSection.endp(), begin(bigEndian, access), end(bigEndian, access));
    };
    inline auto option (bool bigEndian, title::id id) {
        access_t access {0, 0};
        if (title::codeOverride.option()) {
            access = option_override;
        } else {
            switch (id) {
                case title::id::ML4_US:
                case title::id::ML4_EU:
                case title::id::ML4_JP:
                    access = {47, 4};
                    break;
                case title::id::ML5_US:
                case title::id::ML5_EU:
                case title::id::ML5_JP:
                    access = {51, 4};
                    break;
                case title::id::ML1_REMAKE_US:
                case title::id::ML1_REMAKE_EU:
                case title::id::ML1_REMAKE_JP:
                    access = {55, 4};
                    break;
                case title::id::ML3_REMAKE_US:
                case title::id::ML3_REMAKE_EU:
                case title::id::ML3_REMAKE_JP:
                case title::id::ML3_REMAKE_JP_VER_1_2:
                    access = {59, 4};
                    break;
                default:
                    break;
            };
        }
        struct {
            const uint8_t *data;
            uint16_t size;
        } return_value {
            .data = (const uint8_t *)begin(bigEndian, access)
        };
        return_value.size = (const uint8_t *)end(bigEndian, access) - return_value.data;
        return return_value;
    };
};