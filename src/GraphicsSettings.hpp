// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace kotor::video {

struct Resolution {
    int width;
    int height;
    const char* label;
};

// Display output and internal scene rendering size are intentionally separate.
struct DisplayMode {
    int width{1920};
    int height{1080};
    int refreshHz{60};
    bool hdr{false};
};

enum class Quality { Low, Medium, High, Ultra, Custom };

struct GraphicsSettings {
    int renderWidth{3840};
    int renderHeight{2160};
    int resolutionScale{100};           // internal rendering: 50-100%
    int fpsCap{120};                    // 24-120; never intentionally uncapped
    Quality quality{Quality::Ultra};
    bool vsync{true};
    bool fpsOverlay{true};
    bool adaptiveResolution{false};
    int shadowMapSize{4096};
    int anisotropy{16};
    int drawDistanceMeters{128};
    int textureQuality{0};              // reone: 0=high, 1=medium, 2=low
    bool fxaa{true};
    bool grass{true};
    bool shadows{true};
    bool particles{true};
    bool pbr{true};
    bool ssao{true};
    bool ssr{true};
    bool sharpen{true};
};

struct RenderSize { int width; int height; };

const std::vector<Resolution>& standardResolutions();
GraphicsSettings defaults();
void applyPreset(GraphicsSettings& settings, Quality quality);
void validate(GraphicsSettings& settings);
RenderSize internalSize(const GraphicsSettings& settings);
std::string serialize(const GraphicsSettings& settings);
GraphicsSettings deserialize(std::string_view text);
std::string qualityName(Quality quality);
Quality parseQuality(std::string_view text);
std::vector<std::string> capabilityWarnings(const GraphicsSettings& settings, const DisplayMode& output);
std::string reoneOptions(const GraphicsSettings& settings);

} // namespace kotor::video
