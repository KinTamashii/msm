#include <msm/encode/clean.hpp>

#include <msm/terminator.hpp>
#include <msm/abbreviations.hpp>

#include <msm/width.hpp>




#include <ktu/vector.hpp>
#include <msm/common.hpp>




inline bool isPunc(uint32_t codepoint) {
    return codepoint == '.' || codepoint == '!' || codepoint == '?' || codepoint == ';';
}





struct local_ruby_t {
    const uint8_t *position;
    enum type_e {
        begin,
        mid,
        end
    } type;
};


#include <msm/encode/codeData.hpp>


class view {
        
    public:
    
        view(const uint8_t *start, title::id id, width::view &width, language language, encodeCleanFlags_t flags, bool waitCodePresent) :
            first(start),
            last(start), id(id), width(width), language(language), flags(flags), waitCodePresent(waitCodePresent) {}

        
        inline void assign(uint16_t maxWidth, uint8_t maxLines) {
            this->maxWidth = maxWidth;
            this->maxLines = maxLines;
            textSection.clear();
        }
        title::id id;
        width::view &width;
        language language;
        bool waitCodePresent;
        
        void bounds(bool isWs) {
            
            if (curWidth > maxWidth) {
                curLines++;
                bool lineLimit = (curLines >= maxLines) && flags.limitlines;
                curLines = (lineLimit) ? 0 : curLines;

                if (lineLimit && lastPunc) {
                    curWidth = 0;
                    puncWidth = 0;
                    lastHSet = 0;
                    
                    last = lastPunc;
                    
                    curSize = puncSize + ((!isWhitespace(ktu::u8::read(&last))) && (lastPunc <= lineStart || lastPunc[-1] != '\n'));
                    
                    
                    auto it = endlines.rbegin();
                    for (; it != endlines.rend() && ((it->position >= lastPunc)); it++);
                    
                        

                    endlines.resize(endlines.rend()-it+1);
                    endlines.back() = endline {
                        .position = lastPunc,
                        .data = (waitCodePresent) ? code_data_n::page_wait : code_data_n::page,
                        .remove = true
                        
                    };
                    lastPunc = nullptr;
                    
                } else {
                    
                    if (lastWs) {
                        if (endlines.empty() || (endlines.size() && lastWs != endlines.back().position)) {
                            
                            curWidth -= wsWidth;
                            wsWidth = curWidth;
                            lastHSet = 0;
                            
                            endlines.push_back(
                                endline {
                                    .position = lastWs,
                                    .data = (lineLimit) ?
                                            (waitCodePresent) ? code_data_n::page_wait_ellipses : code_data_n::page_ellipses
                                        :
                                            code_data_n::newline,
                                    .remove = true 
                                }
                            );
                        }
                    } else {
                        if (endlines.empty() || (endlines.size() && last != endlines.back().position)) {
                            curWidth = 0;
                            endlines.push_back(
                                endline {
                                    .position = last,
                                    .data = (lineLimit) ?
                                            (waitCodePresent) ? code_data_n::page_wait_ellipses : code_data_n::page_ellipses
                                        :
                                            code_data_n::newline,
                                    .remove = isWs
                                }
                            );
                        }
                    }
                }
            }
        }

        

        const uint8_t *findCodePointer() {
            const uint8_t *lastWsOrPunc = std::max(lastWs, lastPunc);
            uint32_t tcodepoint;
            const uint8_t
                *leftPointer = lastWsOrPunc ? lastWsOrPunc : last,
                *rightPointer = last,
                *readPointer = rightPointer;

            while ((tcodepoint = ktu::u8::read(&readPointer)) && !isWhitespace(tcodepoint) && !isPunc(tcodepoint))
                rightPointer = readPointer;

            return ((last-leftPointer) <= (rightPointer-last)) ? leftPointer : rightPointer;
        }

        template <bool big_endian>
        void codeModifier(const uint8_t *data, size_t size, uint32_t codepoint, const uint8_t *codePointer) {
            
            if (size > 9 && ktu::big_endian<big_endian, uint16_t>(*(uint16_t*)(data+0x2)) == 0x0000 && ktu::big_endian<big_endian, uint16_t>(*(uint16_t*)(data+0x4)) == 0x0002) {
                
                
                

                uint16_t newSize = ktu::big_endian<big_endian, uint16_t>(*(uint16_t*)(data+0x8));
                
                if (width.is(newSize)) return;
                
                if (codePointer == last) {
                    curWidth -= width[codepoint];
                }

                width.set(newSize);

                if (codePointer == lastWs) {
                    curWidth = wsWidth;
                    for (auto c_it = codePointer; c_it != last;) {
                        curWidth += width[ktu::u8::read(&c_it)];
                    }
                } else if (codePointer == lastPunc) {
                    curWidth = puncWidth; 
                    for (auto c_it = codePointer; c_it != last;) {
                        curWidth += width[ktu::u8::read(&c_it)];
                    }
                } else if (codePointer == last) {
                    curWidth += width[codepoint];
                }
                
            }
                
        }


