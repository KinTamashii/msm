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
#include <msm/blz77.hpp>

#include <msm/encode/common.hpp>


class encode_t;






class lbl1 {
    public:
        lbl1() {}
        
        inline void setNumberOfGroups(uint32_t numberOfGroups) {
            this->numberOfGroups = numberOfGroups;
        }
        inline void clear() {
            labels.clear();
        }
        
        inline void push(const std::string &str) {
            labels.push_back(str);
        }
        static uint32_t labelChecksum(const std::string &str, uint32_t numberOfGroups) {
            uint32_t group = 0;
            for (uint8_t c : str) {
                group *= 0x492;
                group += c;
            }
            return group % numberOfGroups;
        }
        template <bool big_endian>
        static void addBuffer(lbl1 *pthis, ktu::buffer &buf, uint8_t paddingChar) {
            size_t pos = buf.size(), offsetTableSize = 4 + (pthis->numberOfGroups * 8);
            uint32_t sectionSize = offsetTableSize;
            std::vector<uint32_t> *groups = new std::vector<uint32_t>[pthis->numberOfGroups];
            for (uint32_t index = 0; index < pthis->labels.size(); index++) {
                const std::string &label = pthis->labels[index];
                sectionSize += label.size() + sizeof(uint8_t) + sizeof(uint32_t);
                groups[labelChecksum(label, pthis->numberOfGroups)].push_back(index);
            }
            size_t nonAlignedSize = buf.size()+sectionSize+0x10, newSize = ktu::align2(nonAlignedSize, 4), dif = newSize - nonAlignedSize;
            buf.resize(newSize);
            memset(buf.data() + (buf.size() - (dif)), paddingChar, dif);

            uint8_t *offsetTablePtr = buf.data() + pos; 


            ktu::write(offsetTablePtr, ktu::numeric_literal("LBL1"));
            ktu::write_big_endian<big_endian>(offsetTablePtr, sectionSize);
            offsetTablePtr += 8;
            uint8_t *start = offsetTablePtr, *labelsPtr = offsetTablePtr + offsetTableSize;
            
            ktu::write_big_endian<big_endian>(offsetTablePtr, pthis->numberOfGroups);
            for (auto &group : ktu::range(groups, groups+pthis->numberOfGroups)) {
                ktu::write_big_endian<big_endian, uint32_t>(offsetTablePtr, group.size());
                ktu::write_big_endian<big_endian, uint32_t>(offsetTablePtr, labelsPtr-start);
                for (uint32_t index : group) {
                    const std::string &label = pthis->labels[index];
                    *labelsPtr = label.size(); labelsPtr++;
                    memcpy(labelsPtr, label.data(), label.size());
                    labelsPtr += label.size();
                    ktu::write_big_endian<big_endian>(labelsPtr, index);
                }
            }
                


            delete[] groups;
        }

        
    private:
        std::vector<std::string> labels;
        uint32_t numberOfGroups;
};

struct dataSection : ktu::buffer {
        dataSection(size_t size) : ktu::buffer(size) {}
    static void addBuffer(dataSection *pthis, ktu::buffer &buf, uint8_t paddingChar) {
        
        buf.insert(buf.end(), (uint8_t*)&*pthis->begin(), (uint8_t*)&*pthis->end());
        
        buf.resize(ktu::align2(buf.size(),4), paddingChar);
        
    }
};


class section {
    public:
        section(void *pthis, void *function) : function((void(*)(void*,ktu::buffer&,uint8_t))function), pthis(pthis) {}
        inline void callback(ktu::buffer &buf, uint8_t paddingChar) {
            function(pthis, buf, paddingChar);
        }
        ~section() {}
    private:
        void *pthis;
        void (*function)(void*,ktu::buffer&,uint8_t);
};

struct tagInfo_t {
    struct ptr_pair {
        const char *ptr;
        const char *end;
    };
    std::vector<uint32_t> valueParameters;
    std::vector<ptr_pair> textParameters;
    title::id gameID;
};

