from __init__ import INCLUDE_DIR
from titleID import Games, CodeId
from cppgen.gen import *
import math





class Data:
    def __init__(self, name : str, data, *trailing):
        self.__name = name
        if type(data) == list:
            self.__data = data
        elif type(data) == CodeId:
            self.__data = data.list(*trailing) + [short for short in trailing]

    def name(self) -> str: return self.__name
    def data(self) -> list: return self.__data


class Access:
    def __init__(self, name, offset : int, size : int):
        self.__offset = offset
        self.__size = size
        self.__name = name
    
    def name(self): return self.__name
    def offset(self) -> int: return self.__offset
    def size(self) -> int: return self.__size

    def set_offset(self, offset : int): self.__offset = offset
    def set_size(self, size : int): self.__size = size

    def __repr__(self) -> str:
        return f"{self.name()}{{{self.offset()}, {self.size()}}}"

def main():

    ELLIPSES_DATA : list = [0x2E,0x2E,0x2E]



    datas : list[Data] = [
        Data("page", CodeId(0, 4)),
        Data("page_wait", CodeId(0, 0).list()+CodeId(0, 4).list()),
        Data("ruby", [0xE, 0x0, 0x0, 0x0, 0x0, 0x0]),
        Data("page_ellipses", ELLIPSES_DATA+CodeId(0, 4).list()+ELLIPSES_DATA),
        Data("page_wait_ellipses", ELLIPSES_DATA+CodeId(0, 0).list()+CodeId(0, 4).list()+ELLIPSES_DATA),
        Data("option_override", CodeId(0, 0)),
        Data("newline", [0xA])
    ]

    originalDatasLength = len(datas)
    for game in Games:
        for codename in game.codenames():
            if codename == "option":
                datas.append(Data([f"title::id::{game.name()}_{region.code()}" for region in game.regions()], game.codenames()[codename][0]))
        
    totalSize = sum([len(data.data()) for data in datas])
    

    max_access = Access("max", 0, 0)
    accesses = []
    current_index = 0
    for data in datas:
        match data.name():
            case "page_wait":
                PAGE_WAIT_OFFSET = current_index + 1
            case "page_wait_ellipses":
                PAGE_WAIT_ELLIPSES_OFFSET = current_index + len(ELLIPSES_DATA) + 1
            case "option_override":
                OPTION_OVERRIDE_OFFSET = current_index + 1
        accesses.append(Access(data.name(), current_index, len(data.data())))
        max_access.set_offset(max(max_access.offset(), current_index))
        max_access.set_size(max(max_access.size(), len(data.data())))
        
        current_index += len(data.data())
        
    offset_bitsize = math.ceil(math.log2(max_access.offset()))
    offset_type = f"uint{math.ceil(offset_bitsize/8)*8}_t"
    size_bitsize = math.ceil(math.log2(max_access.size()))
    size_type = f"uint{math.ceil(size_bitsize/8)*8}_t"

    g = Generator()
    g.line("#pragma once")
    g.line(Include(["<cstdint>","<cstddef>","<msm/titleID.hpp>"]))
    g.nest("namespace code_data_n")
    g.statement(f"constexpr size_t size = {totalSize}")
    g.statement(f"""static uint8_t data[size*4] {{{                                                            
        ', '.join(                                                                                          
            [f'0x{item&0xFF:02X}, 0x{(item&0xFF00) >> 8:02X}' for data in datas for item in data.data()]+     
            [f'0x{(item&0xFF00) >> 8:02X}, 0x{item&0xFF:02X}' for data in datas for item in data.data()]      
        )                                                                                                   
    }}}""")


    g.statement(
        "constexpr struct access_t {\n"   
        f"    {offset_type} offset : {offset_bitsize}; // 00'10_00'00\n"
        f"    {size_type} size : {size_bitsize};\n"
        "    friend inline bool operator==(access_t self, access_t other) {\n"
        "        return *(uint16_t*)&self == *(uint16_t*)&other;\n"
        "    };\n"
        "} " + ', '.join([str(access) for access in accesses[:originalDatasLength]])
    )
    g.line(
        "inline bool setWaitCode(title::id id) {\n"
        "    code_t result = title::getWaitGroupType(id);\n"
        "    if (!result) return false;\n"
        "    uint16_t *dataOffset = (uint16_t*)((void*)&data[0]);\n"
        "    uint16_t\n"
        "        groupLittleEndian = ktu::little_endian(result.group),\n"
        "        typeLittleEndian = ktu::little_endian(result.type),\n"
        "        groupBigEndian = ktu::big_endian(result.group),\n"
        "        typeBigEndian = ktu::big_endian(result.type);\n"
        f"    dataOffset[{PAGE_WAIT_OFFSET}] = groupLittleEndian;\n"
        f"    dataOffset[{PAGE_WAIT_OFFSET+1}] = typeLittleEndian;\n"
        f"    dataOffset[{PAGE_WAIT_ELLIPSES_OFFSET}] = groupLittleEndian;\n"
        f"    dataOffset[{PAGE_WAIT_ELLIPSES_OFFSET+1}] = typeLittleEndian;\n"
        f"    dataOffset[size+{PAGE_WAIT_OFFSET}] = groupBigEndian;\n"
        f"    dataOffset[size+{PAGE_WAIT_OFFSET+1}] = typeBigEndian;\n"
        f"    dataOffset[size+{PAGE_WAIT_ELLIPSES_OFFSET}] = groupBigEndian;\n"
        f"    dataOffset[size+{PAGE_WAIT_ELLIPSES_OFFSET+1}] = typeBigEndian;\n"
        "    return true;\n"
        "};\n"
        "inline void setOptionOverrideCode(code_t code) {\n"
        "    uint16_t *dataOffset = (uint16_t*)((void*)&data[0]);\n"
        f"    dataOffset[{OPTION_OVERRIDE_OFFSET}] = ktu::little_endian(code.group);\n"
        f"    dataOffset[{OPTION_OVERRIDE_OFFSET+1}] = ktu::little_endian(code.type);\n"
        f"    dataOffset[size+{OPTION_OVERRIDE_OFFSET}] = ktu::big_endian(code.group);\n"
        f"    dataOffset[size+{OPTION_OVERRIDE_OFFSET+1}] = ktu::big_endian(code.type);\n"
        "};\n"
        "const uint8_t *begin(bool bigEndian, access_t access) {\n"
        "    return &data[((size_t)(access.offset) + bigEndian * size) * 2];\n"
        "};\n"
        "const uint8_t *end(bool bigEndian, access_t access) {\n"
        "    return &data[(((size_t)access.offset+(size_t)access.size) + bigEndian * size) * 2];\n"
        "};\n"
        "inline void push(txt2 &textSection, bool bigEndian, access_t access) {\n"
        "    textSection.insert(textSection.endp(), begin(bigEndian, access), end(bigEndian, access));\n"
        "};\n" +
        Function([], "inline auto", "option", [("bool", "bigEndian"), ("title::id", "id")],
            "access_t access {0, 0};\n"
            "if (title::codeOverride.option()) {\n"
            "    access = option_override;\n"
            "} else {\n    "+
                Switch("id", [(access.name(), f"access = {{{access.offset()}, {access.size()}}};\nbreak;") for access in accesses[originalDatasLength:]], "break;").replace("\n","\n    ")+"\n}\n"
            "struct {\n"
            "    const uint8_t *data;\n"
            "    uint16_t size;\n"
            "} return_value {\n"
            "    .data = (const uint8_t *)begin(bigEndian, access)\n"
            "};\n"
            "return_value.size = (const uint8_t *)end(bigEndian, access) - return_value.data;\n"
            "return return_value;"
        )
    )
    g.exitnln()
    



    g.write(f"{INCLUDE_DIR}/encode/codeData.hpp")