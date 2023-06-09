#include <msm/decode/clean.hpp>
#include <string>
#include <ktu/bit.hpp>
#include <ktu/unicode.hpp>
#include <fmt/format.h>
#include <ktu/memory.hpp>
#include <msm/terminator.hpp>

#include <msm/mode.hpp>
#include <msm/decode/codes.hpp>
#include <msm/width.hpp>
#include <msm/common.hpp>









template <bool big_endian>
maxBounds_t decodeString(std::string &text, ktu::buffer &binary, ktu::reader reader, title::id id, width::view &width) {
   

    width.set(100);

    

    struct local_code_t {
        local_code_t(const uint8_t * const ptr, const uint32_t size, const uint32_t position) : ptr(ptr), size(size), position(position) {}
        const uint8_t * const ptr;
        const uint32_t size;
        const uint32_t position;
    };
    std::vector<local_code_t> codes;
    uint32_t lineSize = 0;
    maxBounds_t
        curBounds (1,0),
        maxBounds (1,0);
    // uint16_t maxLineWidth = 1;
    // uint8_t maxLines = 0;
    // uint16_t curLineWidth = 1;
    // uint8_t curLines = 0;
    uint16_t cur;
    auto update = [&lineSize, &curBounds, &cur, &text, &width](){
        lineSize++;
        curBounds.addWidth(width[cur]);//curLineWidth += width[cur];
        ktu::u8::push_back(text, cur);
    };
    

    auto push = [&codes, &lineSize](const uint8_t * const ptr, uint32_t size){
        codes.push_back({ptr, size, lineSize});
    };
    size_t lastTextSize = text.size();
    
    auto endline = [&curBounds, &maxBounds, &text, &codes, &binary, &lineSize, &lastTextSize, &reader](uint8_t terminatorValue){

        /*
            &7F {
                00..3F => double(position)[0.0L-1.0L] : {code}
                40 => end_of_string
                41..7F => universal : line_aligned_code
            }
            &80 {
                endline or not
            }
        */
        for (auto &code : codes) {
            
            double fractional_position = code.position / (double) lineSize;

            binary.push_back_big_endian<double>((std::isnan(fractional_position)) ? 0.0 : fractional_position);
            binary.insert(binary.end(), code.ptr, code.ptr + code.size);
        }
        bool endsWithNewline = (lastTextSize != text.size());
        binary.push_back<uint8_t>((!endsWithNewline << 7) | terminatorValue);
        codes.clear();
        
        if (endsWithNewline) text += '\n';
        lastTextSize = text.size();
        lineSize = 0;

        
        maxBounds.setMaxWidth(curBounds);
        curBounds.addLine();

        //maxLineWidth = std::max(maxLineWidth, curLineWidth);
        //curLineWidth = 1;
        //curLines++;
        
    };
    
    size_t widthDataIndex = binary.size();
    binary.insert(binary.end(),3,(uint8_t)0);
    


    
    std::stack<ruby_t> rubies;

    auto checkRuby = [&rubies, &reader, &binary, &text, &endline](){

        if (rubies.size()) {
            auto &ruby = rubies.top();

            while (reader.cur() >= ruby.access[ruby.index]) {
                if (ruby.index) {
                    endline(terminator::rubyPop);
                    rubies.pop();
                    if (rubies.empty())
                        break;
                    ruby = rubies.top();
                }
                endline(terminator::rubyNext);
                ++ruby.index;
            }
        }
    };
    const uint8_t *beg = reader.cur();
    
    

    std::string tmp;

    while (reader.valid()) {
        checkRuby();
        cur = reader.read_big_endian<big_endian, uint16_t>();
        
        no_read:
        switch (cur) {
            case 0xE: {
                tmp += "[code]";
                uint16_t
                    group = reader.read_big_endian<big_endian, uint16_t>(),
                    type = reader.read_big_endian<big_endian, uint16_t>(),
                    size = reader.read_big_endian<big_endian, uint16_t>();
                
                switch (group) {
                    case 0x0000:
                        switch (type) {
                            case 0x0000: {
                                if (size > 1) {
                                    
                                    uint16_t rbSize = reader.read_big_endian<big_endian, uint16_t>();
                                    uint16_t rtSize = reader.read_big_endian<big_endian, uint16_t>();

                                    endline(terminator::rubyPush);
                                    rubies.push({reader.cur()+rtSize, reader.cur()+rtSize+rbSize});
                                    goto next;
                                }
                                push(reader.cur() - 8, size + 8);
                                goto next;
                            }

                            case 0x0002:
                                if (size > 1) {
                                    width.set(reader.peek_big_endian<big_endian, uint16_t>());
                                }
                                push(reader.cur() - 8, size + 8);
                                break;

                            case 0x0004:
                                endline(terminator::endOfPage);
                                maxBounds.setMaxLines(curBounds);
                                curBounds.resetLines();
                                //maxLines = std::max(maxLines, curLines);
                                //curLines = 0;
                                break;
                            default: goto default_path;
                                
                        }
                        break;
                    


                    default:
                        if (title::option(id, {group, type})) {
                            endline(terminator::option);
                            break;
                        }
                        if (size == 2) {
                            if (title::hspace(id, {group, type})) {
                                curBounds.addWidth(reader.peek_big_endian<big_endian, uint16_t>());
                                //curLineWidth += reader.peek_big_endian<big_endian, uint16_t>();
                            } else if (title::hset(id, {group, type})) {
                                curBounds.setWidth(reader.peek_big_endian<big_endian, uint16_t>());
                                //curLineWidth = reader.peek_big_endian<big_endian, uint16_t>();
                            }
                        }
                        default_path:
                        push(reader.cur() - 8, size + 8);
                        break;
                }
                
                
                reader.seek(reader.cur() + size);
                break;
            }
            case 0xF:
                tmp += "[endcode]";
                reader.seek(reader.cur() + 4);
                push(reader.cur() - 6, 6);
                break;
            case '\n':
                tmp += "[newline]";
                maxBounds.setMaxWidth(curBounds);
                curBounds.addLine();
                // maxLineWidth = std::max(maxLineWidth, curLineWidth);
                // curLineWidth = 1;
                // curLines++;
                while (reader.valid() && (cur = reader.read_big_endian<big_endian, uint16_t>()) == '\n')
                    curBounds.addLine<false>();//curLines++;
                if (text.back() != ' ' && cur != ' ') {text += ' '; curBounds.addWidth(width[' ']);}//curLineWidth += width[' ']
                goto no_read;
            case '\r':
            case '\t':
            case ' ':
            case 0x3000:
                ktu::u8::push_back(tmp, cur);
                ktu::u8::push_back(text, cur);
                curBounds.addWidth(width[cur]);//curLineWidth += width[cur];
                break;
            case 0:
                goto exit_function;
                // endline(terminator::endOfString);
                
                // *(uint16_t*)(binary.data()+widthDataIndex) = maxLineWidth;
                // *(binary.data()+widthDataIndex+2) = std::max(maxLines, curLines);
                
                // return;
            default:
                if (0xDFFF < cur && cur < 0xF900) {
                    push(reader.cur() - 2, 2);
                    break;
                }
                ktu::u8::push_back(tmp, cur);
                update();
                break;
        }
        next:;
        
    }
    exit_function:
    endline(terminator::endOfString);
    *(uint16_t*)(binary.data()+widthDataIndex) = maxBounds.width();//maxLineWidth;
    *(binary.data()+widthDataIndex+2) = maxBounds_t::maxLines(maxBounds, curBounds);//std::max(maxLines, curLines);
    // /fmt::print("({}, {});\n", maxBounds.width(), maxBounds.lines());
    return maxBounds;
}