struct mutableData {
    mutableData() {}
    mutableData(mutableData &&other) {
        *this = std::move(other);
    }
    void operator=(mutableData &&other) {
        ptr = other.ptr;
        end = other.end;
        valueParameters.swap(other.valueParameters);
        textParameters.swap(other.textParameters);
        codepoint = other.codepoint;
        state = other.state;
    }
    const char *ptr;
    const char *end;
    std::vector<uint32_t> valueParameters;
    std::vector<tagInfo_t::ptr_pair> textParameters;
    uint32_t codepoint;
    uint32_t state = 0;
    enum {
        whitespaceField =       0b000000000000000000000000000'0'1111,
            noWhitespace =      0b000000000000000000000000000'0'0000,
            queueWhitespace =   0b000000000000000000000000000'0'0001,
            whitespace =        0b000000000000000000000000000'0'0010,
            lockWhitespace =    0b000000000000000000000000000'0'0100,
            usePrevWs =         0b000000000000000000000000000'0'1000,
        hasParameters =         0b000000000000000000000000000'1'0000,
    };
};








class encode_t {
    public:
        encode_t(const std::filesystem::path &inputPath, ktu::buffer &outputBuffer, const std::filesystem::path &externalPath, title::id id = title::id::null) : inputPath(inputPath), buf(outputBuffer), externalPath(externalPath), gameID(id) {
            ktu::buffer inputBuffer;
                if (!inputBuffer.assign(inputPath)) return;
            init((const char*)inputBuffer.data(), (const char*)inputBuffer.data()+inputBuffer.size());
        }
        
        ~encode_t() {
            
        }
        const std::filesystem::path &externalPath;
        const std::filesystem::path &inputPath;
        
        inline void init(const char *ptr, const char *end) {
            push(ptr, end);
            
            push_file();
            switch (archive_v) {
                case mode::value::archiveML4:
                    archive_ml4(fileTable, externalPath, ml4.unknown);
                    ext = "dat";
                    break;
                case mode::value::archiveBG4:
                    archive_bg4(buf, fileTable, bg4);
                    ext = "dat";
                    break;
                default:
                    break;
            }
        }
        inline void push_file() {
            
            if (inFile) {
                if (inString) pushString();
                for (auto &section : sections) {
                    section.callback(buf, paddingChar);
                }

                for (auto dataSec : dataSections) {delete dataSec;} dataSections.clear();
                *(uint16_t*)&buf.access(curBegin + 0xE) = ktu::big_endian<uint16_t>(sections.size(), bigEndian);
                *(uint32_t*)&buf.access(curBegin + 0x12) = ktu::big_endian<uint32_t>(buf.size() - curBegin, bigEndian);

                sections.clear();
                textSection.clear();
                
                uint32_t offset = fileTable.back().offset;
                if (archive_v == mode::value::archiveBG4) {
                    bool compressed = offset & 0x80000000;
                    offset &= 0x7FFFFFFF;
                    if (compressed) {
                        ktu::buffer file(buf.data()+offset, buf.data()+buf.size());
                        buf.resize(offset);
                        compress(file.data(), file.size(), buf);
                    }
                }
                
                
                fileTable.back().size = buf.size() - offset;
            }
            
        };
        
        inline const char *getExt() {return ext;};
        
        
    private:
        uint32_t lastWsCodepoint = ' ';
        uint32_t prevWsCodepoint;
        uint8_t paddingChar = 0;
        mutableData mut;
        std::vector<mutableData> states;
        std::vector<section> sections;
        std::vector<dataSection*> dataSections;
        std::map<std::string, uint64_t> variables{
            {"false", 0},
            {"true", 1}
        };
        struct {
            uint32_t unknown = 0;
        } ml4;
        bg4_t bg4;
        
        
        int archive_v = mode::value::file;
        std::vector<fileData_t> fileTable;
        title::id gameID;
        ktu::buffer &buf;
        txt2 textSection;
        lbl1 labelSection;
        bool inString = false;
        bool inFile = false;
        bool bigEndian = false;
        size_t curBegin;
        const char *ext = "msbt";
        
        void push(const char *ptr, const char *end);
        bool controlSequence();
        void selectCommand(const std::string &str);
        void getTextParameter();
        void getValueParameters();
        std::string getVariable();
        int getBinaryValue();
        int getOctalValue();
        int getDecimalValue();
        int getHexValue();

        template <bool big_endian>
        void codeVariadicParamsf(uint16_t group, uint16_t type);
        void (encode_t::*codeVariadicParamsPtr)(uint16_t,uint16_t) = &encode_t::codeVariadicParamsf<false>;
        inline void codeVariadicParams(uint16_t group, uint16_t type) {
            (this->*codeVariadicParamsPtr)(group, type);
        }

