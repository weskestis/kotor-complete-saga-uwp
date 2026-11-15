// SPDX-License-Identifier: GPL-3.0-or-later
#include "GraphicsMenu.hpp"
#include <algorithm>
#include <array>
#include <sstream>

namespace kotor::video {
namespace {
    constexpr std::array<int,8> fpsCaps{24, 30, 40, 60, 75, 90, 100, 120};
    constexpr std::array<Quality,4> presets{Quality::Low,Quality::Medium,Quality::High,Quality::Ultra};
    template<typename T,size_t N> size_t closest(const std::array<T,N>& a, T x) {
        size_t best=0; for(size_t i=1;i<a.size();++i)
            if(std::abs(static_cast<int>(a[i])-static_cast<int>(x)) <
               std::abs(static_cast<int>(a[best])-static_cast<int>(x))) best=i;
        return best;
    }
    int indexResolution(const GraphicsSettings& s) {
        const auto& r=standardResolutions();
        for(int i=0;i<static_cast<int>(r.size());++i)
            if(r[i].width==s.renderWidth && r[i].height==s.renderHeight) return i;
        return 4; // start from 1080p when loading a custom size
    }
}
GraphicsMenu::GraphicsMenu(GraphicsSettings initial):settings_(initial) { validate(settings_); }
void GraphicsMenu::adjust(int dir) {
    switch(selected_) {
    case 0: {
        const auto& modes=standardResolutions();
        const int i=std::clamp(indexResolution(settings_)+dir,0,static_cast<int>(modes.size())-1);
        settings_.renderWidth=modes[i].width; settings_.renderHeight=modes[i].height;
        break;
    }
    case 1: {
        int i=static_cast<int>(closest(fpsCaps,settings_.fpsCap));
        settings_.fpsCap=fpsCaps[std::clamp(i+dir,0,static_cast<int>(fpsCaps.size())-1)];
        break;
    }
    case 2: {
        int i=static_cast<int>(std::find(presets.begin(),presets.end(),settings_.quality)-presets.begin());
        if(i==static_cast<int>(presets.size()))i=2; // Custom -> High
        applyPreset(settings_,presets[std::clamp(i+dir,0,static_cast<int>(presets.size())-1)]);
        break;
    }
    case 3: settings_.resolutionScale=std::clamp(settings_.resolutionScale+dir*5,50,100);break;
    case 4: settings_.vsync=!settings_.vsync;break;
    case 5: settings_.fpsOverlay=!settings_.fpsOverlay;break;
    case 6: settings_.adaptiveResolution=!settings_.adaptiveResolution;break;
    }
    validate(settings_);
}
bool GraphicsMenu::dispatch(MenuAction action) {
    if(action==MenuAction::Menu) {open_=!open_;return false;}
    if(action==MenuAction::Back) {open_=false;return false;} // Never close the application
    if(!open_) return false;
    if(action==MenuAction::Up) selected_=(selected_+rowCount-1)%rowCount;
    if(action==MenuAction::Down) selected_=(selected_+1)%rowCount;
    if(action==MenuAction::Left || action==MenuAction::Right || action==MenuAction::Confirm) {
        const std::string before=serialize(settings_);
        adjust(action==MenuAction::Left?-1:1);
        return before!=serialize(settings_);
    }
    return false;
}
std::vector<std::string> GraphicsMenu::lines() const {
    const auto s=settings_; const auto internal=internalSize(s);
    const auto str=[](int i){return std::to_string(i);};
    return {
        "Resolution target:  " + str(s.renderWidth) + " x " + str(s.renderHeight),
        "Frame cap:          " + str(s.fpsCap) + " FPS",
        "Quality preset:     " + qualityName(s.quality),
        "Internal scale:     " + str(s.resolutionScale) + "% (" + str(internal.width) + " x " + str(internal.height) + ")",
        std::string("VSync:              ")+(s.vsync?"On":"Off"),
        std::string("FPS overlay:        ")+(s.fpsOverlay?"On":"Off"),
        std::string("Adaptive scaling:   ")+(s.adaptiveResolution?"On (not implemented)":"Off")
    };
}
}
