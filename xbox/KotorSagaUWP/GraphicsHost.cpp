// SPDX-License-Identifier: GPL-3.0-or-later
#include "GraphicsHost.hpp"
#include <d2d1helper.h>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;
namespace kotor::xbox {
namespace {
    void check(HRESULT hr, const char* location) {
        if (FAILED(hr)) throw std::runtime_error(std::string(location)+" failed, HRESULT "+std::to_string(static_cast<uint32_t>(hr)));
    }
    std::wstring wide(const std::string& s) {return std::wstring(s.begin(),s.end());}
}
void GraphicsHost::initialize(IUnknown* coreWindow) {
    window_=coreWindow;
    constexpr D3D_FEATURE_LEVEL levels[]={D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0};
    D3D_FEATURE_LEVEL actual{};
    HRESULT hr=D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,levels,2,D3D11_SDK_VERSION,&device_,&actual,&context_);
    if(hr==E_INVALIDARG) hr=D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,levels+1,1,D3D11_SDK_VERSION,&device_,&actual,&context_);
    check(hr,"D3D11CreateDevice");
    ComPtr<IDXGIDevice2> dxgiDevice;
    check(device_.As(&dxgiDevice),"IDXGIDevice2");
    ComPtr<IDXGIAdapter> adapter;
    check(dxgiDevice->GetAdapter(&adapter),"DXGI adapter");
    ComPtr<IDXGIFactory2> factory;
    check(adapter->GetParent(IID_PPV_ARGS(&factory)),"DXGI factory");
    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width=0; desc.Height=0; // physical CoreWindow output; NOT the selected internal resolution
    desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count=1;
    desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount=2;
    desc.SwapEffect=DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.Scaling=DXGI_SCALING_STRETCH;
    desc.AlphaMode=DXGI_ALPHA_MODE_IGNORE;
    check(factory->CreateSwapChainForCoreWindow(device_.Get(),window_,&desc,nullptr,&swapChain_),"CreateSwapChainForCoreWindow");
    check(dxgiDevice->SetMaximumFrameLatency(1),"SetMaximumFrameLatency");
    check(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,&d2dFactory_),"D2D factory");
    check(d2dFactory_->CreateDevice(dxgiDevice.Get(),&d2dDevice_),"D2D device");
    check(d2dDevice_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,&d2dContext_),"D2D context");
    check(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(dwrite_.GetAddressOf())),"DirectWrite factory");
    check(dwrite_->CreateTextFormat(L"Segoe UI",nullptr,DWRITE_FONT_WEIGHT_SEMI_BOLD,
        DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,33.0f,L"en-us",&typeface_),"TextFormat");
    check(typeface_->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP),"WordWrapping");
    createSizeDependentResources(false);
}
void GraphicsHost::createSizeDependentResources(bool resizeBuffers) {
    if(d2dContext_) d2dContext_->SetTarget(nullptr);
    whiteBrush_.Reset();greenBrush_.Reset();grayBrush_.Reset();darkBrush_.Reset();
    d2dTarget_.Reset();renderTarget_.Reset();
    context_->OMSetRenderTargets(0,nullptr,nullptr);
    context_->Flush();
    if(resizeBuffers)check(swapChain_->ResizeBuffers(2,0,0,DXGI_FORMAT_B8G8R8A8_UNORM,0),"ResizeBuffers");
    ComPtr<ID3D11Texture2D> buffer;
    check(swapChain_->GetBuffer(0,IID_PPV_ARGS(&buffer)),"Backbuffer");
    D3D11_TEXTURE2D_DESC desc{}; buffer->GetDesc(&desc);
    width_=desc.Width;height_=desc.Height;
    check(device_->CreateRenderTargetView(buffer.Get(),nullptr,&renderTarget_),"Backbuffer RTV");
    ComPtr<IDXGISurface> surface;
    check(buffer.As(&surface),"Backbuffer DXGI surface");
    const auto props=D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET|D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_IGNORE),96,96);
    check(d2dContext_->CreateBitmapFromDxgiSurface(surface.Get(),&props,&d2dTarget_),"Backbuffer D2D bitmap");
    d2dContext_->SetTarget(d2dTarget_.Get());
    check(d2dContext_->CreateSolidColorBrush(D2D1::ColorF(0.97f,0.99f,0.98f),&whiteBrush_),"white brush");
    check(d2dContext_->CreateSolidColorBrush(D2D1::ColorF(0.13f,0.92f,0.37f),&greenBrush_),"green brush");
    check(d2dContext_->CreateSolidColorBrush(D2D1::ColorF(0.55f,0.70f,0.60f),&grayBrush_),"gray brush");
    check(d2dContext_->CreateSolidColorBrush(D2D1::ColorF(0.015f,0.04f,0.02f),&darkBrush_),"dark brush");
}
void GraphicsHost::resize() {
    if(!swapChain_)return;
    createSizeDependentResources(true);
}
void GraphicsHost::ensureSceneTarget(const video::GraphicsSettings& settings) {
    const auto desired=video::internalSize(settings);
    if(sceneTexture_ && desired.width==requestedWidth_ && desired.height==requestedHeight_)return;
    requestedWidth_=desired.width;requestedHeight_=desired.height;
    // Scene texture is a real, independently allocated GPU target. Its size changes with
    // user resolution AND internal scale; the swap chain continues to track CoreWindow.
    d2dContext_->SetTarget(nullptr);
    context_->OMSetRenderTargets(0,nullptr,nullptr);
    context_->Flush();
    sceneBitmap_.Reset();sceneRtv_.Reset();sceneTexture_.Reset();
    sceneWidth_=sceneHeight_=0;
    // If Dev Mode denies the requested allocation, use a safe smaller target and report it.
    std::vector<video::RenderSize> candidates{{desired.width,desired.height},{1920,1080},{1280,720},{640,360}};
    for(const auto& size:candidates) {
        if(size.width>desired.width || size.height>desired.height)continue;
        D3D11_TEXTURE2D_DESC d{};
        d.Width=size.width;d.Height=size.height;d.MipLevels=1;d.ArraySize=1;
        d.Format=DXGI_FORMAT_B8G8R8A8_UNORM;d.SampleDesc.Count=1;
        d.Usage=D3D11_USAGE_DEFAULT;d.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
        ComPtr<ID3D11Texture2D> texture;
        if(FAILED(device_->CreateTexture2D(&d,nullptr,&texture)))continue;
        ComPtr<ID3D11RenderTargetView> rtv;
        if(FAILED(device_->CreateRenderTargetView(texture.Get(),nullptr,&rtv)))continue;
        ComPtr<IDXGISurface> surface;
        if(FAILED(texture.As(&surface)))continue;
        ComPtr<ID2D1Bitmap1> bitmap;
        const auto props=D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_IGNORE),96,96);
        if(FAILED(d2dContext_->CreateBitmapFromDxgiSurface(surface.Get(),&props,&bitmap)))continue;
        sceneTexture_=texture;sceneRtv_=rtv;sceneBitmap_=bitmap;
        sceneWidth_=size.width;sceneHeight_=size.height;
        break;
    }
    d2dContext_->SetTarget(d2dTarget_.Get());
    if(!sceneTexture_) throw std::runtime_error("No offscreen GPU render target could be allocated");
}
void GraphicsHost::text(const std::wstring& s,float x,float y,bool selected) {
    const auto rc=D2D1::RectF(x,y,1850.0f,y+46.0f);
    d2dContext_->DrawText(s.c_str(),static_cast<UINT32>(s.size()),typeface_.Get(),rc,
        selected?greenBrush_.Get():whiteBrush_.Get(),D2D1_DRAW_TEXT_OPTIONS_CLIP);
}
void GraphicsHost::drawScene(double elapsedSeconds) {
    // CP1D diagnostic scene only: offscreen Direct3D target, 2D geometry overlay, no KOTOR models.
    const float pulse=static_cast<float>(0.025*std::sin(elapsedSeconds*1.4));
    const float clear[]={0.005f,0.038f+pulse,0.016f,1.0f};
    context_->OMSetRenderTargets(1,sceneRtv_.GetAddressOf(),nullptr);
    context_->ClearRenderTargetView(sceneRtv_.Get(),clear);
    d2dContext_->SetTarget(sceneBitmap_.Get());
    d2dContext_->BeginDraw();
    d2dContext_->SetTransform(D2D1::Matrix3x2F::Scale(
        static_cast<float>(sceneWidth_)/1920.0f,static_cast<float>(sceneHeight_)/1080.0f));
    const float shift=static_cast<float>(65.0*std::sin(elapsedSeconds*0.8));
    for(int i=0;i<26;i++) {
        const float x=80.0f+i*72.0f+shift;
        d2dContext_->DrawLine(D2D1::Point2F(x,0),D2D1::Point2F(x-320,1080),grayBrush_.Get(),1.6f);
    }
    for(int i=0;i<13;i++) {
        const float y=40.0f+i*85.0f;
        d2dContext_->DrawLine(D2D1::Point2F(0,y),D2D1::Point2F(1920,y),grayBrush_.Get(),1.3f);
    }
    const float cx=960.0f+static_cast<float>(180.0*std::sin(elapsedSeconds));
    const float cy=565.0f+static_cast<float>(100.0*std::cos(elapsedSeconds));
    d2dContext_->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx,cy),42,42),greenBrush_.Get());
    text(L"GPU OFFSCREEN SCENE PROBE / NOT GAMEPLAY",85,840,true);
    check(d2dContext_->EndDraw(),"Offscreen EndDraw");
    d2dContext_->SetTarget(d2dTarget_.Get());
}
void GraphicsHost::draw(const video::GraphicsMenu& menu,float fps,double elapsedSeconds) {
    ensureSceneTarget(menu.settings());
    drawScene(elapsedSeconds);
    const auto& settings=menu.settings();
    d2dContext_->SetTarget(d2dTarget_.Get());
    d2dContext_->BeginDraw();
    d2dContext_->SetTransform(D2D1::Matrix3x2F::Identity());
    d2dContext_->Clear(D2D1::ColorF(0.0f,0.01f,0.0f,1.0f));
    d2dContext_->DrawBitmap(sceneBitmap_.Get(),D2D1::RectF(0,0,static_cast<float>(width_),static_cast<float>(height_)),
        1.0f,D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    d2dContext_->SetTransform(D2D1::Matrix3x2F::Scale(
        static_cast<float>(width_)/1920.0f,static_cast<float>(height_)/1080.0f));
    d2dContext_->FillRectangle(D2D1::RectF(45,28,1870,425),darkBrush_.Get());
    text(L"KOTOR COMPLETE SAGA / XBOX CP1D",90,60,true);
    text(L"NATIVE DIRECT3D 11 RENDER TARGET PROBE",90,115);
    text(L"DISPLAY BACKBUFFER: " + std::to_wstring(width_) + L" x " + std::to_wstring(height_),90,173);
    text(L"REQUESTED INTERNAL: " + std::to_wstring(requestedWidth_) + L" x " + std::to_wstring(requestedHeight_),90,225);
    text(L"ALLOCATED INTERNAL: " + std::to_wstring(sceneWidth_) + L" x " + std::to_wstring(sceneHeight_),90,278);
    text(L"TARGET CAP: " + std::to_wstring(settings.fpsCap) + L" FPS / VSYNC: " +(settings.vsync?L"ON":L"OFF"),90,331);
    if(settings.fpsOverlay)text(L"MEASURED LOOP: " + std::to_wstring(static_cast<int>(std::lround(fps))) + L" FPS",90,384,true);
    if(menu.visible()) {
        d2dContext_->FillRectangle(D2D1::RectF(45,443,1870,960),darkBrush_.Get());
        auto lines=menu.lines();
        for(int i=0;i<static_cast<int>(lines.size());++i) {
            const float y=473.0f+i*62.0f;
            if(i==menu.selected())d2dContext_->FillRectangle(D2D1::RectF(69,y-5,1830,y+45),greenBrush_.Get());
            const auto value=wide(lines[i]);
            const auto rc=D2D1::RectF(95,y,1810,y+50);
            d2dContext_->DrawText(value.c_str(),static_cast<UINT32>(value.size()),typeface_.Get(),rc,
                i==menu.selected()?darkBrush_.Get():whiteBrush_.Get());
        }
        text(L"DPAD: NAVIGATE     A: INCREASE     B: BACK     MENU: CLOSE",80,980);
    } else {
        text(L"MENU: GRAPHICS    /    B: BACK, NEVER QUIT",80,960);
        text(L"4K/120 SELECTION DOES NOT GUARANTEE DISPLAY OR GAME PERFORMANCE",80,1015);
    }
    check(d2dContext_->EndDraw(),"Backbuffer EndDraw");
    check(swapChain_->Present(settings.vsync?1:0,0),"DXGI Present");
}
void GraphicsHost::trim() {
    ComPtr<IDXGIDevice3> dxgi;
    if(SUCCEEDED(device_.As(&dxgi)))dxgi->Trim();
}
} // namespace kotor::xbox
