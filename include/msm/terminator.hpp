#pragma once
struct terminator {
    enum {
        endOfPage = 0x40,
        endOfString,
        rubyPush,
        rubyNext,
        rubyPop,
        option
    };
};