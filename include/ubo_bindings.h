#ifndef UBO_BINDINGS_H
#define UBO_BINDINGS_H

#include <cstdint>

enum class UBOBinding : uint32_t
{
    Chunk = 0,
    Crosshair = 1,
    Cubemap = 2,
    DebugPass = 3,
    FogPass = 4,
    FXAAPass = 5,
    Gbuffer = 6,
    Light = 7,
    PresentPass = 8,
    SSAOPass = 9,
    SSAOBlur = 10,
    WaterPass = 11,
};

constexpr uint32_t TO_API_FORM(UBOBinding b) noexcept
{
    return static_cast<uint32_t>(b);
} // end of TO_API_FORM

#endif
