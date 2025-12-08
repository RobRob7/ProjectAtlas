#include "save.h"

//--- PUBLIC ---//
void Save::saveChunkToFile(const ChunkData& chunk, const std::string& worldPath)
{
	// full world dir path
	std::filesystem::path worldDir = std::filesystem::path(SAVE_PATH) / worldPath;

	if (!std::filesystem::exists(worldDir))
	{
		std::filesystem::create_directories(worldDir);

		#ifdef _DEBUG
		std::cout << "Created directory: " << worldDir << "\n";
		#endif
	}

	std::filesystem::path chunkPath = 
		worldDir / ("chunk_" +
			std::to_string(chunk.m_chunkX) + "_" +
			std::to_string(chunk.m_chunkZ) + ".bin");

	std::ofstream out(chunkPath, std::ios::binary);
	if (!out)
	{
		std::cerr << "Failed to open chunk file (w) at path: " << chunkPath << "\n";
		return;
	}

	ChunkFileHeader header;
	header.cx = chunk.m_chunkX;
	header.cz = chunk.m_chunkZ;

	out.write(reinterpret_cast<const char*>(&header), sizeof(header));
	out.write(reinterpret_cast<const char*>(chunk.getBlocks().data()), chunk.getBlocks().size() * sizeof(BlockID));

	#ifdef _DEBUG
	std::cout << "Saved chunk to: " << chunkPath << "\n";
	#endif
} // end of saveChunkToFile()

std::unique_ptr<ChunkData> Save::loadChunkFromFile(int cx, int cz, const std::string& worldPath)
{
	// full world dir path
	std::filesystem::path worldDir = std::filesystem::path(SAVE_PATH) / worldPath;

	std::filesystem::path chunkPath =
		worldDir / ("chunk_" +
			std::to_string(cx) + "_" +
			std::to_string(cz) + ".bin");

	if (!std::filesystem::exists(chunkPath))
	{
		return nullptr;
	}

	std::ifstream in(chunkPath, std::ios::binary);
	if (!in)
	{
		std::cerr << "Failed to open chunk file (r) at path: " << chunkPath << "\n";
		return nullptr;
	}

	ChunkFileHeader header{};
	in.read(reinterpret_cast<char*>(&header), sizeof(header));

	// create chunk data
	std::unique_ptr<ChunkData> chunk = std::make_unique<ChunkData>(cx, cz);

	// load chunk from save
	chunk->loadData(in);

	chunk->m_dirty = false;
	return chunk;
} // end of loadChunkFromFile()