        template <bool big_endian>
        void readline() {
            
            uint32_t codepoint, lastCodepoint;
            const uint8_t *lastPointer;
            bool isWs;
            lineStart = last;
            puncSize = 0;
            uint32_t maxSize = 0;
            curSize = 0;
            auto code_iterator = codes.begin() + lastCodeIndex;
            const uint8_t *temp = last;
            const uint8_t *alt = last;
            

            while ((codepoint = ktu::u8::read(&temp)) && codepoint != '\n') {
                if (!isWhitespaceNoNewline(codepoint)) {
                    maxSize++;
                }
            }
            
            while ((codepoint = ktu::u8::read(&last))) {
                curWidth += width[codepoint];
                if ((isWs = isWhitespace(codepoint))) {
                    switch (lastCodepoint) {
                        case '.':
                            if (abbreviations::match(first, lastPointer, language))
                                break;
                                
                        case '!':
                        case '?':
                        case ';':
                            lastPunc = last;
                            puncWidth = curWidth;
                            puncSize = curSize;
                            break;
                    }
                    lastWs = last;
                    wsWidth = curWidth;
                    
                } else {
                    curSize++;
                }

                const uint8_t *codePointer = findCodePointer();
                
                while (code_iterator != codes.end() && code_iterator->position.ratio*maxSize < curSize) {
                    
                    codeModifier<big_endian>(code_iterator->data, code_iterator->size, codepoint, codePointer);
                    code_iterator->position.real = codePointer;
                    code_iterator++;
                }

                if (codepoint == '\n') {
                    if (last > lineStart)
                        break;
                    curSize = 0;
                }

                
                bounds(isWs);
                
                
                    
                lastCodepoint = codepoint;
                lastPointer = last;
            }
            auto it = last-2;
            for (; it >= first; it--) {
                if (*it == '\n') {
                    it++;
                    break;
                }
            }
            while (code_iterator != codes.end()) {
                codeModifier<big_endian>(code_iterator->data, code_iterator->size, codepoint, last);
                code_iterator->position.real = last;
                code_iterator++;
            }
            lastCodeIndex = codes.end()-codes.begin();
            
        }


        bool ruby_iterate(
            std::vector<local_ruby_t>::iterator &ruby_iterator,
            std::vector<local_ruby_t>::iterator ruby_end,
            const uint8_t *ptr,
            bool bigEndian
        ) {
            if (ruby_iterator == ruby_end || ruby_iterator->position != ptr) return false;
            bool empty = rubyOffsets.empty();
            size_t offset;
            if (empty) {
                if (ruby_iterator->type != local_ruby_t::begin) {
                    ++ruby_iterator;
                    return true;
                }
            } else {
                offset = rubyOffsets.top();
            }
            size_t diff = textSection.size8() - offset - 6;
            auto data = (uint16_t*)(textSection.begp() + offset);
            
            switch (ruby_iterator->type) {
                case local_ruby_t::begin:
                    rubyOffsets.push(textSection.size8()+6);
                    code_data_n::push(textSection, bigEndian, code_data_n::ruby);
                    break;
                case local_ruby_t::mid:
                    data[2] = diff;
                    break;
                case local_ruby_t::end:
                    rubyOffsets.pop();
                    data[1] = diff - data[2];
                    data[0] = diff;
                    break;
            }
            ++ruby_iterator;
            return true;
        }

