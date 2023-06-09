from __init__ import INCLUDE_DIR, SOURCE_DIR
from cppgen.gen import *




class Region:
    def __init__(self, code : str = "", id : int = ""):
        self.__code = code
        self.__id = id

    def code(self):
        return self.__code
    def id(self):
        return self.__id


class CodeId:
    def __init__(self, group : int, type : int):
        self.group = group & 0xFFFF
        self.type = type & 0xFFFF

    def __repr__(self) -> str:
        return f"(group={self.group}, type={self.type})"
    
    def list(self, *trailing) -> list:
        return [0xE, self.group, self.type, len(trailing)*2] + [short for short in trailing]

class Code(CodeId):
    def __init__(self, name : str, group : int, type : int):
        super().__init__(group, type)
        self.name = name
    def __repr__(self):
        return f"{self.name}={super().__repr__()}"

class Game:
    def __init__(self, name : str = "", regions : list[Region] = [], codes : list[Code] = []):
        self.__name = name
        self.__regions = regions
        self.__codeids = {}
        self.__codenames = {}
        for code in codes:
            if (code.group not in self.__codeids):
                self.__codeids[code.group] = {code.type : code.name}
            else:
                self.__codeids[code.group][code.type] = code.name
            if code.name not in self.__codenames:
                self.__codenames[code.name] = [CodeId(code.group, code.type)]
            else:
                self.__codenames[code.name].append(CodeId(code.group, code.type))
    def name(self):
        return self.__name
    def regions(self):
        return self.__regions
    
    def codeids(self):
        return self.__codeids
    def codenames(self):
        return self.__codenames

Games : list[Game] = [
    Game(
        "ML4",
        [Region("US", 0x00040000000D5A00), Region("EU", 0x00040000000D9000), Region("JP", 0x0004000000060600)],
        [Code("wait", 3, 1), Code("option", 5, 1)]
    ),
    Game(
        "ML5",
        [Region("US", 0x0004000000132700), Region("EU", 0x0004000000132800), Region("JP", 0x0004000000132600)],
        [Code("wait", 2, 1), Code("option", 4, 0), Code("option", 4, 1), Code("hspace", 6, 0), Code("hset", 6, 1)]
    ),
    Game(
        "ML1_REMAKE",
        [Region("US", 0x00040000001B8F00), Region("EU", 0x00040000001B9000), Region("JP", 0x0004000000194B00)],
        [Code("wait", 2, 1), Code("option", 4, 0), Code("option", 4, 1), Code("hspace", 6, 0), Code("hset", 6, 1)]
    ),
    Game(
        "ML3_REMAKE",
        [Region("US", 0x00040000001D1400), Region("EU", 0x0004000E001D1500), Region("JP", 0x00040000001CA900), Region("JP_VER_1_2", 0x0004000E001CA900)],
        [Code("wait", 2, 1), Code("option", 4, 0), Code("option", 4, 1), Code("hspace", 6, 0), Code("hset", 6, 1)]
    )
]

class CodeAssociation:
    def __init__(self, game_ids : list[str], values : dict[int, list[int]]):
        self.game_ids : list[str] = game_ids
        self.values : dict[int, list[int]] = values

    def __repr__(self) -> str:
        return f"({self.game_ids} => {self.values})"

def boolsum(L : list):
    result = True
    for item in L:
        result = result and item
    return result



def __local_compare(X, Y):
    first = set(X.keys())
    second = set(Y.keys())
    if first != second:
        return False

    for group in first:
        if set(X[group]) != set(Y[group]):
            return False
    return True

code_associations : dict[str, CodeAssociation] = {codename : [] for game in Games for codename in game.codenames().keys()}
for code in code_associations:
    
    for game in Games:
        new_association = CodeAssociation(
            [f"id::{game.name()}_{region.code()}" for region in game.regions()],
            {group : [type for type in game.codeids()[group].keys() if game.codeids()[group][type] == code] for group in game.codeids() if code in [game.codeids()[group][type] for type in game.codeids()[group]]}
        )
        if not len(new_association.values):
            continue
        for code_association in code_associations[code]:
            if __local_compare(code_association.values, new_association.values):
                code_association.game_ids += new_association.game_ids
                break
        else:
            code_associations[code].append(new_association)
    
    




def ID_Enum():
    return Enum(
        name="id",
        values=[("null", "0")]+[
            (f"{game.name()}_{region.code()}", f"0x{region.id():016X}") for game in Games for region in game.regions()
        ],
        inherited="uint64_t"
    )+'\n'

