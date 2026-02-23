#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

namespace World
{
	const int MIN_GROUND = 100.0;
	const int MAX_TERRAIN = MIN_GROUND;
	const int SEA_LEVEL = MIN_GROUND + 40;

	const int CHUNK_SIZE = 15;
	const int CHUNK_SIZE_Y = 256;

	// blocks
	enum class BlockID : uint8_t
	{
		Air,
		Dirt,
		Grass,
		Stone,
		Tree_Leaf,
		Tree_Trunk,
		Glow_Block,
		Sand,
		Water
	};
};

#endif
