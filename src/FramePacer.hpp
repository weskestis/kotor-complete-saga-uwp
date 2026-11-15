// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <chrono>

namespace kotor::video {
class FramePacer {
public:
    void reset();
    void setCap(int fps) noexcept;
    int cap() const noexcept {return cap_;}
    std::chrono::nanoseconds frameInterval() const noexcept;
    void wait();
    float observedFps() const noexcept { return observedFps_; }
private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point deadline_{};
    Clock::time_point statsStart_{};
    int cap_{60};
    int frames_{0};
    float observedFps_{0.0f};
};
}
