#include "GraphicsSettings.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
using namespace kotor::video;
int main(){
    GraphicsSettings s=defaults();
    assert(s.renderWidth==3840 && s.renderHeight==2160 && s.fpsCap==120);
    assert(internalSize(s).width==3840 && internalSize(s).height==2160);
    s.resolutionScale=75;
    assert(internalSize(s).width==2880 && internalSize(s).height==1620);
    s.fpsCap=999;s.renderWidth=9999;s.renderHeight=9999;s.resolutionScale=120;
    validate(s);
    assert(s.fpsCap==120 && s.renderWidth==3840 && s.renderHeight==2160 && s.resolutionScale==100);
    s.renderWidth=1920;s.renderHeight=1080;s.fpsCap=97;s.vsync=false;
    s.quality=Quality::Custom;s.shadows=false;s.anisotropy=8;
    const auto encoded=serialize(s);const auto decoded=deserialize(encoded);
    assert(decoded.renderWidth==1920 && decoded.renderHeight==1080);
    assert(decoded.fpsCap==97 && decoded.vsync==false && decoded.shadows==false);
    assert(decoded.anisotropy==8 && decoded.quality==Quality::Custom);
    auto bad=deserialize("render_width=garbage\nfps_cap=-40\nresolution_scale=0\n");
    assert(bad.renderWidth==3840 && bad.fpsCap==24 && bad.resolutionScale==50);
    applyPreset(s,Quality::Low);
    assert(!s.shadows && !s.ssao && s.textureQuality==2);
    applyPreset(s,Quality::Ultra);
    assert(s.shadows && s.ssao && s.textureQuality==0);
    assert(reoneOptions(s).find("anisofilter=4")!=std::string::npos);
    DisplayMode connected{1920,1080,60,false};
    auto w=capabilityWarnings(defaults(),connected);
    assert(w.size()==4);
    DisplayMode high{3840,2160,120,false};
    assert(capabilityWarnings(defaults(),high).empty());
    assert(standardResolutions().back().width==3840);
    std::cout<<"All graphics settings tests passed.\n";
}
