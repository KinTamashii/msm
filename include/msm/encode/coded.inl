#pragma once



inline bool valid() {
    return mut.ptr < mut.end;
}
inline bool valid(bool condition) {
    return valid() && condition;
}
inline void read() {
    mut.codepoint = ktu::u8::read(&mut.ptr);   
}
inline void append(uint32_t codepoint) {
    
    (textSection.*pushCodepointPtr)(codepoint);
}
inline void append() {
    append(mut.codepoint);
}



inline bool isWhitespace() {
    return ::isWhitespace(mut.codepoint);
}

inline void skipWhitespace(bool override = false) {
    uint32_t lastCodepoint = mut.codepoint;
    for (;valid(isWhitespace());read()) lastCodepoint = mut.codepoint;
    if (override || mutableData::hasParameters) {
        setLastWs(lastCodepoint);
    }
}

inline void setLastWs(uint32_t codepoint) {
    prevWsCodepoint = lastWsCodepoint;
    lastWsCodepoint = codepoint;
}



inline void skipWhitespaceAdd() {
    skipWhitespace(true);
    if (mut.codepoint == '\\') {
        mut.state |= mutableData::queueWhitespace | mutableData::usePrevWs;
        return;
    }
    bool isWs = isWhitespace();

    uint32_t queuedCodepoint = mut.state & mutableData::usePrevWs ? prevWsCodepoint : lastWsCodepoint;
    
    queuedCodepoint = queuedCodepoint == '\n' ? ' ' : queuedCodepoint;
    
    if ((isWs && valid()) || (!isWs)) {append(queuedCodepoint);}
    
    
}

inline bool isVariable() {
    return ('A' <= mut.codepoint && mut.codepoint <= 'Z') || ('a' <= mut.codepoint && mut.codepoint <= 'z') || (mut.codepoint == '_');
}



inline void inStringH(){
    if (((mut.state & mutableData::queueWhitespace))) {
        uint32_t queuedCodepoint = mut.state & mutableData::whitespace && mut.state & mutableData::usePrevWs ? prevWsCodepoint : lastWsCodepoint;
        queuedCodepoint = queuedCodepoint == '\n' ? ' ' : queuedCodepoint;
        append(queuedCodepoint);
        mut.state &= ~(mutableData::queueWhitespace);
    }
};


inline void addAllTextParameters() {
    for (auto &txt : mut.textParameters)
        push(txt.ptr, txt.end);
}

inline void size() {
    codeVariadicParams(0, 2);
}
inline void pagebreak() {
    codeVariadicParams(0, 4);
}