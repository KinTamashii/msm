#pragma once
#include <string>
#include <filesystem>

#include <ktu/memory.hpp>
#include <iostream>
#include <msm/titleID.hpp>



/* Decode a msbt file into a msm file. */
void decode(const std::filesystem::path &input, const std::filesystem ::path&output, title::id id);

/* Mario and Luigi Dream Team: Decode an .dat archive of msbt files into an msm file. */
void decodeArchiveML4(const std::filesystem::path &input, const std::filesystem::path &output, const std::filesystem::path &external, title::id id);

/* BG4 Compression: Decode a BG4 archive of any files into an msm file. */
void decodeArchiveBG4(const std::filesystem::path &input, const std::filesystem::path &output, title::id id);