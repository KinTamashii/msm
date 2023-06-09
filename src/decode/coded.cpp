
#include <msm/decode/coded.hpp>
#include <string>
#include <ktu/bit.hpp>
#include <ktu/unicode.hpp>
#include <fmt/format.h>
#include <ktu/memory.hpp>
#include <msm/terminator.hpp>
#include <msm/decode/codes.hpp>
#include <msm/width.hpp>





template <bool big_endian>
void decodeCode(std::string &str, ktu::reader &reader, title::id id, std::stack<ruby_t> &rubies) {
    code_t code;
    code.group = reader.read_big_endian<big_endian, uint16_t>();
    code.type = reader.read_big_endian<big_endian, uint16_t>();
    if (code.group == 0) {
        switch (code.type) {
            case 0:
                ruby<big_endian>(str, reader, rubies);
                return;
            case 2:
                size<big_endian>(str, reader);
                return;
            case 3:
                color<big_endian>(str, reader);
                return;
            case 4:
                pagebreak<big_endian>(str, reader, rubies.size());
                return;
        }
    } else if (title::select<big_endian>(id, code, str, reader, rubies)) return;
    uint16_t size = (reader.read_big_endian<big_endian, uint16_t>() / 2);
    generic<big_endian>(str, code, size, reader);
}

template <bool big_endian>
inline void addWhitespace(std::string &str, const int start, const char * name, const uint16_t match, uint16_t &cur, ktu::reader &reader) {
    int i = start;
    while (reader.valid() && (cur = reader.read_big_endian<big_endian, uint16_t>()) == match) i++;
    str += fmt::format("\\{}[{}]", name, i);
}

template <bool big_endian>
bool checkWhitespace(std::string &str, uint16_t &cur, ktu::reader &reader) {
    while (reader.valid()) {
        cur = reader.read_big_endian<big_endian, uint16_t>();
        no_read:
        switch (cur) {
            case '\r':
                addWhitespace<big_endian>(str, 1, "carriage", ktu::big_endian<big_endian, uint16_t>('\r'), cur, reader);
                goto no_read;
            case '\t':
                addWhitespace<big_endian>(str, 1, "tab", ktu::big_endian<big_endian, uint16_t>('\t'), cur, reader);
                goto no_read;
            case ' ':
                addWhitespace<big_endian>(str, 1, "space", ktu::big_endian<big_endian, uint16_t>(' '), cur, reader);
                goto no_read;
            case 0x3000:
                addWhitespace<big_endian>(str, 1, "wspace", ktu::big_endian<big_endian, uint16_t>(0x3000), cur, reader);
                goto no_read;
            default:
                return true;
        }
    }
    return false;
}