        template <bool big_endian>
        void push_string() {

            auto ruby_iterator = rubyOrder.begin(), ruby_end = rubyOrder.end();
            auto code_iterator = codes.begin(), code_end = codes.end();
            auto endline_iterator = endlines.begin(), endline_end = endlines.end();
            auto ptr = first;

            uint16_t leftSize;

            ruby_iterate(ruby_iterator, ruby_end, ptr, big_endian);
            uint32_t codepoint = ktu::u8::read(&ptr);
            for (; ptr < last; codepoint = ktu::u8::read(&ptr)) {
                

                if (!isWhitespace(codepoint)) {
                    curSize++;
                } else if (codepoint == '\n') {
                    curSize = 0;
                }
                
                
                while (code_iterator != code_end && ptr == code_iterator->position.real) {
                    textSection.insert(textSection.endp(), code_iterator->data, code_iterator->data+code_iterator->size);
                    code_iterator++;
                }
                if (ruby_iterate(ruby_iterator, ruby_end, ptr, big_endian)) continue;

                if (endline_iterator != endline_end && ptr == endline_iterator->position) {
                    code_data_n::push(textSection, big_endian, endline_iterator->data);
                    endline_iterator++;
                    if (endline_iterator->remove) continue;
                }
                textSection.push<big_endian>((codepoint == '\n') ? ' ' : codepoint);
            }
            
            while (ruby_iterator != ruby_end) {
                if (ruby_iterate(ruby_iterator, ruby_end, ptr, big_endian)) continue;
                ++ruby_iterator;
            }
            while (code_iterator != code_end) {
                textSection.insert(textSection.endp(), code_iterator->data, code_iterator->data+code_iterator->size);
                code_iterator++;
            }

            
            codes.clear();
            rubyOrder.clear();
            textSection.push_string<big_endian>();
            lastCodeIndex = 0;
        }

        void reset(uint16_t maxWidth, uint8_t maxLines) {
            
            
            

            first = last;

            curWidth = 0;
            curLines = 0;
            lastHSet = 0;
            
            this->maxWidth = maxWidth;
            this->maxLines = maxLines;

            lastPunc = nullptr;
            lastWs = nullptr;

            endlines.clear();
        }


        
        
        inline void ruby(local_ruby_t::type_e type) {
            rubyOrder.push_back({
                .position = last,
                .type = type
            });
        }

        
        inline void option(bool big_endian) {
            auto r = code_data_n::option(big_endian, id);
            if (!r.size) return;
            codes.push_back(code(r.data, r.size, last));
        }

        template <bool big_endian>
        inline void check_horizontal_modifiers(const void *data, uint16_t size) {
            if (!(size == 10 && ktu::big_endian<big_endian>(((const uint16_t*)data)[3]) == 2))
                return;
            uint16_t
                group = ktu::big_endian<big_endian>(((const uint16_t*)data)[1]),
                type = ktu::big_endian<big_endian>(((const uint16_t*)data)[2]),
                value = ktu::big_endian<big_endian>(((const uint16_t*)data)[3]);
            if (title::hspace(id, {group, type})) {
                curWidth += value;
                return;
            }
            
            
            if (title::hset(id, {group, type})) {
                curWidth = value;
                if (value <= lastHSet)
                    endlines.push_back(
                        endline {
                            .position = last,
                            .data = code_data_n::newline,
                            .remove = false
                        }
                    );
                lastHSet = value;
            }
        }

        template <bool big_endian>
        inline void push_code(double position, const uint8_t *data, uint16_t size) {
            check_horizontal_modifiers<big_endian>(data, size);
            
            codes.push_back(code(data, size, position));
        }
        

        inline void pagebreak() {
            
            endlines.push_back(
                endline {
                    .position = last,
                    .data = code_data_n::page,
                    .remove = true
                }
            );
            curWidth = 0;
            curLines = 0;
            lastHSet = 0;
            lastPunc= nullptr;
            lastWs = nullptr;
        };
        
        inline void addBuffer(ktu::buffer &outBuf, uint8_t paddingChar, bool bigEndian) {
            (bigEndian ? txt2::addBuffer<true> : txt2::addBuffer<false>)(&textSection, outBuf, paddingChar);
        }

        friend std::ostream& operator<<(std::ostream&os, const view &obj) {
            auto endline_iterator = obj.endlines.begin(), endline_end = obj.endlines.end();
            os << "\\string{?}\n";
            for (auto it = obj.first; it != obj.last; it++) {
                if (endline_iterator != endline_end && it == endline_iterator->position) {
                    if (endline_iterator->data == code_data_n::page) {
                        os << "\\pagebreak";
                    } else {
                        os << "\\\\";
                    }
                    os << ((endline_iterator->remove) ? "{r}" : "{nr}");
                    os << '\n';
                    endline_iterator++;
                }
                if (*it == '\n')
                    os << ' ';
                else os << *it;
            } os << "\n\n";
            return os;
        } 
        
    private:

        const uint8_t
            *first,
            *lineStart,
            *last,

            *lastPunc = nullptr,
            *lastWs = nullptr;
        
