// SPDX-License-Identifier: GPL-3.0-or-later
#include "GraphicsMenu.hpp"
#include "FramePacer.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
using namespace kotor::video;
int main() {
    auto s=defaults();GraphicsMenu m(s);
    assert(!m.visible());
    assert(!m.dispatch(MenuAction::Right)); // menu must not change when closed
    m.dispatch(MenuAction::Menu);assert(m.visible());
    auto first=m.settings().renderWidth;
    assert(m.dispatch(MenuAction::Left));assert(m.settings().renderWidth<first);
    assert(!m.dispatch(MenuAction::Up));assert(m.selected()==6);
    assert(m.dispatch(MenuAction::Right));assert(m.settings().adaptiveResolution);
    m.dispatch(MenuAction::Back);assert(!m.visible());
    assert(!m.dispatch(MenuAction::Back)); // B never quits app
    m.dispatch(MenuAction::Menu);m.dispatch(MenuAction::Down);
    assert(m.selected()==0); // selection retained and wraps
    FramePacer p;p.setCap(500);assert(p.cap()==120);assert(p.frameInterval().count()>8000000);
    p.setCap(30);assert(p.frameInterval().count()>33000000);
    p.setCap(1);assert(p.cap()==24);p.reset();
    auto l=m.lines();assert(l.size()==GraphicsMenu::rowCount);
    std::cout<<"Host/menu/frame cap tests passed\n";
}