template <bool big_endian>
void decodeString(std::string &text, ktu::reader reader, title::id id) {

    
    
    
    std::stack<ruby_t> rubies;

    auto checkRuby = [&rubies, &text, &reader](){
        if (rubies.size()) {
            auto &ruby = rubies.top();
            
            while (reader.cur() >= ruby.access[ruby.index]) {
                if (ruby.index) {
                    text += "}";
                    rubies.pop();
                    if (rubies.empty())
                        break;
                    ruby = rubies.top();
                    
                }
                text += "}{";
                ++ruby.index;
            }
        }
    };

    uint16_t cur;
    bool eos;
    bool inControlSequence = true;
    while (reader.valid()) {
        checkRuby();
        cur = reader.read_big_endian<big_endian, uint16_t>();
        
        no_read:
        switch (cur) {
            case 0xE:
                inControlSequence = true;
                decodeCode<big_endian>(text, reader, id, rubies);
                if (text.back() == '}' || text.back() == ']')
                    break;
                checkWhitespace<big_endian>(text, cur, reader);
                goto no_read;
                
            case 0xF: {
                inControlSequence = true;
                text += fmt::format(
                    R"(\endcode[{},{}])",
                    reader.read_big_endian<big_endian, uint16_t>(),
                    reader.read_big_endian<big_endian, uint16_t>()
                );
                break;
            }
            case '\\':
                inControlSequence = true;
                text += R"(\backslash )";
                checkWhitespace<big_endian>(text, cur, reader);
                goto no_read;
            case '\n':
                text += (rubies.size()) ? R"(\\)" : R"(\\)" "\n";
                checkWhitespace<big_endian>(text, cur, reader);
                inControlSequence = text.back() == ']';
                goto no_read;
            case '\r':
                cur = reader.read_big_endian<big_endian, uint16_t>();
                if (cur == '\r' || (eos = !cur)) {
                    addWhitespace<big_endian>(text, 2 - eos, "carriage", ktu::big_endian<big_endian, uint16_t>('\r'), cur, reader);
                    inControlSequence = true;
                    if (eos) return;
                } else {
                    text += '\r';
                }
                goto no_read;
            case '\t':
                cur = reader.read_big_endian<big_endian, uint16_t>();
                if (cur == '\t' || (eos = !cur)) {
                    addWhitespace<big_endian>(text, 2 - eos, "tab", ktu::big_endian<big_endian, uint16_t>('\t'), cur, reader);
                    inControlSequence = true;
                    if (eos) return;
                } else {
                    text += '\t';
                }
                goto no_read;
            case ' ':
                cur = reader.read_big_endian<big_endian, uint16_t>();
                if (cur == ' ' || (eos = !cur)) {
                    addWhitespace<big_endian>(text, 2 - eos, "space", ktu::big_endian<big_endian, uint16_t>(' '), cur, reader);
                    inControlSequence = true;
                    if (eos) return;
                } else {
                    text += ' ';
                }
                goto no_read;
            case 0x3000:
                cur = reader.read_big_endian<big_endian, uint16_t>();
                
                if (cur == 0x3000 || (eos = !cur)) {
                    addWhitespace<big_endian>(text, 2 - eos, "wspace", ktu::big_endian<big_endian, uint16_t>(0x3000), cur, reader);
                    inControlSequence = true;
                    if (eos) return;
                } else {
                    text += "　";
                }
                goto no_read;
            case '[':
            case '{':
                if (inControlSequence) {
                    inControlSequence = false;
                    text += '\\';
                }
                text += cur;
                break;
            case 0:
                checkRuby();
                return;
            default:
                if (0xDFFF < cur && cur < 0xF900) {
                    text += fmt::format(R"(\codepoint[0x{:04X}])", cur);
                    inControlSequence = true;
                    break;
                }
                inControlSequence = false;
                ktu::u8::push_back(text, cur);
                break;
        }
    }
}






#include <filesystem>
#include <vector>


template <bool big_endian>
inline void readUnknown(std::string &str, const char *name, ktu::reader &reader) {
    uint32_t sectionSize = reader.read_big_endian<big_endian, uint32_t>();
    reader.seek(reader.cur() + 8);
    const uint8_t *sectionEnd = reader.cur() + ktu::align2(sectionSize, 4);

    str += '\\';
    str += name;
    if (sectionSize) {
        str += '[';
        for (
            const uint8_t *dataTableBeforeEnd = reader.cur() + sectionSize - 1;
            reader.cur() != dataTableBeforeEnd;
        ) {
            str += fmt::format("{},", reader.read());
        }
        str += fmt::format("{}]", reader.peek());
    }
    reader.seek(sectionEnd);
    str += '\n';
}

template <bool big_endian>
void readATO1(std::string &str, ktu::reader &reader) {
    readUnknown<big_endian>(str, "ato", reader);
}

template <bool big_endian>
void readATR1(std::string &str, ktu::reader &reader) {
    readUnknown<big_endian>(str, "atr", reader);
}

