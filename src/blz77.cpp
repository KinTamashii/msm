#include <msm/blz77.hpp>
#include <ktu/bit.hpp>


static const int s_compressWorkSize = (4098 + 4098 + 256 + 256) * sizeof(int16_t);

struct ScompressInfo {
	uint16_t WindowPos;
	uint16_t WindowLen;
	int16_t* OffsetTable;
	int16_t* ReversedOffsetTable;
	int16_t* ByteTable;
	int16_t* EndTable;
};

void initTable(ScompressInfo* info, void* work);
int search(ScompressInfo* info, const uint8_t* a_src, int& a_offset, int a_maxSize);
inline void slide(ScompressInfo* info, const uint8_t* a_src, int a_size);
void slideByte(ScompressInfo* info, const uint8_t* a_src);

bool uncompress(const uint8_t* a_compressed, const uint32_t a_compressedSize, ktu::buffer &uncompressed, size_t position) {

	
	if (a_compressedSize < sizeof(CompFooter)) {
		return false;
	}

	const CompFooter* compFooter = reinterpret_cast<const CompFooter*>(a_compressed + a_compressedSize - sizeof(CompFooter));
	uint32_t top = compFooter->bufferTopAndBottom & 0xFFFFFF;
	uint32_t bottom = compFooter->bufferTopAndBottom >> 24 & 0xFF;
	if (
		bottom < sizeof(CompFooter) 		||
		bottom > sizeof(CompFooter) + 3 	||
		top < bottom 					||
		top > a_compressedSize
	) {
		return false;
	}
	
	uncompressed.shift(position, a_compressedSize + compFooter->originalBottom);
	
	uint8_t *a_uncompressed = uncompressed.data();

	memcpy(a_uncompressed, a_compressed, a_compressedSize);
	uint8_t* dest = a_uncompressed + uncompressed.size();
	uint8_t* src = a_uncompressed + a_compressedSize - bottom;
	uint8_t* pEnd = a_uncompressed + a_compressedSize - top;
	while (src - pEnd > 0) {
		uint8_t flag = *--src;
		for (int i = 0; i < 8; i++) {
			if ((flag << i & 0x80) == 0) {
				if (dest - pEnd < 1 || src - pEnd < 1) {
					return false;
				}
				*--dest = *--src;
			} else {
				if (src - pEnd < 2) {
					return false;
				}
				int size = *--src;
				int offset = (((size & 0x0F) << 8) | *--src) + 3;
				size = (size >> 4 & 0x0F) + 3;
				if (size > dest - pEnd) {
					return false;
				}
				uint8_t* pData = dest + offset;
				if (pData > a_uncompressed + uncompressed.size()) {
					return false;
				}
				for (int j = 0; j < size; j++) {
					*--dest = *--pData;
				}
			}
			if (src - pEnd <= 0) {
				break;
			}
		}
	}
	return true;
}


