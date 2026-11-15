// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GraphicsSettings.hpp"
#include <string>
#include <vector>

namespace kotor::video {
enum class MenuAction { Menu, Back, Up, Down, Left, Right, Confirm };
class GraphicsMenu {
public:
    explicit GraphicsMenu(GraphicsSettings initial);
    bool dispatch(MenuAction action); // returns true if graphic parameters changed
    bool visible() const noexcept { return open_; }
    int selected() const noexcept { return selected_; }
    const GraphicsSettings& settings() const noexcept { return settings_; }
    std::vector<std::string> lines() const;
    static constexpr int rowCount = 7;
private:
    void adjust(int direction);
    GraphicsSettings settings_;
    bool open_{false};
    int selected_{0};
};
}
