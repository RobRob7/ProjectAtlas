#include "ray_tracing_world_vk.h"

#include "vulkan_main.h"

#include "chunk_draw_list.h"
#include "chunk_mesh_gpu_vk.h"

//--- PUBLIC ---//
RayTracingWorldVk::RayTracingWorldVk(VulkanMain& vk)
	: vk_(vk)
{
	packedRTOpaqueInfoBuffer_.reserve(vk_.getMaxFramesInFlight());
	packedRTOpaqueInfoBufferSize_.resize(vk_.getMaxFramesInFlight());
	packedRTOpaqueInfoBufferCapacity_.resize(vk_.getMaxFramesInFlight());

	packedRTWaterInfoBuffer_.reserve(vk_.getMaxFramesInFlight());
	packedRTWaterInfoBufferSize_.resize(vk_.getMaxFramesInFlight());
	packedRTWaterInfoBufferCapacity_.resize(vk_.getMaxFramesInFlight());

	tlas_.reserve(vk_.getMaxFramesInFlight());

	for (uint32_t i = 0; i < vk_.getMaxFramesInFlight(); ++i)
	{
		packedRTOpaqueInfoBuffer_.emplace_back(vk_);
		packedRTWaterInfoBuffer_.emplace_back(vk_);

		tlas_.emplace_back(vk_);
	} // end for
} // end of constructor

RayTracingWorldVk::~RayTracingWorldVk() = default;

void RayTracingWorldVk::upload(
	vk::CommandBuffer cmd,
	const ChunkDrawList& drawList,
	uint32_t frameIndex
)
{
	cmd.beginDebugUtilsLabelEXT({ "RayTracingWorldVk-Upload::cmd" });

	rtSceneReady_ = false;

	std::vector<uint64_t> currentKeys;
	buildRTSceneKeys(drawList, currentKeys);

	if (currentKeys != lastSceneKeys_)
	{
		lastSceneKeys_ = currentKeys;
		rtSceneDirty_ = true;
	}

	AccelerationStructureVk& frameTLAS = tlas_[frameIndex];

	if (rtSceneDirty_ || !frameTLAS.valid())
	{
		RTPackedSceneCPU chunkCPUScene;
		buildPackedOpaqueRTSceneFromDrawList(drawList, chunkCPUScene);
		buildPackedWaterRTSceneFromDrawList(drawList, chunkCPUScene);

		if (chunkCPUScene.opaqueChunkInfos.empty())
		{
			packedRTOpaqueInfoBufferSize_[frameIndex] = 0;
		}

		if (chunkCPUScene.waterChunkInfos.empty())
		{
			packedRTWaterInfoBufferSize_[frameIndex] = 0;
		}

		if (chunkCPUScene.opaqueChunkInfos.empty() &&
			chunkCPUScene.waterChunkInfos.empty())
		{
			rtSceneReady_ = false;
			return;
		}

		uploadPackedRTScene(cmd, frameIndex, chunkCPUScene);

		std::vector<vk::AccelerationStructureInstanceKHR> instances;
		std::vector<vk::AccelerationStructureInstanceKHR> waterInstances;

		buildOpaqueRTInstancesFromDrawList(drawList, instances);
		buildWaterRTInstancesFromDrawList(drawList, waterInstances);

		instances.insert(
			instances.end(),
			waterInstances.begin(),
			waterInstances.end()
		);

		if (instances.empty())
		{
			packedRTOpaqueInfoBufferSize_[frameIndex] = 0;
			packedRTWaterInfoBufferSize_[frameIndex] = 0;
			rtSceneReady_ = false;
			return;
		}

		if (frameTLAS.valid())
		{
			vk_.retireAccelerationStructure(
				frameIndex,
				std::move(frameTLAS)
			);
			frameTLAS = AccelerationStructureVk(vk_);
		}

		frameTLAS.buildTLASOnCmd(cmd, instances);

		vk::MemoryBarrier asBarrier{};
		asBarrier.srcAccessMask =
			vk::AccessFlagBits::eAccelerationStructureWriteKHR;
		asBarrier.dstAccessMask =
			vk::AccessFlagBits::eAccelerationStructureReadKHR |
			vk::AccessFlagBits::eShaderRead;

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
			vk::PipelineStageFlagBits::eRayTracingShaderKHR,
			{},
			asBarrier,
			nullptr,
			nullptr
		);
	}

	rtSceneReady_ =
		frameTLAS.valid() &&
		(packedRTOpaqueInfoBufferSize_[frameIndex] > 0 ||
			packedRTWaterInfoBufferSize_[frameIndex] > 0);

	cmd.endDebugUtilsLabelEXT();
} // end of upload()


