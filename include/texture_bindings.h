#ifndef TEXTURE_BINDINGS_H
#define TEXTURE_BINDINGS_H

#include <cstdint>
#include <array>
#include <string_view>

enum class TextureBinding : uint32_t
{
	GNormalTex = 0,
	GDepthTex = 1,
	SceneColorTex = 2,
	SceneDepthTex = 3,
	AtlasTex = 4,
	SSAOBlurTex = 5,
	SSAONoiseTex = 6,
	SSAORaw = 7,
	WaterReflColorTex = 8,
	WaterRefrColorTex = 9,
	WaterRefrDepthTex = 10,
	DudvTex = 11,
	WaterNormalTex = 12,
	CubemapTex = 13,
	ForwardColorTex = 14,
	ForwardDepthTex = 15,

	COUNT
};

constexpr uint32_t TO_API_FORM(TextureBinding b) noexcept
{
	return static_cast<uint32_t>(b);
} // end of TO_API_FORM

inline constexpr std::array<std::string_view, TO_API_FORM(TextureBinding::COUNT)> TEXTURE_NAMES =
{
	"u_gNormal",
	"u_gDepth",
	"u_sceneColorTex",
	"u_sceneDepthTex",
	"u_atlasTex",
	"u_ssaoBlurTex",
	"u_ssaoNoiseTex",
	"u_ssaoRaw",
	"u_waterReflColorTex",
	"u_waterRefrColorTex",
	"u_waterRefrDepthTex",
	"u_waterDUDVTex",
	"u_waterNormalTex",
	"u_skyboxTex",
	"u_forwardColorTex",
	"u_forwardDepthTex",
};


#endif
