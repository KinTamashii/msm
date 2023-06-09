#ifndef BACKWARDLZ77_H_
#define BACKWARDLZ77_H_

// #include <blz/utility.hpp>
#include <ktu/memory.hpp>

// #include SDW_MSC_PUSH_PACKED
struct CompFooter
{
	uint32_t bufferTopAndBottom;
	uint32_t originalBottom;
};
// SDW_GNUC_PACKED;
// #include SDW_MSC_POP_PACKED




bool uncompress(const uint8_t* compressedData, const uint32_t compressedSize, ktu::buffer &uncompressed, size_t position);
inline bool uncompress(const uint8_t* compressedData, const uint32_t compressedSize, ktu::buffer &uncompressed) {
	return uncompress(compressedData, compressedSize, uncompressed, uncompressed.size());
}
bool compress(const uint8_t* uncompressedData, const uint32_t uncompressedSize, ktu::buffer &compressed, size_t position);
inline bool compress(const uint8_t* uncompressedData, const uint32_t uncompressedSize, ktu::buffer &compressed) {
	return compress(uncompressedData, uncompressedSize, compressed, compressed.size());
}
#include <filesystem>

bool uncompressFile(const std::filesystem::path &inPath, const std::filesystem::path &outPath);
bool compressFile(const std::filesystem::path &inPath, const std::filesystem::path &outPath);

#endif	// BACKWARDLZ77_H_