def IS_Struct():
    return Struct("is", '\n'.join(
            [Function(
                template_args=[],
                return_type="static inline constexpr bool",
                name=game.name(),
                args=[("title::id", "id")],
                body="return "+" || ".join(
                    [f"id == {game.name()}_{region.code()}" for region in game.regions()]
                )+';'
            ) for game in Games]
        ))+"\n"

def Get_Function():
    return Function(
        template_args=[],
        return_type="static constexpr id",
        name="get",
        args=[("const char*", "str")],
        body=   "char c = *str;\n"
                "if (('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f'))\n"
                "    return (title::id)ktu::ston64(str);\n\n"
                + Switch(
                    match="ktu::hash(str)",
                    cases=suml(
                        [
                            [[[f"ktu::hash(\"{game.name()}\")"], f"return id::{game.name()}_US;"]]+
                            [
                                [[f"ktu::hash(\"{game.name()}_{region.code()}\")"], f"return id::{game.name()}_{region.code()};"] for region in game.regions()
                            ] for game in Games
                        ]
                    ),
                    default_body="return id::null;"
                )
    )+'\n'

def Valid_Function():
    return Function(
        template_args=[],
        return_type="static inline constexpr bool",
        name="valid",
        args=[("id","id")],
        body=   Switch(match="id", cases=[
            [
                [f"id::{game.name()}_{region.code()}" for game in Games for region in game.regions()], "return true;"
            ]
        ], default_body="return false;")
    )+'\n'

def Select_Function():
    
    for game in Games:
        [type for group in game.codeids() for type in game.codeids()[group]]
    return Function(
        template_args=[("bool", "big_endian")],
        return_type=f"static bool",
        name=f"select",
        args=[("id", "id"), ("code_t", "code"), ("std::string&", "str"), ("ktu::reader&", "reader"), ("std::stack<ruby_t>&", "rubies")],
        body = "const char *identifier = codeOverride[code];\n"
        "if (!identifier) {\n    "+
        Switch(
            match="id",
            cases=[
                [
                    [f"id::{game.name()}_{region.code()}" for region in game.regions()],
                    Switch(match="code", cases=[

                        [
                            [f"code_t({group}, {type})"],
                            f"identifier = \"{game.codeids()[group][type]}\";\nbreak;"
                        ] for group in game.codeids() for type in game.codeids()[group]
                    ])+"\nbreak;"
                ] for game in Games
            ],
            default_body="break;"
        ).replace("\n","\n    ") + "\n}\nif (!identifier) return false;\n\n"
            "uint16_t size = reader.read_big_endian<big_endian, uint16_t>() / 2;\n"
            "str += '\\\\';\n"
            "str += identifier;\n"
            "variadicCodeParameters<big_endian, false>(str, \" \", size, reader);\n"
            "return true;"

    )


def Specific_Function(name : str, isDeclaration : bool = False):
    return Function(
        template_args=[],
        return_type=f"{'static '*(isDeclaration)}bool",
        name=f"{'title::'*(not isDeclaration)}{name}",
        args=[("id", "id"), ("code_t", "code")],
        body = "" if isDeclaration else
        "const char *codename = codeOverride[code];\n"
        f"if (codename && codename[2] == '{name[2]}')\n"
        "    return true;"
        "\n"+Switch(
            match="id",
            cases=[
                [
                    association.game_ids, 
                    Switch(
                        match="code",
                        cases=[
                            (
                                [f"code_t({group}, {type})" for type in association.values[group]],
                                "return true;"
                            ) for group in association.values
                        ]
                    )
                ] for association in code_associations[name]
            ],
            default_body="break;"
        ) + "\nreturn false;"
    )



def GetWaitGroupType_Function(isDeclaration : bool = False):
    return Function(
        template_args=[],
        return_type=f"{'static '*(isDeclaration)}code_t",
        name=f"{'title::'*(not isDeclaration)}getWaitGroupType",
        args=[("id", "id")],
        body = "" if isDeclaration else
            "code_t code = codeOverride.wait();\n"
            "if (code) return code;\n"
            +Switch(
                match="id",
                cases=[
                    [
                        [f"id::{game.name()}_{region.code()}" for region in game.regions()],
                        [f"code = code_t({group},{type});\nbreak;" for group in game.codeids() for type in game.codeids()[group] if game.codeids()[group][type] == "wait"][0]
                    ] for game in Games
                ],
                default_body="code = code_t(0,0);\nbreak;"
            )+
            "\nreturn code;"
    )