        inline void setEndian() {
            uint8_t * ptr = &buf[curBegin + 8];
            if (bigEndian) {
                codeVariadicParamsPtr = &encode_t::codeVariadicParamsf<true>;
                pushCodepointPtr =    &txt2::push<true>;
                pushStringPtr =       &txt2::push_string<true>;
                endCodePtr =          &txt2::endcode<true>;
                genericptr  =         &encode_t::genericf<true>;
                rubyptr =             &encode_t::rubyf<true>;
                colorptr =            &encode_t::colorf<true>;
                pushTsy1ptr =         &encode_t::pushTsy1f<true>;
                pushUnknownSectionptr = &encode_t::pushUnknownSectionf<true>;
                *ptr++ = 0xFE;
                *ptr = 0xFF;
            } else {
                codeVariadicParamsPtr = &encode_t::codeVariadicParamsf<false>;
                pushCodepointPtr =    &txt2::push<false>;
                pushStringPtr =       &txt2::push_string<false>;
                endCodePtr =          &txt2::endcode<false>;
                genericptr  =         &encode_t::genericf<false>;
                rubyptr =             &encode_t::rubyf<false>;
                colorptr =            &encode_t::colorf<false>;
                pushTsy1ptr =         &encode_t::pushTsy1f<false>;
                pushUnknownSectionptr = &encode_t::pushUnknownSectionf<false>;
                *ptr++ = 0xFF;
                *ptr = 0xFE;
            }
        }

        
        
        void (txt2::*pushCodepointPtr)(uint32_t) = &txt2::push<false>;

        void (txt2::*pushStringPtr)() = &txt2::push_string<false>;
        inline void pushString() {
            (textSection.*pushStringPtr)();
        }

        void (txt2::*endCodePtr)(uint16_t,uint16_t) = &txt2::endcode<false>;
        inline void endCode(uint16_t group, uint16_t type) {
            (textSection.*endCodePtr)(group, type);
        }

        template <bool big_endian>
        void genericf();
        void (encode_t::*genericptr)() = &encode_t::genericf<false>;
        inline void generic() {
            (this->*genericptr)();
        }

        template <bool big_endian>
        void rubyf();
        void (encode_t::*rubyptr)() = &encode_t::rubyf<false>;
        inline void ruby() {
            (this->*rubyptr)();
        }
    
        template <bool big_endian>
        void colorf();
        void (encode_t::*colorptr)() = &encode_t::colorf<false>;
        inline void color() {
            (this->*colorptr)();
        }

        template <bool big_endian>
        void pushUnknownSectionf(uint32_t magic) {
            uint32_t sectionSize = (mut.valueParameters.size());

            auto d = new dataSection(sectionSize+0x10);
            dataSections.push_back(d);

            uint8_t *ptr = d->data();
            ktu::write(ptr, magic);
            ktu::write_big_endian<big_endian, uint32_t>(ptr, sectionSize);
            ptr += 8;
            for (uint8_t value : mut.valueParameters) {
                *ptr++ = value;
            }
            
            sections.push_back(section((void*)(d), (void*)&dataSection::addBuffer));
        }
        void (encode_t::*pushUnknownSectionptr)(uint32_t) = &encode_t::pushUnknownSectionf<false>;
        inline void pushUnknownSection(uint32_t magic) {
            (this->*pushUnknownSectionptr)(magic);
        }


        template <bool big_endian>
        void pushTsy1f() {
            uint32_t sectionSize = (mut.valueParameters.size()*4);

            auto d = new dataSection(sectionSize+0x10);
            dataSections.push_back(d);
            
            uint8_t *ptr = d->data();
            ktu::write(ptr, ktu::numeric_literal("TSY1"));
            ktu::write_big_endian<big_endian, uint32_t>(ptr, sectionSize);
            ptr += 8;
            for (uint32_t value : mut.valueParameters) {
                ktu::write_big_endian<big_endian>(ptr,value);
            }
            sections.push_back(section((void*)(d), (void*)&dataSection::addBuffer));
        }
        void (encode_t::*pushTsy1ptr)() = &encode_t::pushTsy1f<false>;
        inline void pushTsy1() {
            (this->*pushTsy1ptr)();
        }


        #include <msm/encode/coded.inl> // All inline member functions.

        
};

#include <msm/encode/coded.tcc>



/* Encode an msm file. */
void encode(const std::filesystem::path &input, const std::filesystem::path &output, const std::filesystem::path &external, title::id id);


