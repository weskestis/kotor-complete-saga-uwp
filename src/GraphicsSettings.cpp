// SPDX-License-Identifier: GPL-3.0-or-later
#include "GraphicsSettings.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace kotor::video {
namespace {
constexpr int clampi(int value, int lo, int hi) { return std::max(lo, std::min(hi, value)); }
int even(int value) { return std::max(2, value - (value % 2)); }
int integer(const std::string& s, int fallback) {
    try {
        size_t n = 0;
        int result = std::stoi(s, &n, 10);
        return n == s.size() ? result : fallback;
    } catch (const std::exception&) { return fallback; }
}
bool boolean(std::string_view text, bool fallback) {
    if (text == "1" || text == "true" || text == "on") return true;
    if (text == "0" || text == "false" || text == "off") return false;
    return fallback;
}
std::string trim(std::string s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}
}

const std::vector<Resolution>& standardResolutions() {
    static const std::vector<Resolution> all = {
        {854, 480, "480p"}, {960, 540, "540p"}, {1280, 720, "720p"},
        {1600, 900, "900p"}, {1920, 1080, "1080p"}, {2560, 1440, "1440p"},
        {3200, 1800, "1800p"}, {3840, 2160, "4K UHD"}
    };
    return all;
}

GraphicsSettings defaults() { return GraphicsSettings{}; }

void applyPreset(GraphicsSettings& s, Quality q) {
    s.quality = q;
    switch (q) {
    case Quality::Low:
        s.shadowMapSize=512; s.anisotropy=1; s.drawDistanceMeters=48;
        s.textureQuality=2; s.fxaa=false; s.grass=false; s.shadows=false;
        s.particles=false; s.pbr=false; s.ssao=false; s.ssr=false; s.sharpen=false;
        break;
    case Quality::Medium:
        s.shadowMapSize=1024; s.anisotropy=4; s.drawDistanceMeters=64;
        s.textureQuality=1; s.fxaa=true; s.grass=true; s.shadows=true;
        s.particles=true; s.pbr=false; s.ssao=false; s.ssr=false; s.sharpen=true;
        break;
    case Quality::High:
        s.shadowMapSize=2048; s.anisotropy=8; s.drawDistanceMeters=96;
        s.textureQuality=0; s.fxaa=true; s.grass=true; s.shadows=true;
        s.particles=true; s.pbr=true; s.ssao=true; s.ssr=false; s.sharpen=true;
        break;
    case Quality::Ultra:
        s.shadowMapSize=4096; s.anisotropy=16; s.drawDistanceMeters=128;
        s.textureQuality=0; s.fxaa=true; s.grass=true; s.shadows=true;
        s.particles=true; s.pbr=true; s.ssao=true; s.ssr=true; s.sharpen=true;
        break;
    case Quality::Custom: break; // Do not overwrite custom values.
    }
}

void validate(GraphicsSettings& s) {
    // Native target limits: width <= 3840, height <= 2160. Display output is independent.
    s.renderWidth = even(clampi(s.renderWidth, 640, 3840));
    s.renderHeight = even(clampi(s.renderHeight, 360, 2160));
    s.resolutionScale = clampi(s.resolutionScale, 50, 100);
    s.fpsCap = clampi(s.fpsCap, 24, 120);
    const int sizes[] = {512,1024,2048,4096};
    int best=sizes[0];
    for (int v : sizes) if (std::abs(v-s.shadowMapSize) < std::abs(best-s.shadowMapSize)) best=v;
    s.shadowMapSize = best;
    const int af[] = {1,2,4,8,16};
    best=af[0];
    for (int v : af) if (std::abs(v-s.anisotropy) < std::abs(best-s.anisotropy)) best=v;
    s.anisotropy = best;
    s.drawDistanceMeters=clampi(s.drawDistanceMeters,32,256);
    s.textureQuality=clampi(s.textureQuality,0,2);
}

RenderSize internalSize(const GraphicsSettings& input) {
    GraphicsSettings s=input; validate(s);
    return {even(s.renderWidth*s.resolutionScale/100),even(s.renderHeight*s.resolutionScale/100)};
}

std::string qualityName(Quality q) {
    switch (q) {case Quality::Low:return "Low"; case Quality::Medium:return "Medium";
    case Quality::High:return "High"; case Quality::Ultra:return "Ultra";
    case Quality::Custom:return "Custom";}
    return "Custom";
}
Quality parseQuality(std::string_view q) {
    if(q=="Low")return Quality::Low;
    if(q=="Medium")return Quality::Medium;
    if(q=="High")return Quality::High;
    if(q=="Ultra")return Quality::Ultra;
    return Quality::Custom;
}

