#include "chunk_mesh_gpu_vk.h"

#include "vulkan_main.h"
#include "chunk_mesh_data.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

//--- PUBLIC ---//
ChunkMeshGPUVk::ChunkMeshGPUVk(VulkanMain& vk)
	: vk_(vk),
	opaqueVB_(vk),
	opaqueIB_(vk),
	waterVB_(vk),
	waterIB_(vk)
{
} // end of constructor

ChunkMeshGPUVk::~ChunkMeshGPUVk()
{
	vk_.getDevice().waitIdle();
}

void ChunkMeshGPUVk::upload(const ChunkMeshData& data)
{
	opaqueIndexCount_ = static_cast<uint32_t>(data.opaqueIndices.size());
	waterIndexCount_ = static_cast<uint32_t>(data.waterIndices.size());

	// -------- OPAQUE --------
	if (!data.opaqueVertices.empty() && !data.opaqueIndices.empty())
	{
		const vk::DeviceSize vbSize = sizeof(Vertex) * data.opaqueVertices.size();
		const vk::DeviceSize ibSize = sizeof(uint32_t) * data.opaqueIndices.size();

		opaqueVB_.create(
			vbSize,
			vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		opaqueVB_.upload(data.opaqueVertices.data(), vbSize);

		opaqueIB_.create(
			ibSize,
			vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		opaqueIB_.upload(data.opaqueIndices.data(), ibSize);
	}
	else
	{
		opaqueIndexCount_ = 0;
	}

	// -------- WATER --------
	if (!data.waterVertices.empty() && !data.waterIndices.empty())
	{
		const vk::DeviceSize vbSize = sizeof(VertexWater) * data.waterVertices.size();
		const vk::DeviceSize ibSize = sizeof(uint32_t) * data.waterIndices.size();

		waterVB_.create(
			vbSize,
			vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		waterVB_.upload(data.waterVertices.data(), vbSize);

		waterIB_.create(
			ibSize,
			vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		waterIB_.upload(data.waterIndices.data(), ibSize);
	}
	else
	{
		waterIndexCount_ = 0;
	}
} // end of upload()

void ChunkMeshGPUVk::drawOpaque(vk::CommandBuffer cmd)
{
	if (!cmd || opaqueIndexCount_ == 0 || !opaqueVB_.valid() || !opaqueIB_.valid())
		return;

	vk::Buffer vb = opaqueVB_.getBuffer();
	vk::DeviceSize offset = 0;

	cmd.bindVertexBuffers(0, 1, &vb, &offset);
	cmd.bindIndexBuffer(opaqueIB_.getBuffer(), 0, vk::IndexType::eUint32);
	cmd.drawIndexed(opaqueIndexCount_, 1, 0, 0, 0);
} // end of drawOpaque()

void ChunkMeshGPUVk::drawWater(vk::CommandBuffer cmd)
{
	if (!cmd || waterIndexCount_ == 0 || !waterVB_.valid() || !waterIB_.valid())
		return;

	vk::Buffer vb = waterVB_.getBuffer();
	vk::DeviceSize offset = 0;

	cmd.bindVertexBuffers(0, 1, &vb, &offset);
	cmd.bindIndexBuffer(waterIB_.getBuffer(), 0, vk::IndexType::eUint32);
	cmd.drawIndexed(waterIndexCount_, 1, 0, 0, 0);
} // end of drawWater()
