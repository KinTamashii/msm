#include <msm/titleID.hpp>
#include <msm/argparse.hpp>

title::codeOverride_t title::codeOverride;
void title::codeOverride_t::set(char *codeName, char *postAssign) {
    
    int index;
    switch (codeName[2]) {
        case 'e':
            index = indecies::hset;
            break;
        case 'p':
            index = indecies::hspace;
            break;
        case 't':
            index = indecies::option;
            break;
        case 'i':
            index = indecies::wait;
            break;
        default:
            return;
    }
    char *curParameter = postAssign;
    multiSplitParameter_t next {.terminator = ','};
    bool codeUnsetFlag = true;
    while (next.terminator) {
        code_t code (0,0);
        next = multiSplitArgNextParameter(curParameter);
        unsigned long mstou_result = mstou(curParameter);
        if (mstou_result == -1) {
            goto clear_jmp;
        }
        code.group = mstou_result;
        curParameter = next.parameter;
        if (next.terminator == ',') {
            next = multiSplitArgNextParameter(curParameter);
            mstou_result = mstou(curParameter);
            if (mstou_result == -1) {
                clear_jmp:
                codeUnsetFlag = true;
                codes[index] = code_t(0,0);
                codeOverrideMap.clear();
                curParameter = next.parameter;
                while (next.terminator == ',')
                    next = multiSplitArgNextParameter(curParameter = next.parameter);
                curParameter = next.parameter;
                continue;
            }
            code.type = mstou_result;
            curParameter = next.parameter;
            while (next.terminator == ',')
                next = multiSplitArgNextParameter(curParameter = next.parameter);
            curParameter = next.parameter;
        }
        
        if (codeUnsetFlag) {
            codes[index] = code;
            codeUnsetFlag = false;
        }
        codeOverrideMap[code] = codeName;
    }

}
bool title::option (id id, code_t code) {
    const char *codename = codeOverride[code];
    if (codename && codename[2] == 't')
        return true;
    switch (id) {
        case id::ML4_US:
        case id::ML4_EU:
        case id::ML4_JP:
            switch (code) {
                case code_t(5, 1):
                    return true;
            };
        case id::ML5_US:
        case id::ML5_EU:
        case id::ML5_JP:
        case id::ML1_REMAKE_US:
        case id::ML1_REMAKE_EU:
        case id::ML1_REMAKE_JP:
        case id::ML3_REMAKE_US:
        case id::ML3_REMAKE_EU:
        case id::ML3_REMAKE_JP:
        case id::ML3_REMAKE_JP_VER_1_2:
            switch (code) {
                case code_t(4, 0):
                case code_t(4, 1):
                    return true;
            };
        default:
            break;
    };
    return false;
};
bool title::hspace (id id, code_t code) {
    const char *codename = codeOverride[code];
    if (codename && codename[2] == 'p')
        return true;
    switch (id) {
        case id::ML5_US:
        case id::ML5_EU:
        case id::ML5_JP:
        case id::ML1_REMAKE_US:
        case id::ML1_REMAKE_EU:
        case id::ML1_REMAKE_JP:
        case id::ML3_REMAKE_US:
        case id::ML3_REMAKE_EU:
        case id::ML3_REMAKE_JP:
        case id::ML3_REMAKE_JP_VER_1_2:
            switch (code) {
                case code_t(6, 0):
                    return true;
            };
        default:
            break;
    };
    return false;
};
bool title::hset (id id, code_t code) {
    const char *codename = codeOverride[code];
    if (codename && codename[2] == 'e')
        return true;
    switch (id) {
        case id::ML5_US:
        case id::ML5_EU:
        case id::ML5_JP:
        case id::ML1_REMAKE_US:
        case id::ML1_REMAKE_EU:
        case id::ML1_REMAKE_JP:
        case id::ML3_REMAKE_US:
        case id::ML3_REMAKE_EU:
        case id::ML3_REMAKE_JP:
        case id::ML3_REMAKE_JP_VER_1_2:
            switch (code) {
                case code_t(6, 1):
                    return true;
            };
        default:
            break;
    };
    return false;
};
code_t title::getWaitGroupType (id id) {
    code_t code = codeOverride.wait();
    if (code) return code;
    switch (id) {
        case id::ML4_US:
        case id::ML4_EU:
        case id::ML4_JP:
            code = code_t(3,1);
            break;
        case id::ML5_US:
        case id::ML5_EU:
        case id::ML5_JP:
        case id::ML1_REMAKE_US:
        case id::ML1_REMAKE_EU:
        case id::ML1_REMAKE_JP:
        case id::ML3_REMAKE_US:
        case id::ML3_REMAKE_EU:
        case id::ML3_REMAKE_JP:
        case id::ML3_REMAKE_JP_VER_1_2:
            code = code_t(2,1);
            break;
        default:
            code = code_t(0,0);
            break;
    };
    return code;
};