        uint16_t
            curWidth = 0,
            wsWidth,
            puncWidth,
            maxWidth,
            lastHSet = 0;
        uint8_t
            curLines = 0,
            maxLines;

        uint32_t
            curSize = 0,
            puncSize;

        txt2 textSection;
        struct code {
            code(const uint8_t *data, uint16_t size, double ratio) :
                data(data), size(size) {position.ratio = ratio;}
            code(const uint8_t *data, uint16_t size, const uint8_t *real) :
                data(data), size(size) {position.real = real;}
            union {
                double ratio;
                const uint8_t *real;
            } position;
            const uint8_t *data;
            uint16_t size;
        };
        std::vector<code> codes;
        size_t lastCodeIndex = 0;
        struct endline {
            const uint8_t *position;
            //size_t
            code_data_n::access_t data;
            
            bool remove;
        };
        std::vector<endline> endlines;

        std::vector<local_ruby_t> rubyOrder;
        std::stack<size_t> rubyOffsets;
        encodeCleanFlags_t flags;
};




void encodeClean(const std::filesystem::path &input, const std::filesystem::path &output, const std::filesystem::path &format, const std::filesystem::path &external, title::id id, width::view width, language language, encodeCleanFlags_t flags) {
    


    bool waitCodePresent = code_data_n::setWaitCode(id);
    code_data_n::setOptionOverrideCode(title::codeOverride.option());
    
    ktu::buffer textBuf, formatBuf, outBuf;

    if (!(textBuf.assign(input) && formatBuf.assign(format))) return;

    ktu::reader formatReader(formatBuf);
    uint64_t magic = formatReader.read<uint64_t>(); 
    uint32_t ml4_unknown = 0;
    bg4_t bg4;
    
    if ((magic & ktu::byte_mask(7)) != ktu::numeric_literal("MsgStdF")) return;

    
    unsigned archive_v = ((magic & ktu::byteswap<std::endian::native == std::endian::little, uint64_t>(0xFF)) >> ((std::endian::native == std::endian::little) ? 7*8 : 0));

    if (archive_v == mode::value::archiveML4) {
        ml4_unknown = formatReader.read_little_endian<uint32_t>();
    } else if (archive_v == mode::value::archiveBG4) {
        bg4.version = formatReader.read_little_endian<uint16_t>();
        bg4.fileEntryCountMultiplier = formatReader.read_little_endian<uint16_t>();
    }
    size_t fileBeg = 0;
    std::vector<fileData_t> fileTable;
    view v(textBuf.data(), id, width, language, flags, waitCodePresent); 
    while (formatReader.valid()) {
        bool compressed = false;
        if (formatReader.peek() == 0xFF) {
            formatReader.seek(formatReader.cur()+1);
            fileTable.push_back({.offset=(uint32_t)fileBeg, .size=(archive_v == mode::value::archiveBG4) ? 0x80000000 : 0});
            bg4.fileNames.push_back("");
            continue;
        } else if (formatReader.peek() == 0xFE) {
            formatReader.seek(formatReader.cur()+1);
            compressed = formatReader.read<uint8_t>();
            bg4.fileNames.push_back(std::string(formatReader.cur<char>()));
            formatReader.seek(formatReader.cur()+bg4.fileNames.back().size()+1);
            bg4.headerSize += bg4.fileNames.back().size()+1;
        }

        bool bigEndian = formatReader.read();
        uint8_t paddingChar = formatReader.read();
        maxBounds_t maxBounds(formatReader);

        uint32_t
            formatSize = formatReader.read_little_endian<uint32_t>(),
            preFormatSize = formatReader.read_little_endian<uint32_t>(),
            postFormatSize = formatReader.read_little_endian<uint32_t>();


        ktu::reader formatSectionReader(formatReader.cur(), formatSize);
        
        auto read = [&]<bool big_endian>(){

            uint16_t maxWidth;
            uint8_t maxLines;
        
            if (flags.maxwidth) {
                maxWidth = maxBounds.width();
                formatSectionReader.skip(sizeof(uint16_t));
            } else {
                maxWidth = formatSectionReader.read_little_endian<uint16_t>();
            }

            if (flags.maxlines) {
                maxLines = maxBounds.lines();
                formatSectionReader.skip();
            } else {
                maxLines = formatSectionReader.read();
            }

            v.assign(maxWidth, maxLines);

            auto code = [&]() {
                double position = formatSectionReader.read_big_endian<double>();
                const uint8_t *endOfCode;
                uint16_t codeSize;
                uint16_t codepoint;
                switch (codepoint = formatSectionReader.peek_big_endian<big_endian, uint16_t>()) {
                    case 0xE:
                        endOfCode = formatSectionReader.cur() +
                            (codeSize = ktu::big_endian<big_endian, uint16_t>(formatSectionReader.cur<uint16_t>()[3])+8);
                        break;

                    case 0xF:
                        endOfCode = formatSectionReader.cur() + (codeSize = 6);
                        break;
                    default:
                        if (0xDFFF < codepoint && codepoint < 0xF900) {
                            endOfCode = formatSectionReader.cur() + (codeSize = 2);
                            break;
                        }
                        return;
                }
                v.push_code<big_endian>(position, formatSectionReader.cur(), codeSize);
                formatSectionReader.seek(endOfCode);
                

            };
            auto endline = [&]() {
                uint8_t terminatorValue = formatSectionReader.read();
                
                if (!(terminatorValue & 0x80)) {
                    
                    v.readline<big_endian>();
                    v.push_code<big_endian>(0,0,0);
                }

                switch (terminatorValue & 0x7F) {
                    case terminator::endOfPage:
                        if (flags.autopage)
                            break;
                        v.pagebreak();
                        break;
                    case terminator::endOfString:
                        v.push_string<big_endian>();
                        width.set(100);
                        v.reset(formatSectionReader.read_little_endian<uint16_t>(), formatSectionReader.read());
                        if (!formatSectionReader.valid()) return;
                        break;
                    
                    case terminator::rubyPush:
                        v.ruby(local_ruby_t::begin);
                        break;
                    case terminator::rubyNext:
                        v.ruby(local_ruby_t::mid);
                        break;
                    case terminator::rubyPop:
                        v.ruby(local_ruby_t::end);
                        break;
                    case terminator::option:
                        v.option(big_endian);
                        break;
                    default:
                        break;
                }
            };

            width.set(100);
            while (formatSectionReader.valid()) {
                if (formatSectionReader.peek() < 0x40) {
                    code();
                } else {
                    endline();
                }
            }

        };
        if (bigEndian) read.operator()<true>();
        else read.operator()<false>();
    

        fileTable.push_back({.offset=(uint32_t)fileBeg});
        
        outBuf.insert(outBuf.end(), (void*)(formatReader.cur()+formatSize), preFormatSize);
        v.addBuffer(outBuf, paddingChar, bigEndian);
        outBuf.insert(outBuf.end(), formatReader.cur()+formatSize+preFormatSize, formatReader.cur()+formatSize+preFormatSize+postFormatSize);
        formatReader.seek(formatReader.cur()+formatSize+preFormatSize+postFormatSize);
        *(uint32_t*)(outBuf.data()+fileBeg+0x12) = ktu::big_endian<uint32_t>(outBuf.size(), bigEndian);
        fileTable.back().size = outBuf.size() - fileBeg;
        fileBeg = outBuf.size();
    }
    

    
    std::filesystem::path outputPath {output};
    const char *extension = "msbt";
    switch (archive_v) {
        case mode::value::archiveML4:
            extension = "dat";
            archive_ml4(fileTable, external, ml4_unknown);
            break;
        case mode::value::archiveBG4:
            extension = "dat";
            archive_bg4(outBuf, fileTable, bg4);
            break;
        default:
            break;
    }
    if (!outputPath.has_extension()) outputPath.replace_extension(extension);
    outBuf.write(outputPath);
}





#define MSM_ASSIGN_FLAG_GENERAL(STRNAME, VARNAME, TO_ASSIGN) \
    case ktu::hash(STRNAME): \
        VARNAME = TO_ASSIGN;            \
        break;

#define MSM_ASSIGN_FLAG(NAME)                       \
    MSM_ASSIGN_FLAG_GENERAL(#NAME, NAME, true)      \
    MSM_ASSIGN_FLAG_GENERAL(#NAME "~", NAME, false) 


void encodeCleanFlags_t::assign(const char *s, std::map<std::string, std::string> &vars) {
    std::string str = parse(s, vars);
    
    char *curParameter = str.data(), *nextParameter;
    while (*curParameter) {
        nextParameter = splitArgNextParameter(curParameter);
        switch (ktu::hash(curParameter)) {
            MSM_ASSIGN_FLAG(autopage)
            MSM_ASSIGN_FLAG(limitlines)
            MSM_ASSIGN_FLAG(maxlines)
            MSM_ASSIGN_FLAG(maxwidth)
            case ktu::hash("default"):
                *this = encodeCleanFlags_t();
                break;
        }
        
        curParameter = nextParameter;
    }
}