bool compress(const uint8_t* a_uncompressed, const uint32_t a_uncompressedSize, ktu::buffer &compressed, size_t position) {
	
	//compressed.resize(a_uncompressedSize);
	compressed.shift(position, a_uncompressedSize);
	uint8_t *a_compressed = compressed.data() + position;
	uint32_t a_compressedSize = a_uncompressedSize;
	// memset(a_compressed, 0, a_compressedSize);
	const uint32_t a_compressedSizeOriginal = a_compressedSize;



	
	if (a_uncompressedSize < sizeof(CompFooter) || a_compressedSize < a_uncompressedSize) {
		return false;
	}
	uint8_t* work = new uint8_t[s_compressWorkSize];
	
	ScompressInfo info;
	initTable(&info, work);
	const int maxSize = 0xF + 3;
	const uint8_t* src = a_uncompressed + a_uncompressedSize;
	uint8_t* dest = a_compressed + a_uncompressedSize;
	while (src - a_uncompressed > 0 && dest - a_compressed > 0) {
		uint8_t* flag = --dest;
		*flag = 0;
		for (int i = 0; i < 8 && src - a_uncompressed > 0; i++) {
			int offset = 0;
			int size = search(&info, src, offset, static_cast<int>(std::min<int64_t>(std::min<int64_t>(maxSize, src - a_uncompressed), a_uncompressed + a_uncompressedSize - src)));
			if (size < 3) {
				if (dest - a_compressed < 1) {
					return false;
				}
				slide(&info, src, 1);
				*--dest = *--src;
				continue;
			}


			if (dest - a_compressed < 2) {
				return false;
			}
			*flag |= 0x80 >> i;
			slide(&info, src, size);
			src -= size;
			size -= 3;
			*--dest = (size << 4 & 0xF0) | ((offset - 3) >> 8 & 0x0F);
			*--dest = (offset - 3) & 0xFF;
			
		}
	}
	a_compressedSize = static_cast<uint32_t>(a_compressed + a_uncompressedSize - dest);
	
	delete[] work;
	
	uint32_t origSize = a_uncompressedSize;
	uint8_t* compressBuffer = a_compressed + a_uncompressedSize - a_compressedSize;
	uint32_t compressBufferSize = a_compressedSize;
	uint32_t origSafe = 0;
	uint32_t compressSafe = 0;
	
	while (origSize > 0) {
		uint8_t flag = compressBuffer[--compressBufferSize];
		for (int i = 0; i < 8 && origSize > 0; i++) {
			if ((flag << i & 0x80) == 0) {
				compressBufferSize--;
				origSize--;
				continue;
			}
			
			
			int size = (compressBuffer[--compressBufferSize] >> 4 & 0x0F) + 3;
			compressBufferSize--;
			origSize -= size;
			if (origSize < compressBufferSize) {
				origSafe = origSize;
				compressSafe = compressBufferSize;
				goto loop_over;
			}
			
		}
	}
	loop_over:
	uint32_t compressedSize = a_compressedSize - compressSafe;
	uint32_t padOffset = origSafe + compressedSize;
	uint32_t compFooterOffset = ktu::align2<uint32_t>(padOffset, 2);
	a_compressedSize = compFooterOffset + sizeof(CompFooter);
	uint32_t top = a_compressedSize - origSafe;
	uint32_t bottom = a_compressedSize - padOffset;
	if (a_compressedSize >= a_uncompressedSize || top > 0xFFFFFF) {
		return false;
	}
	memcpy(a_compressed, a_uncompressed, origSafe);
	memmove(a_compressed + origSafe, compressBuffer + compressSafe, compressedSize);
	memset(a_compressed + padOffset, 0xFF, compFooterOffset - padOffset);
	CompFooter* compFooter = reinterpret_cast<CompFooter*>(a_compressed + compFooterOffset);
	compFooter->bufferTopAndBottom = top | (bottom << 24);
	compFooter->originalBottom = a_uncompressedSize - a_compressedSize;
	
	
	//compressed.resize(a_compressedSize);
	compressed.erase(position+a_compressedSize, a_compressedSizeOriginal-a_compressedSize);
	return true;
}


void initTable(ScompressInfo* info, void* work) {
	info->WindowPos = 0;
	info->WindowLen = 0;
	info->OffsetTable = static_cast<int16_t*>(work);
	info->ReversedOffsetTable = static_cast<int16_t*>(work) + 4098;
	info->ByteTable = static_cast<int16_t*>(work) + 4098 + 4098;
	info->EndTable = static_cast<int16_t*>(work) + 4098 + 4098 + 256;
	for (int i = 0; i < 256; i++) {
		info->ByteTable[i] = -1;
		info->EndTable[i] = -1;
	}
}

