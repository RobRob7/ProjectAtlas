#ifndef ICHUNKMESHGPU_H
#define ICHUNKMESHGPU_H

struct ChunkMeshData;

class IChunkMeshGPU
{
public:
	virtual ~IChunkMeshGPU() = default;

	virtual void upload(const ChunkMeshData& data) = 0;
	virtual void drawOpaque() = 0;
	virtual void drawWater() = 0;
};

#endif
