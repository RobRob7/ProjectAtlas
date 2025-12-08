#ifndef SAVE_H
#define SAVE_H

#include "chunkdata.h"

#include <cstdint>
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include <iostream>

struct ChunkFileHeader
{
	int32_t version = 1;
	int32_t cx;
	int32_t cz;
	int32_t sizeX = CHUNK_SIZE;
	int32_t sizeY = CHUNK_SIZE_Y;
	int32_t sizeZ = CHUNK_SIZE;
};

class Save
{
public:
	Save() = default;

	void saveChunkToFile(const ChunkData& chunk, const std::string& worldPath);
	std::unique_ptr<ChunkData> loadChunkFromFile(int cx, int cz, const std::string& worldPath);
};

#endif
