#include <msm/width.hpp>


#include <fmt/format.h>


#include <filesystem>



void impl::width_base::_set(uint16_t size, const void *data) {
    this->size = size;
    if (!endOfRange) return;
    currentWidths = (const uint8_t*)endOfRange;
    const uint16_t *it = (const uint16_t*)data;
    while (true) {
        base = *it++;

        if (it != endOfRange && size < *it++)
            break;
        currentWidths += 0x10000;
    }
}









class width_file_parse {
    static uint16_t values[0x10000];
    public:
        width_file_parse(uint16_t base, uint16_t max) : base(base), max(max) {}
        bool is_valid(const std::filesystem::path &path) {
            fmt::print("[{}];\n", path.string());
            if (!buf.assign(path)) return false;
            reader = buf;
            switch (reader.read<uint32_t>()) {
                default:
                    return false;
                case ktu::numeric_literal("FFNT"):
                case ktu::numeric_literal("CFNT"):
                case ktu::numeric_literal("CFNU"):
                    break;
            }
            
            bigEndian = reader.read() < reader.read();

            uint16_t headerSize = reader.read_big_endian<uint16_t>(bigEndian);
            
            
            uint32_t version = reader.read_big_endian<uint32_t>(bigEndian);

            

            switch (version) {
                default:
                    return false;
                case 0x03000000:
                    reader.seek(headerSize+0x14);
                    break;
                case 0x04000000:
                    reader.seek(headerSize+0x18);
                    break;
            }

            return true;
        }

        void read(uint8_t *widths) {


            uint32_t
                cwdhOffset = reader.read_big_endian<uint32_t>(bigEndian),
                cmapOffset = reader.read_big_endian<uint32_t>(bigEndian);

            

            enum method : uint16_t{
                direct = 0,
                table = 1,
                scan = 2
            };
            while (cmapOffset) {
                reader.seek(cmapOffset);
                uint16_t
                    begin = reader.read_big_endian<uint16_t>(bigEndian),
                    end = reader.read_big_endian<uint16_t>(bigEndian),
                    method = reader.read_big_endian<uint16_t>(bigEndian);
                reader.seek(reader.cur()+2);
                cmapOffset = reader.read_big_endian<uint32_t>(bigEndian);
                
                switch (method) {
                    case method::direct: {
                        uint16_t indexValue = reader.read_big_endian<uint16_t>(bigEndian);
                        for (auto code = begin; code < end; code++) {
                            values[code - begin + indexValue] = code;
                        }
                        break;
                    }
                    case method::table:
                        for (auto code = begin; code <= end; code++) {
                            uint16_t indexValue = reader.read_big_endian<uint16_t>(bigEndian);
                            if (indexValue != 0xFFFF) {
                                values[indexValue] = code;
                            }
                        }
                        break;
                    case method::scan: {
                        uint16_t count = reader.read_big_endian<uint16_t>(bigEndian);
                        
                        for (int i = 0; i < count; i++) {
                            uint16_t
                                code = reader.read_big_endian<uint16_t>(bigEndian),
                                offset = reader.read_big_endian<uint16_t>(bigEndian);
                            values[offset] = code;
                        }
                        break;
                    }
                }
            }



            while (cwdhOffset) {
                reader.seek(cwdhOffset);
                uint16_t
                    start = reader.read_big_endian<uint16_t>(bigEndian),
                    end = reader.read_big_endian<uint16_t>(bigEndian);
                cwdhOffset = reader.read_big_endian<uint32_t>(bigEndian);

                for (auto i = start; i <= end; i++) {
                    uint8_t left = reader.read();
                    reader.seek(reader.cur()+1);

                    uint8_t _char = reader.read();
                    widths[values[i]] = left+_char;
                }
            }
        }
        uint16_t base;
        uint16_t max;
    private:
        
        ktu::buffer buf;
        ktu::reader reader;
        bool bigEndian;
};

uint16_t width_file_parse::values [0x10000] {};
#include <vector>


#include <vector>
#include <string>
#include <map>

#include <msm/argparse.hpp>
#include <ktu/string.hpp>

void parameters(const std::string &str, const std::vector<size_t> &indecies, std::vector<width_file_parse> &files) {
    if (indecies.empty())
        return;

    std::filesystem::path dir{str.data() + indecies[0]};
    
    int a = -1, b = -1;
    if (indecies.size() > 1)
        a = ktu::ston(str.data() + indecies[1]);
    if (indecies.size() > 2)
        b = ktu::ston(str.data() + indecies[2]);
    
    


    width_file_parse file(a, b);
    
    if (!file.is_valid(dir)) return;
    files.push_back(std::move(file));
}





void width::assign(const char *s, std::map<std::string, std::string> &vars) {
    std::vector<width_file_parse> files;
    std::string str;
    std::vector<size_t> indecies;
    if (*s == '"' || *s == '\'') ++s; // If the string is quoted, skip the quote char.
    size_t index = 0;
    while (true) {
        switch (*s) {
            case '"':
            case '\'':
                s = quote(str, s);
                break;
            case '#':
            case '@':
                s = macro(str, s, vars);
                break;
            case '\\':
                ++s;
                if (!*s) goto out;
                str += *s++;
                break;
            case ',':
                indecies.push_back(index);
                str += '\0';
                index = str.size();
                ++s;
                break;
            case ':':
                parameters(str, indecies, files);
                indecies.clear();
                index = 0;
                str.clear();
                ++s;
                break;
            case '\0':
                parameters(str, indecies, files);
                goto out;
            default:
                str += *s++;
                break;
        }
    }
    out:;
    if (files.empty()) return;
    size_t range_size = files.size() * (sizeof(uint16_t)+sizeof(uint16_t)), widths_size = 0x10000*files.size();

    data.resize(widths_size);
    uint16_t *range_offset = data.data<uint16_t>();
    uint8_t *offset = data.data() + range_size;
    endOfRange = (uint16_t*)offset;
    
    for (auto file : files) {
        
        *range_offset++ = file.base;
        *range_offset++ = file.max;
        file.read(offset);
        offset += 0x10000;
    }
    set(100);
}
