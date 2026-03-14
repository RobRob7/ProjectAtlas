#include "chunk_mesh_gpu_vk.h"

#include <vulkan/vulkan.hpp>

#include "vulkan_main.h"
#include "chunk_mesh_data.h"

using namespace World;

//--- PUBLIC ---//
ChunkMeshGPUVk::ChunkMeshGPUVk(VulkanMain& vk)
	: vk_(&vk),
	opaqueVB_(vk),
	opaqueIB_(vk),
	waterVB_(vk),
	waterIB_(vk)
{
} // end of constructor

ChunkMeshGPUVk::~ChunkMeshGPUVk()
{
	if (vk_)
	{
		retireCurrentBuffers(vk_->currentFrameIndex());
	}
} // end of destructor

void ChunkMeshGPUVk::upload(const ChunkMeshData& data)
{
	BufferVk newOpaqueVB(*vk_);
	BufferVk newOpaqueIB(*vk_);
	BufferVk newWaterVB(*vk_);
	BufferVk newWaterIB(*vk_);

	uint32_t newOpaqueIndexCount = 0;
	uint32_t newWaterIndexCount = 0;

	// -------- OPAQUE --------
	if (!data.opaqueVertices.empty() && !data.opaqueIndices.empty())
	{
		vk::DeviceSize vbSize = sizeof(Vertex) * data.opaqueVertices.size();
		vk::DeviceSize ibSize = sizeof(uint32_t) * data.opaqueIndices.size();

		newOpaqueVB.create(
			vbSize,
			vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		newOpaqueVB.upload(data.opaqueVertices.data(), vbSize);

		newOpaqueIB.create(
			ibSize,
			vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		newOpaqueIB.upload(data.opaqueIndices.data(), ibSize);

		newOpaqueIndexCount = static_cast<uint32_t>(data.opaqueIndices.size());
	}

	// -------- WATER --------
	if (!data.waterVertices.empty() && !data.waterIndices.empty())
	{
		vk::DeviceSize vbSize = sizeof(VertexWater) * data.waterVertices.size();
		vk::DeviceSize ibSize = sizeof(uint32_t) * data.waterIndices.size();

		newWaterVB.create(
			vbSize,
			vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		newWaterVB.upload(data.waterVertices.data(), vbSize);

		newWaterIB.create(
			ibSize,
			vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		newWaterIB.upload(data.waterIndices.data(), ibSize);

		newWaterIndexCount = static_cast<uint32_t>(data.waterIndices.size());
	}

	retireCurrentBuffers(vk_->currentFrameIndex());

	opaqueVB_ = std::move(newOpaqueVB);
	opaqueIB_ = std::move(newOpaqueIB);
	waterVB_ = std::move(newWaterVB);
	waterIB_ = std::move(newWaterIB);

	opaqueIndexCount_ = newOpaqueIndexCount;
	waterIndexCount_ = newWaterIndexCount;
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


//--- PRIVATE ---//
void ChunkMeshGPUVk::retireCurrentBuffers(uint32_t frameIndex)
{
	if (!opaqueVB_.valid() && !opaqueIB_.valid() &&
		!waterVB_.valid() && !waterIB_.valid())
	{
		return;
	}

	vk_->retireChunkBuffers(
		frameIndex,
		std::move(opaqueVB_),
		std::move(opaqueIB_),
		std::move(waterVB_),
		std::move(waterIB_)
	);
} // end of retireCurrentBuffers()