#include <filesystem>
#include <vector>

#include <msm/decode/common.hpp>




template <bool big_endian>
inline maxBounds_t readTXT2(std::string &text, ktu::buffer &binary, ktu::reader &reader, title::id id, width::view& width) {
    return impl::readTXT2<big_endian, std::string&, ktu::buffer &, ktu::reader&, title::id, width::view&>(
        text, binary, reader, id, width
    );
}



template <bool big_endian>
inline void skipSection(ktu::reader &reader, uint8_t &paddingChar) {
    uint32_t sectionSize = reader.read_big_endian<big_endian, uint32_t>();
    reader.seek(reader.cur() + 8);
    reader.seek(reader.cur() + ktu::align2(sectionSize, 4));
    if (sectionSize & 0xF) paddingChar = reader.cur()[-1];
}




void readClean(std::string &text, ktu::buffer &binary, ktu::reader reader, title::id id, width::view& width) {
    
    if (reader.read<uint64_t>() != ktu::numeric_literal<uint64_t>("MsgStdBn"))
        return;
    bool bigEndian = reader.read() < reader.read();
    
    reader.seek(0x20);
    
    uint8_t paddingChar = 0xAB;
    uint32_t preSize = 0, postSize = 0;
    
    while (reader.valid()) {
        
        if (reader.read<uint32_t>() == ktu::numeric_literal("TXT2")) {
            
            size_t pos = binary.size();
            
            binary.resize(binary.size() + 17);

            auto *ptr = &binary[pos]; preSize = (reader.cur() - 4) - reader.begin();
            *ptr++ = bigEndian;
            *ptr++ = paddingChar;
            ptr += 7;

            ktu::write_little_endian(ptr, preSize);
            
            
            
            maxBounds_t maxBounds(
                (bigEndian) ? readTXT2<true>(text, binary, reader, id, width) : readTXT2<false>(text, binary, reader, id, width)
            );
            postSize = (reader.cur() <= reader.end()) * (reader.end() - reader.cur());
            
            
    
            ptr = &binary[pos + 2];
            maxBounds.write(ptr);
            ktu::write_little_endian<uint32_t>(ptr, binary.size()- pos - 17);
            ptr += 4;
            ktu::write_little_endian(ptr, postSize);
            pos = binary.size();
            binary.resize(binary.size() + preSize + postSize);
            ptr = &binary[pos];
            memcpy(ptr, reader.begin(), preSize);
            ptr += preSize;
            memcpy(ptr, reader.cur(), postSize);
            break;
        }
        ((bigEndian) ? skipSection<true> : skipSection<false>)(reader, paddingChar);
        
    }
    
    
}