template <bool big_endian>
uint32_t readLBL1(std::string &str, std::vector<std::string> &labels, ktu::reader &reader, uint8_t &paddingChar) {
    uint32_t sectionSize = reader.read_big_endian<big_endian, uint32_t>();
    reader.seek(reader.cur() + 8);
    const uint8_t *sectionEnd = reader.cur() + ktu::align2(sectionSize, 4);
    uint32_t numberOfGroups = reader.read_big_endian<big_endian, uint32_t>();
    const uint8_t *baseptr = reader.cur() - 4;
    const uint8_t *tmp = reader.cur();
    uint32_t labelCount = 0;
    for (int i = 0; i < numberOfGroups; i++) {
        labelCount += ktu::read_big_endian<big_endian, uint32_t>(tmp);
        tmp += 4;
    }
    labels.resize(labelCount);
    


    for (int i = 0; i < numberOfGroups; i++) {
        uint32_t numberOfLabels = reader.read_big_endian<big_endian, uint32_t>();
        uint32_t offset = reader.read_big_endian<big_endian, uint32_t>();
        const uint8_t *labelptr = baseptr + offset;
        for (int j = 0; j < numberOfLabels; j++) {
            uint8_t size = *labelptr++;
            std::string label = std::string((const char*)labelptr, size);
            labelptr += size;
            labels[ktu::read_big_endian<big_endian, uint32_t>(labelptr)] = label;
        }
    }
    reader.seek(sectionEnd);
    if (sectionSize & 0xF) paddingChar = reader.cur()[-1];

    return numberOfGroups;
}


template <bool big_endian>
void readNLI1(std::string &str, ktu::reader &reader) {
    readUnknown<big_endian>(str, "nli", reader);
}

#include <msm/decode/common.hpp>



template <bool big_endian>
inline void readTXT2(std::string &text, const std::vector<std::string> &labels, ktu::reader &reader, title::id id) {
    impl::readTXT2<big_endian, std::string&, const std::vector<std::string> &, ktu::reader&>(text, labels, reader, id);
}

template <bool big_endian>
inline void readTXT2(std::string &text, ktu::buffer &binary, ktu::reader &reader, title::id id) {
    impl::readTXT2<big_endian, std::string&, ktu::buffer &, ktu::reader&>(text, binary, reader, id);
}


template <bool big_endian>
void readTSY1(std::string &str, ktu::reader &reader) {
    uint32_t sectionSize = reader.read_big_endian<big_endian, uint32_t>();
    reader.seek(reader.cur() + 8);
    const uint8_t *sectionEnd = reader.cur() + ktu::align2(sectionSize, 4);

    str += '\\';
    str += "tsy";
    if (sectionSize) {
        str += '[';
        for (
            const uint8_t *dataTableBeforeEnd = reader.cur() + sectionSize - sizeof(uint32_t);
            reader.cur() != dataTableBeforeEnd;
        ) str += fmt::format("{},", reader.read_big_endian<big_endian, uint32_t>());
        str += fmt::format("{}]", reader.read_big_endian<big_endian, uint32_t>());
    }
    reader.seek(sectionEnd);
    str += '\n';
}

template <bool big_endian>
inline void skipSection(ktu::reader &reader, uint8_t &paddingChar) {
    uint32_t sectionSize = reader.read_big_endian<big_endian, uint32_t>();
    reader.seek(reader.cur() + 8);
    reader.seek(reader.cur() + ktu::align2(sectionSize, 4));
    if (sectionSize & 0xF) paddingChar = reader.cur()[-1];
}