def main():
    

    g = Generator()

    g.line("#pragma once")

    g.indented(Include(["<ktu/string.hpp>","<ktu/algorithm.hpp>","<msm/decode/codes.hpp>","<msm/common.hpp>","<map>"]))

    g.line(Struct("title", ID_Enum()+IS_Struct()+Get_Function()+Valid_Function()+
    "private:\n"
    "    class codeOverride_t {\n"
    "        private:\n"
    "            struct indecies {enum {hset, hspace, option, wait};};\n"
    "        public:\n"
    "            void set(char *codeName, char *postAssign);\n"
    "            KTU_INLINE code_t hset()    const {return codes[indecies::hset];}\n"
    "            KTU_INLINE code_t hspace()  const {return codes[indecies::hspace];}\n"
    "            KTU_INLINE code_t option()  const {return codes[indecies::option];}\n"
    "            KTU_INLINE code_t wait()    const {return codes[indecies::wait];}\n"
    "            KTU_INLINE const char *operator[](code_t code) const {\n"
    "                auto it = codeOverrideMap.find(code);\n"
    "                if (it == codeOverrideMap.end())\n"
    "                    return nullptr;\n"
    "                return it->second;\n"
    "            }\n"
    "        private:\n"
    "            std::map<code_t, const char*> codeOverrideMap;\n"
    "            code_t codes[4] {{0,0},{0,0},{0,0},{0,0}};\n"
    "    };\n"
    "public:\n"
    "    static codeOverride_t codeOverride;\n"
    """static constexpr id find(const char *path) {
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
"""+Select_Function()+'\n'+
    Specific_Function("option", True)+
    Specific_Function("hspace", True)+
    Specific_Function("hset", True)+
    GetWaitGroupType_Function(True)))



    g.write(f"{INCLUDE_DIR}/titleID.hpp")


    g = Generator()
    g.indented(Include(["<msm/titleID.hpp>","<msm/argparse.hpp>"]))
    
    g.line(
        "\n"
        "title::codeOverride_t title::codeOverride;\n"
        "void title::codeOverride_t::set(char *codeName, char *postAssign) {\n"
        "    \n"
        "    int index;\n"
        "    switch (codeName[2]) {\n"
        "        case 'e':\n"
        "            index = indecies::hset;\n"
        "            break;\n"
        "        case 'p':\n"
        "            index = indecies::hspace;\n"
        "            break;\n"
        "        case 't':\n"
        "            index = indecies::option;\n"
        "            break;\n"
        "        case 'i':\n"
        "            index = indecies::wait;\n"
        "            break;\n"
        "        default:\n"
        "            return;\n"
        "    }\n"
        "    char *curParameter = postAssign;\n"
        "    multiSplitParameter_t next {.terminator = ','};\n"
        "    bool codeUnsetFlag = true;\n"
        "    while (next.terminator) {\n"
        "        code_t code (0,0);\n"
        "        next = multiSplitArgNextParameter(curParameter);\n"
        "        unsigned long mstou_result = mstou(curParameter);\n"
        "        if (mstou_result == -1) {\n"
        "            goto clear_jmp;\n"
        "        }\n"
        "        code.group = mstou_result;\n"
        "        curParameter = next.parameter;\n"
        "        if (next.terminator == ',') {\n"
        "            next = multiSplitArgNextParameter(curParameter);\n"
        "            mstou_result = mstou(curParameter);\n"
        "            if (mstou_result == -1) {\n"
        "                clear_jmp:\n"
        "                codeUnsetFlag = true;\n"
        "                codes[index] = code_t(0,0);\n"
        "                codeOverrideMap.clear();\n"
        "                curParameter = next.parameter;\n"
        "                while (next.terminator == ',')\n"
        "                    next = multiSplitArgNextParameter(curParameter = next.parameter);\n"
        "                curParameter = next.parameter;\n"
        "                continue;\n"
        "            }\n"
        "            code.type = mstou_result;\n"
        "            curParameter = next.parameter;\n"
        "            while (next.terminator == ',')\n"
        "                next = multiSplitArgNextParameter(curParameter = next.parameter);\n"
        "            curParameter = next.parameter;\n"
        "        }\n"
        "        \n"
        "        if (codeUnsetFlag) {\n"
        "            codes[index] = code;\n"
        "            codeUnsetFlag = false;\n"
        "        }\n"
        "        codeOverrideMap[code] = codeName;\n"
        "    }\n"
        "\n"
        "}\n"+
        Specific_Function("option")+'\n'+
        Specific_Function("hspace")+'\n'+
        Specific_Function("hset")+'\n'+
        GetWaitGroupType_Function()
    )


    #print(g)

    g.write(f"{SOURCE_DIR}/titleID.cpp")