void decodeClean(const std::filesystem::path &input, const std::filesystem::path &output, const std::filesystem::path &format, title::id id, width::view width) {
    
    std::string text;
    ktu::buffer buf, binary{(uint64_t)ktu::numeric_literal("MsgStdF\0")};
    if (!buf.assign((input))) return;
    
    readClean(text, binary, ktu::reader(buf), id, width);

    binary.writef(format);
    ktu::writef(output, text.data(), text.data()+text.size());
}



// #include <msm/mode.hpp>


void decodeCleanArchiveML4(const std::filesystem::path &input, const std::filesystem::path &output, const std::filesystem::path &format, const std::filesystem::path &external, title::id id, width::view width) {
    const std::filesystem::path &datPath = input;
    const std::filesystem::path &binPath = external;
    
    ktu::buffer inputbuf, binBuf;
    if (!(inputbuf.assign(datPath) && binBuf.assign(binPath))) return;
    ktu::reader binReader(binBuf);

    binReader.seek(binReader.cur()+2);
    uint16_t tableLength = binReader.read_little_endian<uint16_t>();
    uint32_t unknown = binReader.read_little_endian<uint32_t>();

    std::string text;
    
    ktu::buffer binary{(uint64_t)ktu::numeric_literal("MsgStdF\0") | ktu::big_endian<std::endian::native == std::endian::little, uint64_t>((uint8_t)(mode::value::archiveML4))}; //mode::value::archiveML4  >> (shift-1)
    binary.push_back_little_endian(unknown);
    
    binReader.seek(binReader.cur()+8);

    
    
    for (auto end = binReader.cur() + (tableLength * 8); binReader.valid() && binReader.cur() != end;) {
        uint32_t
            offset = binReader.read_little_endian<uint32_t>(),
            size = binReader.read_little_endian<uint32_t>();
        if (!size) {
            binary.push_back<uint8_t>(0xFF);
            continue;
        }
        readClean(text, binary, ktu::reader(inputbuf.data() + offset, size), id, width);
    }

    
    binary.writef(format);
    ktu::writef(output, text.data(), text.data()+text.size());
} 

