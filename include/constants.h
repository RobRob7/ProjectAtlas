#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <glm/glm.hpp>

#include <cstdint>
#include <array>
#include <string_view>

enum class Backend
{
	OpenGL,
	Vulkan,
	DX12
};

struct RenderContext
{
	Backend backend = Backend::OpenGL;
	const void* nativeCmd = nullptr;
};

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

namespace Light_Constants
{
	inline constexpr float MIN_COLOR = 0.0f;
	inline constexpr float MAX_COLOR = 1.0f;

	struct LightUBO
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		glm::vec4 color;
	};

	inline constexpr std::array<float, 108> CUBE_VERTICES = {
		// Back face
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,

		 // Front face
		 -0.5f, -0.5f,  0.5f,
		  0.5f, -0.5f,  0.5f,
		  0.5f,  0.5f,  0.5f,
		  0.5f,  0.5f,  0.5f,
		 -0.5f,  0.5f,  0.5f,
		 -0.5f, -0.5f,  0.5f,

		 // Left face
		 -0.5f,  0.5f,  0.5f,
		 -0.5f,  0.5f, -0.5f,
		 -0.5f, -0.5f, -0.5f,
		 -0.5f, -0.5f, -0.5f,
		 -0.5f, -0.5f,  0.5f,
		 -0.5f,  0.5f,  0.5f,

		 // Right face
		  0.5f, -0.5f, -0.5f,
		  0.5f,  0.5f, -0.5f,
		  0.5f,  0.5f,  0.5f,
		  0.5f,  0.5f,  0.5f,
		  0.5f, -0.5f,  0.5f,
		  0.5f, -0.5f, -0.5f,

		  // Bottom face
		  -0.5f, -0.5f, -0.5f,
		   0.5f, -0.5f, -0.5f,
		   0.5f, -0.5f,  0.5f,
		   0.5f, -0.5f,  0.5f,
		  -0.5f, -0.5f,  0.5f,
		  -0.5f, -0.5f, -0.5f,

		  // Top face
		   0.5f,  0.5f,  0.5f,
		   0.5f,  0.5f, -0.5f,
		  -0.5f,  0.5f, -0.5f,
		  -0.5f,  0.5f, -0.5f,
		  -0.5f,  0.5f,  0.5f,
		   0.5f,  0.5f,  0.5f
	};

	struct VertexLight
	{
		glm::vec3 pos;
	};
};

namespace Cubemap_Constants
{
	struct CubemapUBO
	{
		glm::mat4 view;
		glm::mat4 proj;
	};

	const std::array<float, 108> SKYBOX_VERTICES =
	{
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// cubemap default
	const std::array<std::string_view, 6> DEFAULT_FACES = { {
		"texture/cubemap/space_alt/right.png",
		"texture/cubemap/space_alt/left.png",
		"texture/cubemap/space_alt/top.png",
		"texture/cubemap/space_alt/bottom.png",
		"texture/cubemap/space_alt/front.png",
		"texture/cubemap/space_alt/back.png"
	} };

	struct VertexCubemap
	{
		glm::vec3 pos;
	};
};

namespace Chunk_Constants
{
	struct ChunkOpaqueUBO
	{
		// vert
		glm::vec3 u_chunkOrigin;
		float _pad0;

		glm::mat4 u_view;
		glm::mat4 u_proj;

		// frag
		glm::vec4 u_clipPlane;

		glm::vec3 u_viewPos;
		float _pad1;

		glm::vec3 u_lightPos;
		float _pad2;

		glm::vec3 u_lightColor;
		float u_ambientStrength;

		glm::vec2 u_screenSize;
		int32_t u_useSSAO;
		int32_t _pad3;
	};

	struct ChunkWaterUBO
	{
		// vert
		glm::mat4 u_model;
		glm::mat4 u_view;
		glm::mat4 u_proj;

		glm::vec4 u_tileScale_pad = glm::vec4{ 0.02f, 0.0f, 0.0f, 0.0f };

		// frag
		float u_time;
		float u_distortStrength = 8.0f;
		float u_waveSpeed = 0.04f;
		float _pad_waves;

		float u_near;
		float u_far;
		glm::vec2 u_screenSize;

		glm::vec3 u_viewPos;
		int32_t _pad0;

		glm::vec3 u_lightPos;
		int32_t _pad1;

		glm::vec3 u_lightColor;
		float u_ambientStrength;
	};
};

#endif