//--- PRIVATE ---//
void RayTracingWorldVk::buildOpaqueRTInstancesFromDrawList(
	const ChunkDrawList& opaqueDrawList,
	std::vector<vk::AccelerationStructureInstanceKHR>& out
)
{
	out.clear();

	for (const auto& item : opaqueDrawList.items)
	{
		if (!item.gpu)
			continue;

		auto* chunkGpuVk = dynamic_cast<ChunkMeshGPUVk*>(item.gpu.get());
		if (!chunkGpuVk)
			continue;

		if (!chunkGpuVk->getOpaqueBLAS().valid())
			continue;

		const auto& chunkVerts = chunkGpuVk->getOpaqueRTVerticesCPU();
		const auto& chunkIndices = chunkGpuVk->getOpaqueRTIndicesCPU();

		if (chunkVerts.empty() || chunkIndices.empty())
			continue;

		vk::AccelerationStructureInstanceKHR inst{};

		inst.transform.matrix[0][0] = 1.0f;
		inst.transform.matrix[0][1] = 0.0f;
		inst.transform.matrix[0][2] = 0.0f;
		inst.transform.matrix[0][3] = item.chunkOrigin.x;

		inst.transform.matrix[1][0] = 0.0f;
		inst.transform.matrix[1][1] = 1.0f;
		inst.transform.matrix[1][2] = 0.0f;
		inst.transform.matrix[1][3] = item.chunkOrigin.y;

		inst.transform.matrix[2][0] = 0.0f;
		inst.transform.matrix[2][1] = 0.0f;
		inst.transform.matrix[2][2] = 1.0f;
		inst.transform.matrix[2][3] = item.chunkOrigin.z;

		inst.instanceCustomIndex = static_cast<uint32_t>(out.size());
		inst.mask = 0x01;
		inst.instanceShaderBindingTableRecordOffset = 0;
		inst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		inst.accelerationStructureReference = chunkGpuVk->getOpaqueBLAS().deviceAddress();

		out.push_back(inst);
	} // end for
} // end of buildOpaqueRTInstancesFromDrawList()

void RayTracingWorldVk::buildWaterRTInstancesFromDrawList(
	const ChunkDrawList& waterDrawList,
	std::vector<vk::AccelerationStructureInstanceKHR>& out
)
{
	out.clear();

	for (const auto& item : waterDrawList.items)
	{
		if (!item.gpu)
			continue;

		auto* chunkGpuVk = dynamic_cast<ChunkMeshGPUVk*>(item.gpu.get());
		if (!chunkGpuVk)
			continue;

		if (!chunkGpuVk->getWaterBLAS().valid())
			continue;

		const auto& waterVerts = chunkGpuVk->getWaterRTVerticesCPU();
		const auto& waterIndices = chunkGpuVk->getWaterRTIndicesCPU();

		if (waterVerts.empty() || waterIndices.empty())
			continue;

		vk::AccelerationStructureInstanceKHR inst{};

		inst.transform.matrix[0][0] = 1.0f;
		inst.transform.matrix[0][1] = 0.0f;
		inst.transform.matrix[0][2] = 0.0f;
		inst.transform.matrix[0][3] = item.chunkOrigin.x;

		inst.transform.matrix[1][0] = 0.0f;
		inst.transform.matrix[1][1] = 1.0f;
		inst.transform.matrix[1][2] = 0.0f;
		inst.transform.matrix[1][3] = item.chunkOrigin.y;

		inst.transform.matrix[2][0] = 0.0f;
		inst.transform.matrix[2][1] = 0.0f;
		inst.transform.matrix[2][2] = 1.0f;
		inst.transform.matrix[2][3] = item.chunkOrigin.z;

		inst.instanceCustomIndex = static_cast<uint32_t>(out.size());
		inst.mask = 0x02;
		inst.instanceShaderBindingTableRecordOffset = 1;
		inst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		inst.accelerationStructureReference = chunkGpuVk->getWaterBLAS().deviceAddress();

		out.push_back(inst);
	} // end for
} // end of buildWaterRTInstancesFromDrawList()