template <bool big_endian>
void readSections(std::string &str, uint16_t numSections, ktu::reader &reader, title::id id) {
    class section {
        public:
            section(void (*fn)(std::string &, ktu::reader&), ktu::reader reader) : fn(fn), reader(reader) {}
            void execute(std::string &str) {
                fn(str, reader);
            }
        private:
            ktu::reader reader;
            void (*fn)(std::string &, ktu::reader&);
    };
    uint8_t paddingChar = 0xAB;
    std::vector<section> sections;
    std::vector<std::string> labels;
    ktu::reader txt2_reader;
    int lbl1_index = -1, txt2_index;
    uint32_t numberOfGroups;
    for (int i = 0; reader.valid() && i < numSections; i++) {
        uint8_t * sectionEnd, *cur;
        uint32_t sectionSize;

        switch (reader.read<uint32_t>()) {
            case ktu::numeric_literal<uint32_t>("ATO1"):
                sections.push_back(section(&readATO1<big_endian>, reader));
                skipSection<big_endian>(reader, paddingChar);
                break;
            case ktu::numeric_literal<uint32_t>("ATR1"): {
                sections.push_back(section(&readATR1<big_endian>, reader));
                skipSection<big_endian>(reader, paddingChar);
                break;
            }
            case ktu::numeric_literal<uint32_t>("LBL1"):
                lbl1_index = sections.size();
                numberOfGroups = readLBL1<big_endian>(str, labels, reader, paddingChar);
                break;
            case ktu::numeric_literal<uint32_t>("NLI1"):
                sections.push_back(section(&readNLI1<big_endian>, reader));
                skipSection<big_endian>(reader, paddingChar);
                break;
            case ktu::numeric_literal<uint32_t>("TXT2"):
                txt2_index = sections.size();
                txt2_reader = reader;
                skipSection<big_endian>(reader, paddingChar);
                break;
            case ktu::numeric_literal<uint32_t>("TSY1"):
                sections.push_back(section(&readTSY1<big_endian>, reader));
                skipSection<big_endian>(reader, paddingChar);
                break;
        }
    }
    str += fmt::format(R"(\padding[{}])" "\n\n", paddingChar);
    auto checkTXTLBL = [&lbl1_index,&txt2_index,&txt2_reader,&str,&numberOfGroups,&labels,id](int index) {
        if (lbl1_index != -1 && index == lbl1_index) {
            str += fmt::format(R"(\lbl[{}])" "\n", numberOfGroups);
        } else if (index == txt2_index && txt2_reader.cur()) {
            readTXT2<big_endian>(str, labels, txt2_reader, id);
        }
    };


    for (int i = 0; i < sections.size(); i++) {
        checkTXTLBL(i);
        sections[i].execute(str);
    }
    checkTXTLBL(sections.size());
}





void read(
    std::string &str,
    ktu::reader reader,
    title::id id,
    const std::filesystem::path &embedPath,
    const char *filename = "",
    const char *fileParameters = "",
    const char *embedParameters = nullptr
) {
    if (reader.read<uint64_t>() != ktu::numeric_literal<uint64_t>("MsgStdBn")) {
        str += fmt::format(R"(\embed)" "{{{}}}{}\n\n", (embedPath.filename() / filename).string(), embedParameters ? embedParameters : fileParameters);
        reader.writef(embedPath / filename);
        return;
    }
    
    str += fmt::format(R"(\file)" "{}\n", fileParameters);
    bool bigEndian = reader.read() < reader.read();
    str += fmt::format(R"(\bigendian[{}])" "\n", bigEndian);
    uint16_t unknown1, unknown2, unknown3, numSections;
    unknown1 = reader.read_little_endian<uint16_t>();
    unknown2 = reader.read_little_endian<uint16_t>();
    numSections = reader.read_big_endian<uint16_t>(bigEndian);
    unknown3 = reader.read_little_endian<uint16_t>();
    reader.seek(reader.cur()+sizeof(uint32_t));
    
    str += fmt::format(R"(\unknown[{},{},{})",
        unknown1,
        unknown2,
        unknown3);
    for (auto *unkEnd = reader.cur() + 10; reader.cur() != unkEnd;) {
        str += fmt::format(",{}", reader.read());
    }
    str += "]\n\n";

    ((bigEndian) ? readSections<true> : readSections<false>)(str, numSections, reader, id);
}



void decode(const std::filesystem::path &input, const std::filesystem::path& output, title::id id) {
    ktu::buffer buf;
    if (!buf.assign(input)) return;
    std::string str;
    std::filesystem::path embed_dir; (embed_dir = output).replace_extension("");
    read(str, ktu::reader(buf), id, embed_dir, "0.bin");
    std::filesystem::path outputPath = output;
    if (!outputPath.has_extension()) outputPath.replace_extension(".msm");
    ktu::writef(outputPath,str.data(),str.data()+str.size());
}