int search(ScompressInfo* info, const uint8_t* a_src, int& a_offset, int a_maxSize) {
	if (a_maxSize < 3) {
		return 0;
	}
	const uint8_t* search = nullptr;
	int size = 2;
	const uint16_t windowPos = info->WindowPos;
	const uint16_t windowLen = info->WindowLen;
	int16_t* reversedOffsetTable = info->ReversedOffsetTable;
	for (int16_t offset = info->EndTable[*(a_src - 1)]; offset != -1; offset = reversedOffsetTable[offset]) {
		if (offset < windowPos) {
			search = a_src + windowPos - offset;
		} else {
			search = a_src + windowLen + windowPos - offset;
		}
		if (search - a_src < 3) {
			continue;
		}
		if (*(search - 2) != *(a_src - 2) || *(search - 3) != *(a_src - 3)){
			continue;
		}
		int maxSize = static_cast<int>(std::min<int64_t>(a_maxSize, search - a_src));
		int currentSize = 3;
		while (currentSize < maxSize && *(search - currentSize - 1) == *(a_src - currentSize - 1)) {
			currentSize++;
		}
		if (currentSize > size) {
			size = currentSize;
			a_offset = static_cast<int>(search - a_src);
			if (size == a_maxSize) {
				break;
			}
		}
	}
	if (size < 3) {
		return 0;
	}
	return size;
}

inline void slide(ScompressInfo* info, const uint8_t* a_src, int a_size) {
	for (int i = 0; i < a_size; i++) {
		slideByte(info, a_src--);
	}
}

void slideByte(ScompressInfo* info, const uint8_t* a_src) {
	uint8_t inData = *(a_src - 1);
	uint16_t insertOffset = 0;
	const uint16_t windowPos = info->WindowPos;
	const uint16_t windowLen = info->WindowLen;
	int16_t* offsetTable = info->OffsetTable;
	int16_t* reversedOffsetTable = info->ReversedOffsetTable;
	int16_t* byteTable = info->ByteTable;
	int16_t* endTable = info->EndTable;
	if (windowLen == 4098) {
		uint8_t outData = *(a_src + 4097);
		if ((byteTable[outData] = offsetTable[byteTable[outData]]) == -1) {
			endTable[outData] = -1;
		} else {
			reversedOffsetTable[byteTable[outData]] = -1;
		}
		insertOffset = windowPos;
	} else {
		insertOffset = windowLen;
	}
	int16_t offset = endTable[inData];
	if (offset == -1) {
		byteTable[inData] = insertOffset;
	} else {
		offsetTable[offset] = insertOffset;
	}
	endTable[inData] = insertOffset;
	offsetTable[insertOffset] = -1;
	reversedOffsetTable[insertOffset] = offset;
	if (windowLen == 4098) {
		info->WindowPos = (windowPos + 1) % 4098;
	} else {
		info->WindowLen++;
	}
}
#include <fmt/format.h>


bool uncompressFile(const std::filesystem::path &inPath, const std::filesystem::path &outPath) {
	fmt::print("- decoding \'{}\' => \'{}\'\n", inPath.string(), outPath.string());
	ktu::buffer compressed, uncompressed;

	if (!compressed.assign(inPath)) {

		return false;
	}
	
	if (!uncompress(compressed.data(), compressed.size(), uncompressed, uncompressed.size())) {
		fmt::print("ERROR: uncompress error\n\n");
		return false;
	}
	
	uncompressed.writef(outPath);
	
	
	return true;
}

bool compressFile(const std::filesystem::path &inPath, const std::filesystem::path &outPath) {
	fmt::print("- encoding \'{}\' => \'{}\'\n", inPath.string(), outPath.string());
	ktu::buffer uncompressed, compressed;
	if (!uncompressed.assign(inPath)) {
		
		return false;
	}


	
	
	if (!compress(uncompressed.data(), uncompressed.size(), compressed, compressed.size())) {
		fmt::print("ERROR: compress error\n\n");
		return false;
	}
	
	compressed.writef(outPath);
	return true;
}