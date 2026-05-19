#ifndef RAY_TRACING_WORLD_VK_H
#define RAY_TRACING_WORLD_VK_H

#include <vulkan/vulkan.hpp>

#include "constants.h"

#include "buffer_vk.h"
#include "acceleration_structure_vk.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

class VulkanMain;
struct ChunkDrawList;

struct RTPackedSceneCPU
{
	std::vector<World::RTChunkInfo> opaqueChunkInfos;
	std::vector<World::RTChunkInfo> waterChunkInfos;
};

class RayTracingWorldVk
{
public:
	explicit RayTracingWorldVk(VulkanMain& vk);
	~RayTracingWorldVk();

	void upload(
		vk::CommandBuffer cmd,
		const ChunkDrawList& drawList,
		uint32_t frameIndex
	);

	const std::vector<AccelerationStructureVk>& getTLAS() const { return tlas_; }

	const std::vector<BufferVk>& getPackedRTOpaqueInfoBuffer() const { return packedRTOpaqueInfoBuffer_; }
	const std::vector<vk::DeviceSize>& getPackedRTOpaqueInfoBufferSize() const { return packedRTOpaqueInfoBufferSize_; }
	const std::vector<BufferVk>& getPackedRTWaterInfoBuffer() const { return packedRTWaterInfoBuffer_; }
	const std::vector<vk::DeviceSize>& getPackedRTWaterInfoBufferSize() const { return packedRTWaterInfoBufferSize_; }

private:
	void buildOpaqueRTInstancesFromDrawList(
		const ChunkDrawList& opaqueDrawList,
		std::vector<vk::AccelerationStructureInstanceKHR>& out
	);
	void buildWaterRTInstancesFromDrawList(
		const ChunkDrawList& waterDrawList,
		std::vector<vk::AccelerationStructureInstanceKHR>& out
	);

	void buildPackedOpaqueRTSceneFromDrawList(
		const ChunkDrawList& opaqueDrawList,
		RTPackedSceneCPU& out
	);
	void buildPackedWaterRTSceneFromDrawList(
		const ChunkDrawList& waterDrawList,
		RTPackedSceneCPU& out
	);

	void uploadPackedRTScene(
		vk::CommandBuffer cmd,
		uint32_t frameIndex,
		const RTPackedSceneCPU& cpuScene
	);

	void buildRTSceneKeys(
		const ChunkDrawList& rtDrawList,
		std::vector<uint64_t>& out
	);
private:
	VulkanMain& vk_;

	bool rtSceneReady_{ false };

	std::vector<uint64_t> lastSceneKeys_;
	bool rtSceneDirty_{ true };

	std::vector<BufferVk> packedRTOpaqueInfoBuffer_;
	std::vector<vk::DeviceSize> packedRTOpaqueInfoBufferSize_;
	std::vector<vk::DeviceSize> packedRTOpaqueInfoBufferCapacity_;

	std::vector<BufferVk> packedRTWaterInfoBuffer_;
	std::vector<vk::DeviceSize> packedRTWaterInfoBufferSize_;
	std::vector<vk::DeviceSize> packedRTWaterInfoBufferCapacity_;

	std::vector<AccelerationStructureVk> tlas_;
};

#endif