std::string serialize(const GraphicsSettings& input) {
    auto s=input; validate(s);
    std::ostringstream out;
    out << "# KOTOR Complete Saga graphics config v1\n";
    out << "render_width="<<s.renderWidth<<"\nrender_height="<<s.renderHeight
        <<"\nresolution_scale="<<s.resolutionScale<<"\nfps_cap="<<s.fpsCap
        <<"\nquality="<<qualityName(s.quality)
        <<"\nvsync="<<s.vsync<<"\nfps_overlay="<<s.fpsOverlay
        <<"\nadaptive_resolution="<<s.adaptiveResolution
        <<"\nshadow_map_size="<<s.shadowMapSize<<"\nanisotropy="<<s.anisotropy
        <<"\ndraw_distance="<<s.drawDistanceMeters<<"\ntexture_quality="<<s.textureQuality
        <<"\nfxaa="<<s.fxaa<<"\ngrass="<<s.grass<<"\nshadows="<<s.shadows
        <<"\nparticles="<<s.particles<<"\npbr="<<s.pbr<<"\nssao="<<s.ssao
        <<"\nssr="<<s.ssr<<"\nsharpen="<<s.sharpen<<"\n";
    return out.str();
}

GraphicsSettings deserialize(std::string_view text) {
    GraphicsSettings s=defaults();
    std::istringstream in{std::string(text)};
    std::string line;
    std::unordered_map<std::string,std::string> fields;
    while(std::getline(in,line)) {
        const auto hash=line.find('#'); if(hash!=std::string::npos) line.erase(hash);
        const auto eq=line.find('='); if(eq==std::string::npos) continue;
        const auto name=trim(line.substr(0,eq)), value=trim(line.substr(eq+1));
        fields[name]=value;
    }
    auto number=[&](const char* key,int& target){auto it=fields.find(key);if(it!=fields.end())target=integer(it->second,target);};
    auto flag=[&](const char* key,bool& target){auto it=fields.find(key);if(it!=fields.end())target=boolean(it->second,target);};
    number("render_width",s.renderWidth); number("render_height",s.renderHeight);
    number("resolution_scale",s.resolutionScale);number("fps_cap",s.fpsCap);
    number("shadow_map_size",s.shadowMapSize);number("anisotropy",s.anisotropy);
    number("draw_distance",s.drawDistanceMeters);number("texture_quality",s.textureQuality);
    flag("vsync",s.vsync);flag("fps_overlay",s.fpsOverlay);
    flag("adaptive_resolution",s.adaptiveResolution);flag("fxaa",s.fxaa);
    flag("grass",s.grass);flag("shadows",s.shadows);
    flag("particles",s.particles);flag("pbr",s.pbr);
    flag("ssao",s.ssao);flag("ssr",s.ssr);flag("sharpen",s.sharpen);
    if(auto it=fields.find("quality");it!=fields.end())s.quality=parseQuality(it->second);
    validate(s); // Imported settings must never exceed Xbox project limits.
    return s;
}

std::vector<std::string> capabilityWarnings(const GraphicsSettings& s, const DisplayMode& mode) {
    std::vector<std::string> notes;
    if(s.fpsCap>mode.refreshHz)
        notes.push_back("Frame cap exceeds current display refresh: extra frames may not be shown.");
    if(s.renderWidth>mode.width||s.renderHeight>mode.height)
        notes.push_back("Internal rendering resolution exceeds display output; downsampling will be needed.");
    if(mode.width<3840||mode.height<2160)
        notes.push_back("The connected output is not currently in 4K mode.");
    if(mode.refreshHz<120)
        notes.push_back("The connected output is not currently in a 120 Hz mode.");
    return notes;
}

std::string reoneOptions(const GraphicsSettings& input) {
    auto s=input;validate(s);
    const auto size=internalSize(s);
    auto flag=[](bool b){return b?"1":"0";};
    auto af=[](int a){switch(a){case 1:return 0;case 2:return 1;case 4:return 2;case 8:return 3;default:return 4;}};
    // Reone's current options list has no frame cap; Xbox host must implement pacing.
    std::ostringstream out;
    out<<"width="<<size.width<<"\nheight="<<size.height
       <<"\nvsync="<<flag(s.vsync)<<"\ntexquality="<<s.textureQuality
       <<"\nshadowres="<<(s.shadowMapSize==512?0:s.shadowMapSize==1024?0:s.shadowMapSize==2048?1:2)
       <<"\nanisofilter="<<af(s.anisotropy)<<"\ndrawdist="<<s.drawDistanceMeters
       <<"\ngrass="<<flag(s.grass)<<"\nshadows="<<flag(s.shadows)
       <<"\nparticles="<<flag(s.particles)<<"\npbr="<<flag(s.pbr)
       <<"\nssao="<<flag(s.ssao)<<"\nssr="<<flag(s.ssr)
       <<"\nfxaa="<<flag(s.fxaa)<<"\nsharpen="<<flag(s.sharpen)<<"\n";
    return out.str();
}
} // namespace kotor::video
