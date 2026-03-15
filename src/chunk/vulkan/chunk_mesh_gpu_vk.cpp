#include "chunk_mesh_gpu_vk.h"

#include <vulkan/vulkan.hpp>

#include "vulkan_main.h"
#include "chunk_mesh_data.h"

#include <vector>

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

	vk::CommandBuffer cmd = vk_->beginSingleTimeCommands();
	std::vector<BufferVk> stagingBuffers;

	// -------- OPAQUE --------
	if (!data.opaqueVertices.empty() && !data.opaqueIndices.empty())
	{
		vk::DeviceSize vbSize = sizeof(Vertex) * data.opaqueVertices.size();
		vk::DeviceSize ibSize = sizeof(uint32_t) * data.opaqueIndices.size();

		// VB staging
		BufferVk stagingVB(*vk_);
		stagingVB.create(
			vbSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		stagingVB.upload(data.opaqueVertices.data(), vbSize);

		// VB device local
		newOpaqueVB.create(
			vbSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);
		stagingBuffers.push_back(std::move(stagingVB));
		vk_->recordCopyBuffer(
			cmd,
			stagingBuffers.back().getBuffer(),
			newOpaqueVB.getBuffer(),
			vbSize
		);

		// IB staging
		BufferVk stagingIB(*vk_);
		stagingIB.create(
			ibSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		stagingIB.upload(data.opaqueIndices.data(), ibSize);

		// IB device local
		newOpaqueIB.create(
			ibSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);
		stagingBuffers.push_back(std::move(stagingIB));
		vk_->recordCopyBuffer(
			cmd,
			stagingBuffers.back().getBuffer(),
			newOpaqueIB.getBuffer(), 
			ibSize
		);

		newOpaqueIndexCount = static_cast<uint32_t>(data.opaqueIndices.size());
	}

	// -------- WATER --------
	if (!data.waterVertices.empty() && !data.waterIndices.empty())
	{
		vk::DeviceSize vbSize = sizeof(VertexWater) * data.waterVertices.size();
		vk::DeviceSize ibSize = sizeof(uint32_t) * data.waterIndices.size();

		// VB staging
		BufferVk stagingVB(*vk_);
		stagingVB.create(
			vbSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		stagingVB.upload(data.waterVertices.data(), vbSize);

		// VB device local
		newWaterVB.create(
			vbSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);
		stagingBuffers.push_back(std::move(stagingVB));
		vk_->recordCopyBuffer(
			cmd,
			stagingBuffers.back().getBuffer(),
			newWaterVB.getBuffer(),
			vbSize
		);

		// IB staging
		BufferVk stagingIB(*vk_);
		stagingIB.create(
			ibSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		stagingIB.upload(data.waterIndices.data(), ibSize);

		// IB device local
		newWaterIB.create(
			ibSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);
		stagingBuffers.push_back(std::move(stagingIB));
		vk_->recordCopyBuffer(
			cmd,
			stagingBuffers.back().getBuffer(),
			newWaterIB.getBuffer(),
			ibSize
		);

		newWaterIndexCount = static_cast<uint32_t>(data.waterIndices.size());
	}

	retireCurrentBuffers(vk_->currentFrameIndex());

	opaqueVB_ = std::move(newOpaqueVB);
	opaqueIB_ = std::move(newOpaqueIB);
	waterVB_ = std::move(newWaterVB);
	waterIB_ = std::move(newWaterIB);

	opaqueIndexCount_ = newOpaqueIndexCount;
	waterIndexCount_ = newWaterIndexCount;

	if (!stagingBuffers.empty())
	{
		vk_->submitUpload(cmd, std::move(stagingBuffers));
	}
	else
	{
		vk_->discardSingleTimeCommands(cmd);
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