void decodeArchiveML4(const std::filesystem::path &input, const std::filesystem::path &output, const std::filesystem::path &external, title::id id) {
    // const std::filesystem::path &datPath = *globals.input;
    // const std::filesystem::path &binPath = globals.external;
    ktu::buffer datBuf, binBuf;
    std::filesystem::path outputPath = output;
    if (!outputPath.has_extension()) outputPath.replace_extension(".msm");
    if (!(datBuf.assign(input) && binBuf.assign(external))) return;
    ktu::reader datReader(datBuf), binReader(binBuf);

    binReader.seek(binReader.cur()+2);
    uint16_t tableLength = binReader.read_little_endian<uint16_t>();
    uint32_t unknown = binReader.read_little_endian<uint32_t>();

    std::string str = fmt::format("\\archive{{ml4}}[{}]\n\n", unknown);
    
    binReader.seek(binReader.cur()+8);

    std::filesystem::path embed_dir; (embed_dir = output).replace_extension("");
    
    size_t i = 0;
    for (auto end = binReader.cur() + (tableLength * 8); binReader.valid() && binReader.cur() != end;) {
        uint32_t
            offset = binReader.read_little_endian<uint32_t>(),
            size = binReader.read_little_endian<uint32_t>();
        if (!size) {
            str += "\\empty\n\n";
            continue;
        }
        read(str, ktu::reader(datBuf.data() + offset, size), id, embed_dir, fmt::format("{}.bin", i++).c_str());
        str += "\n\n";
    }

    
    
    ktu::writef(outputPath,str.data(),str.data()+str.size());
    
};


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
void decodeArchiveBG4(const std::filesystem::path &input, const std::filesystem::path &output, title::id id) {
    
    ktu::buffer inputbuf;
    if (!inputbuf.assign(input))
        return;
    ktu::reader inputReader(inputbuf);

    uint32_t magic = inputReader.read<uint32_t>();
    if (magic != ktu::numeric_literal("BG4\0"))
        return;
    
    uint16_t version                    = inputReader.read_little_endian<uint16_t>();
    uint16_t fileEntryCount             = inputReader.read_little_endian<uint16_t>();
    uint32_t metaSecSize                = inputReader.read_little_endian<uint32_t>();
    uint16_t fileEntryCountDerived      = inputReader.read_little_endian<uint16_t>();
    uint16_t fileEntryCountMultiplier   = inputReader.read_little_endian<uint16_t>();

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
    std::string info = fmt::format("\\archive{{bg4}}[{},{}]\n\n", version, fileEntryCountMultiplier);

    ktu::buffer uncompressBuffer;
    std::filesystem::path embed_dir; (embed_dir = output).replace_extension("");
    for (auto &entry : ktu::range(entries, entries+fileEntryCount)) {
        if (entry.fileSize & 0x80000000) {
            info += "\\empty\n\n";
            continue;
        }
        bool compressed = (entry.fileOffset & 0x80000000);
    
        
    
        filePos = inputReader.begin()+(entry.fileOffset & 0x7FFFFFFF);
        fileSize = entry.fileSize & 0x7FFFFFFF;
        
        if (compressed) {
            
            if (!uncompress(filePos, fileSize, uncompressBuffer))
                return;
            
            filePos = uncompressBuffer.data();
            fileSize = uncompressBuffer.size();
        }

        ktu::reader fileReader(filePos, fileSize);

        read(info, fileReader, id, embed_dir, stringReader.begin<char>() + entry.nameOffset,
            fmt::format("{{{}}}[{}]", (stringReader.begin<char>() + entry.nameOffset), compressed ? "true" : "false").c_str(),
            fmt::format("[{}]", compressed ? "true" : "false").c_str());
        

        uncompressBuffer.clear();
    }


    delete[] entries;

    ktu::writef(output,info.data(),info.data()+info.size());
}