#pragma once


template <bool big_endian>
void encode_t::codeVariadicParamsf(uint16_t group, uint16_t type) {
    textSection.codeVariadicParams<big_endian>(mut.valueParameters, group, type);
    addAllTextParameters();
}

template <bool big_endian>
void txt2::codeVariadicParams(std::vector<uint32_t> &valueParameters, uint16_t group, uint16_t type) {
    code<big_endian>(group, type);
    stringData.push_back_big_endian<big_endian, uint16_t>(valueParameters.size()*2);
    for (auto val : valueParameters) {
        stringData.push_back_big_endian<big_endian, uint16_t>(val);
    }
}


template <bool big_endian>
void encode_t::genericf() {
    if (mut.valueParameters.empty()) {
        textSection.code<big_endian>(0, 0);
        textSection.push_short<big_endian>(0);
        addAllTextParameters();
        return;
    }
    if (mut.valueParameters.size() < 2) {
        textSection.code<big_endian>(mut.valueParameters[0], 0);
        textSection.push_short<big_endian>(0);
        addAllTextParameters();
        return;
    }
    textSection.code<big_endian>(mut.valueParameters[0], mut.valueParameters[1]);
    textSection.push_short<big_endian>((mut.valueParameters.size()-2)*2);
    for (auto it = mut.valueParameters.begin() + 2 ; it != mut.valueParameters.end(); it++) {
        textSection.push_short<big_endian>(*it);
    }
    addAllTextParameters();
}

template <bool big_endian>
void encode_t::rubyf() {
    textSection.code<big_endian>(0, 0);
    if (mut.textParameters.empty()) {
        textSection.push_short<big_endian>(0);
        return;
    }
    size_t bufSize = textSection.bytesize() + 2;
    textSection.resize(textSection.size() + 3);
    
    size_t index = textSection.size()-3;
    uint16_t rtSize, totalSize;
    if (mut.textParameters.size() < 2) {
        push(mut.textParameters[0].ptr, mut.textParameters[0].end);
        uint16_t *ptr = textSection.offset(index);
        totalSize = textSection.bytesize() - bufSize;
        *ptr++ = ktu::big_endian<big_endian, uint16_t>(totalSize);
        *ptr++ = 0;
        *ptr = ktu::big_endian<big_endian, uint16_t>(totalSize - 4);
        return;
    }
    
    push(mut.textParameters[0].ptr, mut.textParameters[0].end);
    rtSize = textSection.bytesize() - bufSize - 4;
    push(mut.textParameters[1].ptr, mut.textParameters[1].end);
    totalSize = textSection.bytesize() - bufSize;
    uint16_t *ptr = textSection.offset(index);
    *ptr++ = ktu::big_endian<big_endian, uint16_t>(totalSize);
    *ptr++ = ktu::big_endian<big_endian, uint16_t>((totalSize - 4) - rtSize);
    *ptr = ktu::big_endian<big_endian, uint16_t>(rtSize);

    for (auto it = mut.textParameters.begin() + 2; it != mut.textParameters.end(); it++)
        push(it->ptr, it->end);
}



template <bool big_endian>
void encode_t::colorf() {
    textSection.code<big_endian>(0, 3);
    uint8_t rgba[4] = {0x00,0x00,0x00,(mut.valueParameters.size())?(uint8_t)0x00:(uint8_t)0xFF};
    textSection.push_short<big_endian>(4);
    for (int i = 0; i < 4 && i < mut.valueParameters.size(); i++) {
        rgba[i] = mut.valueParameters[i];
    }
    textSection.push_array(rgba);
    if (mut.textParameters.empty())
        return;
    push(mut.textParameters[0].ptr, mut.textParameters[0].end);
    textSection.code<big_endian>(0, 3);
    textSection.push_short<big_endian>(4);
    uint8_t defaultValue[4] = {0x00,0x00,0x00,0xFF};
    textSection.push_array(defaultValue);
    for (auto it = mut.textParameters.begin() + 1; it != mut.textParameters.end(); it++)
        push(it->ptr, it->end);
}