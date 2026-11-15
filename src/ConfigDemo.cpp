// Minimal portable validation utility; NOT an Xbox game or an MSIX.
#include "GraphicsSettings.hpp"
#include <iostream>
int main() {
    using namespace kotor::video;
    auto s=defaults();
    const auto scene=internalSize(s);
    std::cout<<"KOTOR Complete Saga / CP1A configuration core\n";
    std::cout<<"Default: "<<scene.width<<"x"<<scene.height<<", fps cap "<<s.fpsCap<<"\n\n";
    std::cout<<serialize(s)<<"\n";
    std::cout<<"Proposed reone config:\n"<<reoneOptions(s);
}