void RayTracingWorldVk::buildPackedOpaqueRTSceneFromDrawList(
	const ChunkDrawList& opaqueDrawList,
	RTPackedSceneCPU& out
)
{
	out.opaqueChunkInfos.clear();

	for (const auto& item : opaqueDrawList.items)
	{
		if (!item.gpu)
			continue;

		auto* chunkGpuVk = dynamic_cast<ChunkMeshGPUVk*>(item.gpu.get());
		if (!chunkGpuVk)
			continue;

		if (!chunkGpuVk->getOpaqueBLAS().valid())
			continue;

		const std::vector<World::RTVertex>& chunkVerts = chunkGpuVk->getOpaqueRTVerticesCPU();
		const std::vector<uint32_t>& chunkIndices = chunkGpuVk->getOpaqueRTIndicesCPU();

		if (chunkVerts.empty() || chunkIndices.empty())
			continue;

		World::RTChunkInfo info{};
		info.vertexAddress = chunkGpuVk->getOpaqueRTVertexAddress();
		info.indexAddress = chunkGpuVk->getOpaqueRTIndexAddress();
		info.countsPad.x = static_cast<uint32_t>(chunkVerts.size());
		info.countsPad.y = static_cast<uint32_t>(chunkIndices.size());
		info.chunkOrigin = glm::vec4(item.chunkOrigin, 0.0f);

		out.opaqueChunkInfos.push_back(info);
	} // end for
} // end of buildPackedOpaqueRTSceneFromDrawList()

void RayTracingWorldVk::buildPackedWaterRTSceneFromDrawList(
	const ChunkDrawList& waterDrawList,
	RTPackedSceneCPU& out
)
{
	out.waterChunkInfos.clear();

	for (const auto& item : waterDrawList.items)
	{
		if (!item.gpu)
			continue;

		auto* chunkGpuVk = dynamic_cast<ChunkMeshGPUVk*>(item.gpu.get());
		if (!chunkGpuVk)
			continue;

		if (!chunkGpuVk->getWaterBLAS().valid())
			continue;

		const std::vector<World::RTVertex>& chunkVerts = chunkGpuVk->getWaterRTVerticesCPU();
		const std::vector<uint32_t>& chunkIndices = chunkGpuVk->getWaterRTIndicesCPU();

		if (chunkVerts.empty() || chunkIndices.empty())
			continue;

		World::RTChunkInfo info{};
		info.vertexAddress = chunkGpuVk->getWaterRTVertexAddress();
		info.indexAddress = chunkGpuVk->getWaterRTIndexAddress();
		info.countsPad.x = static_cast<uint32_t>(chunkVerts.size());
		info.countsPad.y = static_cast<uint32_t>(chunkIndices.size());
		info.chunkOrigin = glm::vec4(item.chunkOrigin, 0.0f);

		out.waterChunkInfos.push_back(info);
	} // end for
} // end of buildPackedOpaqueRTSceneFromDrawList()

