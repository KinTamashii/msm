from __init__ import INCLUDE_DIR
from titleID import Games
from cppgen.gen import *
"""case ktu::hash("wait"):
    if (!(inString)) break;
    inStringH();
    if (!(title::is::ML4(gameID)))
        break;
    codeVariadicParams(3, 1);
    return;
case ktu::hash("option"):
    if (!(inString)) break;
    inStringH();
    if (!(title::is::ML4(gameID)))
        break;
    codeVariadicParams(5, 1);
    return;"""


def main():
    g = Generator()
    for codename in set({codename for game in Games for codename in game.codenames()}):
        g.line(f"case ktu::hash(\"{codename}\"):")
        g.indent()
        cases = []
        for game in Games:
            if codename in game.codenames():
                cases.append(
                    [
                        [f"title::id::{game.name()}_{region.code()}" for region in game.regions()],
                        f"codeVariadicParams({game.codenames()[codename][0].group}, {game.codenames()[codename][0].type});\nreturn;"
                    ]
                )
        g.line("if (!(inString)) break;\ninStringH();\n" + Switch(match="gameID", cases=cases, default_body="break;") + "\nbreak;")
        g.unindent()
            

    
    #print(g)
    g.write(f"{INCLUDE_DIR}/encode/gameSpecific.inl")