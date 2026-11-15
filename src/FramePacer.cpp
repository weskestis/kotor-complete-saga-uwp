// SPDX-License-Identifier: GPL-3.0-or-later
#include "FramePacer.hpp"
#include <algorithm>
#include <thread>
namespace kotor::video {
void FramePacer::reset() {deadline_={};statsStart_={};frames_=0;observedFps_=0;}
void FramePacer::setCap(int fps) noexcept {cap_=std::clamp(fps,24,120);}
std::chrono::nanoseconds FramePacer::frameInterval() const noexcept {
    return std::chrono::nanoseconds(1000000000LL/cap_);
}
void FramePacer::wait() {
    const auto now=Clock::now();
    if(statsStart_==Clock::time_point{})statsStart_=now;
    ++frames_;
    const auto elapsed=now-statsStart_;
    if(elapsed>=std::chrono::seconds(1)) {
        observedFps_=static_cast<float>(frames_)/std::chrono::duration<float>(elapsed).count();
        frames_=0;statsStart_=now;
    }
    if(deadline_==Clock::time_point{})deadline_=now;
    deadline_+=frameInterval();
    if(deadline_>now)std::this_thread::sleep_until(deadline_);
    // Avoid catch-up bursts after VSync blocking, suspend or rendering stalls.
    if(Clock::now()-deadline_>frameInterval())deadline_=Clock::now();
}
}
