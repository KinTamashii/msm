#include <msm/encode/coded.hpp>

#include <msm/abbreviations.hpp>

#include <msm/width.hpp>






void archive_ml4(const std::vector<fileData_t> &fileTable, const std::filesystem::path &external, uint32_t ml4unknown) {
    uint16_t dataSize = fileTable.size()*(sizeof(uint32_t));
    ktu::buffer table((fileTable.size()*sizeof(fileData_t))+0x10);
    
    dataSize /= 4;
    uint8_t *ptr = table.data() + 2;
    ktu::write_little_endian<uint16_t>(ptr, fileTable.size());
    ktu::write_little_endian<uint32_t>(ptr, ml4unknown);
    ktu::write_little_endian<uint32_t>(ptr, table.size()); ptr += 2;
    ktu::write_little_endian<uint16_t>(ptr, fileTable.size()-1);
    for (auto f : fileTable) {
        ktu::write_little_endian<uint32_t>(ptr, f.offset);
        ktu::write_little_endian<uint32_t>(ptr, f.size);
    }
    table.writef(external);
    
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

void archive_bg4(ktu::buffer &buf, const std::vector<fileData_t> &fileTable, const bg4_t &bg4) {
    size_t entriesSize = fileTable.size() * Bg4Entry::nonalignedsize;
    
    size_t headerSize = bg4.headerSize + entriesSize;
    const char invalidString[] = "(invalid)";

    uint32_t headerPaddedSize = ktu::align2(headerSize + sizeof(invalidString), 2);




    buf.shift(0, headerPaddedSize);
    const uint8_t* headerPtr = buf.data(), *stringPtrStart = buf.data() + 0x10 + entriesSize, *stringPtr = stringPtrStart;
    memset((void*)headerPtr, 0, headerPaddedSize);
    ktu::write_little_endian(headerPtr, ktu::numeric_literal("BG4\0"));
    ktu::write_little_endian(headerPtr, bg4.version);
    ktu::write_little_endian<uint16_t>(headerPtr, fileTable.size());
    ktu::write_little_endian(headerPtr, headerPaddedSize);
    ktu::write_little_endian<uint16_t>(headerPtr, fileTable.size() / bg4.fileEntryCountMultiplier);
    ktu::write_little_endian(headerPtr, bg4.fileEntryCountMultiplier);

    memcpy((void*)stringPtr, &invalidString[0], sizeof(invalidString) - 1);
    stringPtr += sizeof(invalidString);
    auto fileIt = fileTable.begin(), fileEnd = fileTable.end();
    auto nameIt = bg4.fileNames.begin(), nameEnd = bg4.fileNames.end();


    for (;fileIt != fileEnd && nameIt != nameEnd; fileIt++, nameIt++) {
        
        const auto &file = *fileIt;
        const auto &name = *nameIt;
        
        auto hash = [](const char *str, size_t size) constexpr {
            uint32_t value = 0;
            const char *ptr = str + size - 1;
            while (ptr >= str) {
                value = value * 0x1F + *ptr;
                --ptr;
            }
            return value;
        };
        
        
        
        if (file.size & 0x80000000) {
            ktu::write_little_endian<uint32_t>(headerPtr, file.offset);
            ktu::write_little_endian<uint32_t>(headerPtr, file.size);
            ktu::write_little_endian<uint32_t>(headerPtr, 0xFFFFFFFF);
            ktu::write_little_endian<uint16_t>(headerPtr, 0x0);
        } else {
            ktu::write_little_endian<uint32_t>(headerPtr, file.offset + headerPaddedSize);
            ktu::write_little_endian<uint32_t>(headerPtr, file.size);
            ktu::write_little_endian<uint32_t>(headerPtr, hash(name.data(), name.size()));
            ktu::write_little_endian<uint16_t>(headerPtr, stringPtr-stringPtrStart);
            memcpy((void*)stringPtr, name.data(), name.size());
            stringPtr += name.size() + 1;
        }
    }
    memset((void*)stringPtr, 0xFF, headerPaddedSize-(stringPtr-buf.data()));
}

void encode_t::push(const char *ptr, const char *end) {
    states.push_back(std::move(mut));
    mut.ptr = ptr;
    mut.end = end;
    mut.textParameters.clear();
    mut.valueParameters.clear();
    while (valid()) {
        read();
        no_read:

        if (mut.codepoint == '\\') {
            control_sequence:
            if (!valid()) break;
            if (controlSequence() || valid()) goto no_read;
            break;
            
        }
        
        if (isWhitespace()) {
            skipWhitespace();
            if (valid()) goto no_read;
            break;
        }
        

        while (valid()) {
            if (inString) {
                if (isWhitespace()) {
                    skipWhitespaceAdd();
                    if (valid()) goto no_read;
                    break;
                } else {
                    append();
                    
                }
            }
            if (!valid()) break;
            read();
            if (mut.codepoint == '\\')
                goto control_sequence;
               
        }
        if ((inString) && !valid() && !isWhitespace() && mut.codepoint != '\\')
            {append();}
            

        
    }
    mut = std::move(states.back());
    states.pop_back();
}



bool encode_t::controlSequence() {
    
    read();
    if (mut.codepoint == '\\') {
        
        if (inString) {inStringH();append('\n');}
        if (valid()) read();
        
        return true;
    }
    
    
    bool param = true;
    if (isVariable()) {
        std::string command = getVariable();
        mut.state &= ~(mutableData::whitespace);
        bool addLast = false;
        if (valid()) goto no_read;
        while (valid()) {
            
            read();
            no_read:
            if (mut.codepoint == '{') {
                getTextParameter();
                mut.state &= ~(mutableData::whitespace);
                mut.state |= mutableData::hasParameters;
                continue;
            }
            if (mut.codepoint == '[') {
                getValueParameters();
                mut.state &= ~(mutableData::whitespace);
                mut.state |= mutableData::hasParameters;
                continue;
            }
            if (isWhitespace()) {
                
                skipWhitespace();
                mut.state &= ~(mutableData::whitespace);
                mut.state |= (mutableData::whitespace);
                
                if (valid())
                    goto no_read;

                break;
            }
            addLast = (!valid() && mut.codepoint != '\\');
            break;
        }
        selectCommand(command);
        
        
        if ((mut.state & mutableData::whitespace) && (mut.state & mutableData::hasParameters)) {
            
            if (mut.codepoint == '\\') {
                
                mut.state &= ~(mutableData::whitespaceField);
                mut.state |= mutableData::queueWhitespace | mutableData::usePrevWs;
                
            } else if (inString) {
                
                uint32_t queuedCodepoint = mut.state & mutableData::usePrevWs ? prevWsCodepoint : lastWsCodepoint;
                queuedCodepoint = queuedCodepoint == '\n' ? ' ' : queuedCodepoint;
                if (valid()) {append(queuedCodepoint);}
                else if (!isWhitespace()) {
                    append(queuedCodepoint);
                    append();
                }
                
                
            }
        } else {
            mut.state &= ~(mutableData::usePrevWs);
        }
        
        mut.state &= ~(mutableData::hasParameters);
        
        if ((inString) && addLast) { 
            append();
            
        }
        param = mut.textParameters.empty() && mut.valueParameters.empty();
        mut.textParameters.clear();
        mut.valueParameters.clear();
        
    }
    return false;
}


void encode_t::selectCommand(const std::string &str) {

    auto outString = [this](){
        mut.state &= ~(mutableData::whitespaceField);
        if (inString) {
            inString = false;
            pushString();
        }
    };


    switch (ktu::hash(str)) {
        case ktu::hash("ruby"):
            if (!(inString)) break;
            inStringH();
            ruby();
            return;
        case ktu::hash("color"):
            if (!(inString)) break;
            inStringH();
            color();
            return;
        case ktu::hash("size"):
            if (!(inString)) break;
            inStringH();
            size();
            return;
        case ktu::hash("pagebreak"): 
            if (!(inString)) break;
            inStringH();
            pagebreak();
            return;
        case ktu::hash("code"):
            if (!(inString)) break;
            inStringH();
            generic();
            return;
        case ktu::hash("endcode"):
            if (!(inString)) break;
            inStringH();
            if (mut.valueParameters.size() > 1)
                endCode(mut.valueParameters[0], mut.valueParameters[1]);
            break;
        case ktu::hash("newline"):
            if (!(inString)) break;
            inStringH();
            append('\n');
            break;
        case ktu::hash("backslash"):
            if (!(inString)) break;
            inStringH();
            append('\\');
            break;
        case ktu::hash("vbar"):
            if (!(inString)) break;
            inStringH();
            append('|');
            break;
        case ktu::hash("lbrack"):
            if (!(inString)) break;
            inStringH();
            append('[');
            break;
        case ktu::hash("rbrack"):
            if (!(inString)) break;
            inStringH();
            append(']');
            break;
        case ktu::hash("lbrace"):
            if (!(inString)) break;
            inStringH();
            append('{');
            break;
        case ktu::hash("rbrace"):
            if (!(inString)) break;
            inStringH();
            append('}');
            break;
        case ktu::hash("codepoint"):
            if (!(inString)) break;
            inStringH();
            for (auto val : mut.valueParameters)
                {append(val);}
            break;
        case ktu::hash("space"):
            if (!(inString)) break;
            inStringH();
            if (mut.valueParameters.empty()) break;
            for (int i = 0; i < mut.valueParameters[0]; i++) {
                append(' ');
            }
            break;
        case ktu::hash("carriage"):
            if (!(inString)) break;
            inStringH();
            if (mut.valueParameters.empty()) break;
            for (int i = 0; i < mut.valueParameters[0]; i++) {
                append('\r');
            }
            break;
        case ktu::hash("tab"):
            if (!(inString)) break;
            inStringH();
            if (mut.valueParameters.empty()) break;
            for (int i = 0; i < mut.valueParameters[0]; i++) {
                append('\t');
            }
            break;
        case ktu::hash("wspace"):
            if (!(inString)) break;
            inStringH();
            if (mut.valueParameters.empty()) break;
            for (int i = 0; i < mut.valueParameters[0]; i++) {
                append(0x3000);
            }
            break;
        case ktu::hash("file"): {
            outString();
            push_file();
            inFile = true;
            fileTable.push_back(fileData_t{
                .offset=(uint32_t)buf.size() | ((archive_v == mode::value::archiveBG4 && mut.valueParameters.size() && mut.valueParameters[0]) * 0x80000000), .size = 0x00000000
            });
            if (mut.textParameters.size()) {
                bg4.fileNames.push_back(std::string(mut.textParameters[0].ptr, mut.textParameters[0].end));
                bg4.headerSize += (mut.textParameters[0].end-mut.textParameters[0].ptr) + 1;
            } else {
                bg4.fileNames.push_back("");
            }
            curBegin = buf.size();
            buf.insert(
                buf.end(),
                std::initializer_list<uint8_t>{
                    0x4D,0x73,0x67,0x53,0x74,0x64,0x42,0x6E,
                    0x00,0x00,
                    0x00,0x00,
                    0x01,0x03,
                    0x00,0x00,
                    0x00,0x00,
                    0x00,0x00,0x00,0x00,
                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
                }
            );
            return;
        }
        case ktu::hash("empty"): {
            outString();
            bool isEmpty = true;
            push_file();
            inFile = false;
            fileTable.push_back(fileData_t{
                .offset = (uint32_t)buf.size(),
                .size = (archive_v == mode::value::archiveBG4) ? 0x80000000 : 0x00000000
            });
            bg4.fileNames.push_back("");
            return;
        }
        case ktu::hash("embed"): {
            outString();
            push_file();
            inFile = false;
            if (mut.textParameters.empty())
                return;
            bool compressed = mut.valueParameters.size() && mut.valueParameters[0];
            ktu::buffer embeddedFile;
            
            
            std::filesystem::path embeddedDir = inputPath.parent_path() / std::string(mut.textParameters[0].ptr, mut.textParameters[0].end);
            std::cout << embeddedDir  << '\n';
            if (!embeddedFile.assign(embeddedDir))
                return;
            std::cout << "!!!\n";
            std::string filename = embeddedDir.filename().string();
            bg4.fileNames.push_back(filename);
            bg4.headerSize += filename.size() + 1;

            fileTable.push_back(fileData_t{.offset=(uint32_t)buf.size()});
            if (compressed) {
                compress(embeddedFile.data(), embeddedFile.size(), buf);
            } else {
                buf.insert(buf.end(), embeddedFile.begin(), embeddedFile.end());
            }
            fileTable.back().size = buf.size() - fileTable.back().offset;
            fileTable.back().offset |= 0x80000000 * compressed;
            return;
        }
            
            
        case ktu::hash("archive"):
            outString();
            if (mut.textParameters.empty()) return;
            {
                const auto &textParam = mut.textParameters[0];
                switch (ktu::hash(textParam.ptr, textParam.end-textParam.ptr)) {
                    case ktu::hash("ml4"):
                        archive_v = mode::value::archiveML4;
                        if (mut.valueParameters.empty())
                            return;
                        ml4.unknown = mut.valueParameters[0];
                        return;
                    case ktu::hash("bg4"):
                        archive_v = mode::value::archiveBG4;
                        if (mut.valueParameters.empty())
                            return;
                        bg4.version = mut.valueParameters[0];
                        if (mut.valueParameters.size() < 2) {
                            return;
                        }
                        bg4.fileEntryCountMultiplier = mut.valueParameters[1];
                        return;
                }
                
            }
            return;
        case ktu::hash("bigendian"):
            outString();
            if (mut.valueParameters.empty()) return;
            bigEndian = mut.valueParameters[0];
            setEndian();
            return;
        case ktu::hash("unknown"): {
            outString();
            if (mut.valueParameters.empty()) return;
            uint8_t *ptr = &buf.access(curBegin + 10);
            ktu::write_little_endian<uint16_t>(ptr, mut.valueParameters[0]);
            if (mut.valueParameters.size() < 2) return;
            ktu::write_little_endian<uint16_t>(ptr, mut.valueParameters[1]);
            if (mut.valueParameters.size() < 3) return;
            ptr += sizeof(uint16_t);
            ktu::write_little_endian<uint16_t>(ptr, mut.valueParameters[2]);
            ptr += sizeof(uint32_t);
            int limit = std::min((int)mut.valueParameters.size(), 13);
            for (int i = 3; i < limit; i++) {
                *ptr++ = mut.valueParameters[i];
            }
            return;
        }
        case ktu::hash("padding"): {
            outString();
            if (mut.valueParameters.empty()) return;
            paddingChar = mut.valueParameters[0];
            return;
        }
        case ktu::hash("ato"):
            outString();
            pushUnknownSection(ktu::numeric_literal("ATO1"));
            return;
        case ktu::hash("atr"):
            outString();
            pushUnknownSection(ktu::numeric_literal("ATR1"));
            return;
        case ktu::hash("lbl"):
            outString();
            labelSection.clear();
            labelSection.setNumberOfGroups((mut.valueParameters.empty()) ? 101 : mut.valueParameters[0]);
            sections.push_back(section(&labelSection, (void*)((bigEndian) ? &lbl1::addBuffer<true> : &lbl1::addBuffer<false>)));
            return;
        case ktu::hash("nli"):
            outString();
            pushUnknownSection(ktu::numeric_literal("NLI1"));
            return;
        case ktu::hash("tsy"): {
            outString();
            pushTsy1();
            return;
        }
            
        case ktu::hash("string"):
            mut.state &= ~(mutableData::whitespaceField);
            if (inString) {
                pushString();
            } else {
                inString = true;
                sections.push_back(section(&textSection, (void*)((bigEndian) ? &txt2::addBuffer<true> : &txt2::addBuffer<false>)));
            }
            if (mut.textParameters.empty()) return;
            labelSection.push(std::string(mut.textParameters[0].ptr, mut.textParameters[0].end));
            return;
            
        #include <msm/encode/gameSpecific.inl>

        default:
            if (!(inString)) break;
            inStringH();
            return;
    }
    addAllTextParameters();
}


void encode_t::getTextParameter() {
    const char *ptr = mut.ptr;
    read();
    int counter = 1;
    const char *end;
    for (;valid(counter += (mut.codepoint == '{') - (mut.codepoint == '}'));read());
    end = mut.ptr - 1;
    
    mut.textParameters.push_back({ptr, end});
}


void encode_t::getValueParameters() {
    
    if (valid()) goto no_read;
    while (valid() && mut.codepoint != ']') {
        read();
        no_read:
        if (isWhitespace()) {
            skipWhitespace();
            if (valid())
                goto no_read;
            return;
        }
        
        if (mut.codepoint == '0') {
            read();
            mut.valueParameters.push_back(
                (mut.codepoint == 'b' || mut.codepoint == 'B') ?
                    getBinaryValue() :
                    (mut.codepoint == 'x' || mut.codepoint == 'X') ?
                        getHexValue() :
                            ('0' <= mut.codepoint && mut.codepoint <= '9') ?
                                getOctalValue() : 0
            );
            for (;valid();read()) {
                if (mut.codepoint == ',') break;
                if (mut.codepoint == ']') return;
            }
            continue;
        }
        if ('1' && mut.codepoint && mut.codepoint <= '9') {
            mut.valueParameters.push_back(getDecimalValue());
            for (;valid();read()) {
                if (mut.codepoint == ',') break;
                if (mut.codepoint == ']') return;
            }
            continue;
        }
        if (isVariable()) {
            
            std::string var = getVariable();
            variables.count(var) ?
                mut.valueParameters.push_back(variables[var]) :
                mut.valueParameters.push_back(0);
            if (valid()) goto no_read2;
            while (valid()) {
                read();
                no_read2:
                if (mut.codepoint == ',') break;
                if (mut.codepoint == ']') return;
            }
            continue;
        }
    }
}


std::string encode_t::getVariable() {
    std::string str;
    for (;valid(isVariable());read())
        ktu::u8::push_back(str, mut.codepoint);
    if (!valid()) {
        if (isVariable()) ktu::u8::push_back(str, mut.codepoint);
        else append();
    }
    return str;
}


int encode_t::getBinaryValue() {
    int value = 0;
    while (valid((mut.codepoint == '0') || (mut.codepoint == '1'))) {
        read();
        value <<= 1;
        value |= mut.codepoint - '0';
    }
    return value;
}



int encode_t::getOctalValue() {
    int value = 0;
    if (valid('0' <= mut.codepoint && mut.codepoint <= '7')) goto no_read;
    while (valid('0' <= mut.codepoint && mut.codepoint <= '7')) {
        read();
        no_read:
        value <<= 3;
        value |= mut.codepoint - '0';
    }
    return value;
}

int encode_t::getDecimalValue() {
    int value = 0;
    for (;valid('0' <= mut.codepoint && mut.codepoint <= '9');read()) {
        value *= 10;
        value += mut.codepoint - '0';
    }
    return value;
}

int encode_t::getHexValue() {
    int value = 0;
    read();
    for (;valid(
        ('0' <= mut.codepoint && mut.codepoint <= '9') ||
        ('A' <= mut.codepoint && mut.codepoint <= 'F') ||
        ('a' <= mut.codepoint && mut.codepoint <= 'f')
    );read()) {
        value <<= 4;
        value |= mut.codepoint -
            '0' -
            ('A' <= mut.codepoint && mut.codepoint <= 'F')*0x7 -
            ('a' <= mut.codepoint && mut.codepoint <= 'f')*0x27;
    }
    return value;
}



void encode(const std::filesystem::path &input, const std::filesystem::path &output, const std::filesystem::path &external, title::id id) {
    ktu::buffer outputBuffer;
    encode_t enc(input, outputBuffer, external, id);
    std::filesystem::path outputPath {output};
    if (!outputPath.has_extension()) outputPath.replace_extension(enc.getExt());
    outputBuffer.writef(outputPath);
}
