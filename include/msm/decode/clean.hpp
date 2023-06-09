#pragma once
#include <string>
#include <filesystem>

#include <ktu/memory.hpp>
#include <iostream>
#include <msm/titleID.hpp>
#include <msm/width.hpp>
#include <msm/common.hpp>


#include <stack>

/* Decode a msbt file into a text file. */
void decodeClean(const std::filesystem::path &input, const std::filesystem::path &output, const std::filesystem::path &format, title::id id, width::view width);

/* Mario and Luigi Dream Team: Decode an .dat archive of msbt files into a clean text file. */
void decodeCleanArchiveML4(const std::filesystem::path &input, const std::filesystem::path &output, const std::filesystem::path &format, const std::filesystem::path &external, title::id id, width::view width);

/* BG4 Compression: Decode a BG4 archive of any files into a text file. */
void decodeCleanArchiveBG4(const std::filesystem::path &input, const std::filesystem::path &output, const std::filesystem::path &format, title::id id, width::view width);