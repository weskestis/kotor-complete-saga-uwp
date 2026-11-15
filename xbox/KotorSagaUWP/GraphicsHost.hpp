// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <d2d1_1.h>
#include <d3d11_1.h>
#include <dwrite.h>
#include <dxgi1_3.h>
#include <wrl/client.h>
#include <cstdint>
#include <string>
#include "../../src/GraphicsMenu.hpp"

namespace kotor::xbox {
// CP1C: real GPU offscreen target + scaled swap-chain output, NO KOTOR gameplay.
class GraphicsHost final {
public:
    void initialize(IUnknown* coreWindow);
    void resize();
    void draw(const video::GraphicsMenu& menu, float fps, double elapsedSeconds);
    void trim();
    uint32_t width() const noexcept { return width_; }
    uint32_t height() const noexcept { return height_; }
    uint32_t sceneWidth() const noexcept { return sceneWidth_; }
    uint32_t sceneHeight() const noexcept { return sceneHeight_; }
private:
    void createSizeDependentResources(bool resizeBuffers);
    void ensureSceneTarget(const video::GraphicsSettings& settings);
    void drawScene(double elapsedSeconds);
    void text(const std::wstring& value, float x, float y, bool selected=false);
    uint32_t width_{0}, height_{0}, sceneWidth_{0}, sceneHeight_{0};
    int requestedWidth_{-1}, requestedHeight_{-1};
    IUnknown* window_{nullptr}; // Borrowed: owned by CoreWindow for host lifetime.
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> sceneTexture_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneRtv_;
    Microsoft::WRL::ComPtr<ID2D1Factory1> d2dFactory_;
    Microsoft::WRL::ComPtr<ID2D1Device> d2dDevice_;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dContext_;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dTarget_;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> sceneBitmap_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> whiteBrush_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> greenBrush_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> grayBrush_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> darkBrush_;
    Microsoft::WRL::ComPtr<IDWriteFactory> dwrite_;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> typeface_;
};
} // namespace kotor::xbox
