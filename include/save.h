#ifndef SAVE_H
#define SAVE_H

#include <string_view>
#include <memory>

class ChunkData;
struct ChunkFileHeader;

class Save
{
public:
	Save() = default;
	~Save();

	void saveChunkToFile(const ChunkData& chunk, const std::string_view& worldPath);
	bool loadChunkFromFile(ChunkData& dst, int cx, int cz, const std::string_view& worldPath);
};

#endif