struct Bg4Entry {
    static constexpr size_t nonalignedsize = sizeof(uint32_t)*3 + sizeof(uint16_t);
    uint32_t fileOffset;
    uint32_t fileSize;
    uint32_t nameHash;
    uint16_t nameOffset;
    auto friend operator <=>(const Bg4Entry &lhs, const Bg4Entry &rhs) {
        return lhs.nameOffset <=> rhs.nameOffset;
    }
};
#include <msm/blz77.hpp>
void decodeCleanArchiveBG4(const std::filesystem::path &input, const std::filesystem::path &output, const std::filesystem::path &format, title::id id, width::view width) {

    

    ktu::buffer inputbuf;
    if (!inputbuf.assign(input))
        return;
    ktu::reader inputReader(inputbuf);

    std::string text;
        
    ktu::buffer binary{(uint64_t)ktu::numeric_literal("MsgStdF\0") | ktu::big_endian<std::endian::native == std::endian::little, uint64_t>((uint8_t)(mode::value::archiveBG4))}; //  >> (shift-1)
    
   

    uint32_t magic = inputReader.read<uint32_t>();
    if (magic != ktu::numeric_literal("BG4\0"))
        return;
    
    uint16_t version                    = inputReader.read_little_endian<uint16_t>();
    uint16_t fileEntryCount             = inputReader.read_little_endian<uint16_t>();
    uint32_t metaSecSize                = inputReader.read_little_endian<uint32_t>();
    uint16_t fileEntryCountDerived      = inputReader.read_little_endian<uint16_t>();
    uint16_t fileEntryCountMultiplier   = inputReader.read_little_endian<uint16_t>();

    binary.push_back_little_endian<uint16_t>(version);
    binary.push_back_little_endian<uint16_t>(fileEntryCountMultiplier);

    Bg4Entry *entries = new Bg4Entry[fileEntryCount], *entry_it = entries;
    for (auto end = inputReader.cur()+Bg4Entry::nonalignedsize*fileEntryCount; inputReader.cur() != end; ) {
        Bg4Entry entry = Bg4Entry{
            .fileOffset = inputReader.read_little_endian<uint32_t>(),
            .fileSize = inputReader.read_little_endian<uint32_t>(),
            .nameHash = inputReader.read_little_endian<uint32_t>(),
            .nameOffset = inputReader.read_little_endian<uint16_t>()
        };
        *entry_it++ = entry;
        
    }
    

    ktu::reader stringReader(inputReader.slice(
        inputReader.cur(), metaSecSize - (inputReader.cur() - inputReader.begin())
    ));
    
    const uint8_t *filePos; size_t fileSize;
    
    ktu::buffer uncompressBuffer;
    for (auto &entry : ktu::range(entries, entries+fileEntryCount)) {
        if (entry.fileSize & 0x80000000) {
            binary.push_back<uint8_t>(0xFF);
            continue;
        }
        bool compressed = (entry.fileOffset & 0x80000000);

        binary.push_back<uint8_t>(0xFE);
        binary.push_back<uint8_t>(compressed);
        binary.push_back((void*)(stringReader.begin<char>() + entry.nameOffset), strlen(stringReader.begin<char>() + entry.nameOffset));
        binary.push_back<uint8_t>(0x00);
        
        filePos = inputReader.begin()+(entry.fileOffset & 0x7FFFFFFF);
        fileSize = entry.fileSize & 0x7FFFFFFF;
        
        if (compressed) {
            
            if (!uncompress(filePos, fileSize, uncompressBuffer))
                return;
            
            filePos = uncompressBuffer.data();
            fileSize = uncompressBuffer.size();
        }

        ktu::reader fileReader(filePos, fileSize);

        if (fileReader.peek<uint64_t>() == ktu::numeric_literal("MsgStdBn"))
            readClean(text, binary, fileReader, id, width);
        else {
            fmt::print("Not implemented yet.");
            exit(-1);
        }

        uncompressBuffer.clear();
    }

    

    delete[] entries;
    
    
    binary.writef(format);
    ktu::writef(output, text.data(), text.data()+text.size());
    
    
}