#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <glm/glm.hpp>

#include <cstdint>
#include <array>
#include <string_view>

struct RenderContext
{
	enum class Backend { OpenGL, Vulkan };

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

	// pos, normals, texcoords
	inline constexpr std::array<float, 288> CUBE_VERTICES = {
		// =========================
		// Back face (Z-)
		// =========================
		 0.5f, 0.5f,-0.5f,   0.0f,0.0f,-1.0f,   1.0f,1.0f,
		 0.5f,-0.5f,-0.5f,   0.0f,0.0f,-1.0f,   1.0f,0.0f,
		-0.5f,-0.5f,-0.5f,   0.0f,0.0f,-1.0f,   0.0f,0.0f,
		-0.5f,-0.5f,-0.5f,   0.0f,0.0f,-1.0f,   0.0f,0.0f,
		-0.5f, 0.5f,-0.5f,   0.0f,0.0f,-1.0f,   0.0f,1.0f,
		 0.5f, 0.5f,-0.5f,   0.0f,0.0f,-1.0f,   1.0f,1.0f,

		 // =========================
		 // Front face (Z+)
		 // =========================
		 -0.5f,-0.5f, 0.5f,   0.0f,0.0f, 1.0f,   0.0f,0.0f,
		  0.5f,-0.5f, 0.5f,   0.0f,0.0f, 1.0f,   1.0f,0.0f,
		  0.5f, 0.5f, 0.5f,   0.0f,0.0f, 1.0f,   1.0f,1.0f,
		  0.5f, 0.5f, 0.5f,   0.0f,0.0f, 1.0f,   1.0f,1.0f,
		 -0.5f, 0.5f, 0.5f,   0.0f,0.0f, 1.0f,   0.0f,1.0f,
		 -0.5f,-0.5f, 0.5f,   0.0f,0.0f, 1.0f,   0.0f,0.0f,

		 // =========================
		 // Left face (X-)
		 // =========================
		 -0.5f, 0.5f, 0.5f,  -1.0f,0.0f,0.0f,   1.0f,0.0f,
		 -0.5f, 0.5f,-0.5f,  -1.0f,0.0f,0.0f,   1.0f,1.0f,
		 -0.5f,-0.5f,-0.5f,  -1.0f,0.0f,0.0f,   0.0f,1.0f,
		 -0.5f,-0.5f,-0.5f,  -1.0f,0.0f,0.0f,   0.0f,1.0f,
		 -0.5f,-0.5f, 0.5f,  -1.0f,0.0f,0.0f,   0.0f,0.0f,
		 -0.5f, 0.5f, 0.5f,  -1.0f,0.0f,0.0f,   1.0f,0.0f,

		 // =========================
		 // Right face (X+)
		 // =========================
		  0.5f,-0.5f,-0.5f,   1.0f,0.0f,0.0f,   0.0f,1.0f,
		  0.5f, 0.5f,-0.5f,   1.0f,0.0f,0.0f,   1.0f,1.0f,
		  0.5f, 0.5f, 0.5f,   1.0f,0.0f,0.0f,   1.0f,0.0f,
		  0.5f, 0.5f, 0.5f,   1.0f,0.0f,0.0f,   1.0f,0.0f,
		  0.5f,-0.5f, 0.5f,   1.0f,0.0f,0.0f,   0.0f,0.0f,
		  0.5f,-0.5f,-0.5f,   1.0f,0.0f,0.0f,   0.0f,1.0f,

		  // =========================
		  // Bottom face (Y-)
		  // =========================
		  -0.5f,-0.5f,-0.5f,   0.0f,-1.0f,0.0f,   0.0f,1.0f,
		   0.5f,-0.5f,-0.5f,   0.0f,-1.0f,0.0f,   1.0f,1.0f,
		   0.5f,-0.5f, 0.5f,   0.0f,-1.0f,0.0f,   1.0f,0.0f,
		   0.5f,-0.5f, 0.5f,   0.0f,-1.0f,0.0f,   1.0f,0.0f,
		  -0.5f,-0.5f, 0.5f,   0.0f,-1.0f,0.0f,   0.0f,0.0f,
		  -0.5f,-0.5f,-0.5f,   0.0f,-1.0f,0.0f,   0.0f,1.0f,

		  // =========================
		  // Top face (Y+)
		  // =========================
		  0.5f, 0.5f, 0.5f,   0.0f,1.0f,0.0f,   1.0f,0.0f,
		  0.5f, 0.5f,-0.5f,   0.0f,1.0f,0.0f,   1.0f,1.0f,
		 -0.5f, 0.5f,-0.5f,   0.0f,1.0f,0.0f,   0.0f,1.0f,
		 -0.5f, 0.5f,-0.5f,   0.0f,1.0f,0.0f,   0.0f,1.0f,
		 -0.5f, 0.5f, 0.5f,   0.0f,1.0f,0.0f,   0.0f,0.0f,
		  0.5f, 0.5f, 0.5f,   0.0f,1.0f,0.0f,   1.0f,0.0f,
	};

	struct VertexLight
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
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
		// positions          
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

#endif
