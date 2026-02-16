#ifndef SAVE_H
#define SAVE_H

#include <string_view>
#include <memory>

class ChunkData;
struct ChunkFileHeader;

class Save
{
public:
	Save();
	~Save();

	void saveChunkToFile(const ChunkData& chunk, const std::string_view& worldPath);
	std::unique_ptr<ChunkData> loadChunkFromFile(int cx, int cz, const std::string_view& worldPath);
};

#endif