void RayTracingWorldVk::uploadPackedRTScene(
	vk::CommandBuffer cmd,
	uint32_t frameIndex,
	const RTPackedSceneCPU& cpuScene
)
{
	std::vector<BufferVk> stagingBuffers;
	stagingBuffers.reserve(2);

	packedRTOpaqueInfoBufferSize_[frameIndex] =
		sizeof(World::RTChunkInfo) * cpuScene.opaqueChunkInfos.size();

	packedRTWaterInfoBufferSize_[frameIndex] =
		sizeof(World::RTChunkInfo) * cpuScene.waterChunkInfos.size();

	// packed opaque info buffer
	if (packedRTOpaqueInfoBufferSize_[frameIndex] > 0)
	{
		BufferVk staging(vk_);
		staging.create(
			packedRTOpaqueInfoBufferSize_[frameIndex],
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent
		);

		staging.upload(
			cpuScene.opaqueChunkInfos.data(),
			packedRTOpaqueInfoBufferSize_[frameIndex]
		);

		if (!packedRTOpaqueInfoBuffer_[frameIndex].getBuffer() ||
			packedRTOpaqueInfoBufferSize_[frameIndex] > packedRTOpaqueInfoBufferCapacity_[frameIndex])
		{
			if (packedRTOpaqueInfoBuffer_[frameIndex].valid())
			{
				vk_.retireBuffer(frameIndex, std::move(packedRTOpaqueInfoBuffer_[frameIndex]));
			}

			packedRTOpaqueInfoBuffer_[frameIndex] = BufferVk(vk_);
			packedRTOpaqueInfoBuffer_[frameIndex].create(
				packedRTOpaqueInfoBufferSize_[frameIndex],
				vk::BufferUsageFlagBits::eTransferDst |
				vk::BufferUsageFlagBits::eStorageBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal
			);

			packedRTOpaqueInfoBufferCapacity_[frameIndex] = packedRTOpaqueInfoBufferSize_[frameIndex];
		}

		stagingBuffers.push_back(std::move(staging));

		vk_.recordCopyBuffer(
			cmd,
			stagingBuffers.back().getBuffer(),
			packedRTOpaqueInfoBuffer_[frameIndex].getBuffer(),
			packedRTOpaqueInfoBufferSize_[frameIndex]
		);
	}

	// packed water info buffer
	if (packedRTWaterInfoBufferSize_[frameIndex] > 0)
	{
		BufferVk staging(vk_);
		staging.create(
			packedRTWaterInfoBufferSize_[frameIndex],
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent
		);

		staging.upload(
			cpuScene.waterChunkInfos.data(),
			packedRTWaterInfoBufferSize_[frameIndex]
		);

		if (!packedRTWaterInfoBuffer_[frameIndex].getBuffer() ||
			packedRTWaterInfoBufferSize_[frameIndex] > packedRTWaterInfoBufferCapacity_[frameIndex])
		{
			if (packedRTWaterInfoBuffer_[frameIndex].valid())
			{
				vk_.retireBuffer(frameIndex, std::move(packedRTWaterInfoBuffer_[frameIndex]));
			}

			packedRTWaterInfoBuffer_[frameIndex] = BufferVk(vk_);
			packedRTWaterInfoBuffer_[frameIndex].create(
				packedRTWaterInfoBufferSize_[frameIndex],
				vk::BufferUsageFlagBits::eTransferDst |
				vk::BufferUsageFlagBits::eStorageBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal
			);

			packedRTWaterInfoBufferCapacity_[frameIndex] = packedRTWaterInfoBufferSize_[frameIndex];
		}

		stagingBuffers.push_back(std::move(staging));

		vk_.recordCopyBuffer(
			cmd,
			stagingBuffers.back().getBuffer(),
			packedRTWaterInfoBuffer_[frameIndex].getBuffer(),
			packedRTWaterInfoBufferSize_[frameIndex]
		);
	}

	for (auto& staging : stagingBuffers)
	{
		vk_.retireBuffer(frameIndex, std::move(staging));
	} // end for

	std::vector<vk::BufferMemoryBarrier> barriers;

	auto addBarrier = [&](BufferVk& buffer, vk::DeviceSize size)
		{
			if (!buffer.getBuffer() || size == 0)
				return;

			vk::BufferMemoryBarrier b{};
			b.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			b.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			b.buffer = buffer.getBuffer();
			b.offset = 0;
			b.size = size;

			barriers.push_back(b);
		};

	addBarrier(packedRTOpaqueInfoBuffer_[frameIndex], packedRTOpaqueInfoBufferSize_[frameIndex]);
	addBarrier(packedRTWaterInfoBuffer_[frameIndex], packedRTWaterInfoBufferSize_[frameIndex]);

	if (!barriers.empty())
	{
		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eRayTracingShaderKHR,
			{},
			0, nullptr,
			static_cast<uint32_t>(barriers.size()), barriers.data(),
			0, nullptr
		);
	}
} // end of uploadPackedRTScene()

void RayTracingWorldVk::buildRTSceneKeys(
	const ChunkDrawList& rtDrawList,
	std::vector<uint64_t>& out
)
{
	out.clear();
	out.reserve(rtDrawList.items.size());

	for (const auto& item : rtDrawList.items)
	{
		if (!item.gpu)
			continue;

		auto* chunkGpuVk = dynamic_cast<ChunkMeshGPUVk*>(item.gpu.get());
		if (!chunkGpuVk)
			continue;

		bool hasOpaque =
			chunkGpuVk->getOpaqueBLAS().valid() &&
			!chunkGpuVk->getOpaqueRTVerticesCPU().empty() &&
			!chunkGpuVk->getOpaqueRTIndicesCPU().empty();

		bool hasWater =
			chunkGpuVk->getOpaqueBLAS().valid() &&
			!chunkGpuVk->getWaterRTVerticesCPU().empty() &&
			!chunkGpuVk->getWaterRTIndicesCPU().empty();

		if (!hasOpaque && !hasWater)
			continue;

		const uint64_t x = static_cast<uint64_t>(static_cast<uint32_t>(
			static_cast<int32_t>(item.chunkOrigin.x)));
		const uint64_t y = static_cast<uint64_t>(static_cast<uint32_t>(
			static_cast<int32_t>(item.chunkOrigin.y)));
		const uint64_t z = static_cast<uint64_t>(static_cast<uint32_t>(
			static_cast<int32_t>(item.chunkOrigin.z)));

		const uint64_t posKey =
			(x << 42) ^ (y << 21) ^ z;

		// mix position with geometry version
		const uint64_t versionKey =
			item.geometryVersion * 0x9E3779B185EBCA87ull;
		out.push_back(posKey ^ versionKey);
	} // end for

	std::sort(out.begin(), out.end());
} // end of buildRTSceneKeys()