// SPDX-License-Identifier: GPL-3.0-or-later
// C++/CX Windows.Universal DirectX CoreWindow entry point (no Win32/desktop process).
#include "GraphicsHost.hpp"
#include "../../src/FramePacer.hpp"
#include "../../src/GraphicsMenu.hpp"
#include <Windows.h>
#include <collection.h>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <sstream>
#include <iomanip>
#include <exception>

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Gaming::Input;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::System;
using namespace Windows::UI::Core;
using namespace kotor;

namespace {
    std::filesystem::path configPath() {
        const auto base=ApplicationData::Current->LocalFolder->Path;
        return std::filesystem::path(base->Data()) / L"graphics.cfg";
    }
    video::GraphicsSettings loadSettings() {
        // Conservative first-boot setting; 4K and 120 FPS selectable in the menu.
        auto settings=video::defaults();
        settings.renderWidth=1920;settings.renderHeight=1080;
        settings.fpsCap=60;
        video::applyPreset(settings,video::Quality::Medium);
        try {
            std::ifstream input(configPath(),std::ios::binary);
            if(input) {
                const std::string contents(std::istreambuf_iterator<char>{input},{});
                settings=video::deserialize(contents);
            }
        } catch(const std::exception&) {
            OutputDebugStringW(L"KotorSaga: failed to read config; using defaults\n");
        }
        return settings;
    }
    void saveSettings(const video::GraphicsSettings& s) {
        try {
            std::ofstream output(configPath(),std::ios::trunc|std::ios::binary);
            output<<video::serialize(s);
        } catch(const std::exception&) {
            OutputDebugStringW(L"KotorSaga: failed to write graphics config\n");
        }
    }
    // All diagnostic files remain inside the app's sandbox (LocalState).
    // No stdout or remote server is required to capture a failed launch.
    void diagnostic(const std::string& message) noexcept {
        try {
            std::ofstream log(configPath().parent_path()/L"diagnostics.log",std::ios::app);
            log<<message<<'\n';
        } catch(...) {} // Never permit logging to crash a running game.
        OutputDebugStringA(("KotorSaga: "+message+"\n").c_str());
    }
    unsigned bits(GamepadButtons button) {return static_cast<unsigned>(button);}
}

ref class SagaView sealed : IFrameworkView {
internal:
    SagaView():menu_(loadSettings()) {
        pacer_.setCap(menu_.settings().fpsCap);
        diagnostic("CP1D / 0.1.3: game host created");
    }
public:
    virtual void Initialize(CoreApplicationView^ view) {
        view->Activated+=ref new TypedEventHandler<CoreApplicationView^,IActivatedEventArgs^>(
            this,&SagaView::onActivated);
        CoreApplication::Suspending+=ref new EventHandler<SuspendingEventArgs^>(this,&SagaView::onSuspending);
        CoreApplication::Resuming+=ref new EventHandler<Platform::Object^>(this,&SagaView::onResuming);
    }
    virtual void SetWindow(CoreWindow^ window) {
        window_=window;
        window->Closed+=ref new TypedEventHandler<CoreWindow^,CoreWindowEventArgs^>(this,&SagaView::onClosed);
        window->VisibilityChanged+=ref new TypedEventHandler<CoreWindow^,VisibilityChangedEventArgs^>(
            this,&SagaView::onVisibility);
        window->SizeChanged+=ref new TypedEventHandler<CoreWindow^,WindowSizeChangedEventArgs^>(
            this,&SagaView::onSizeChanged);
        window->KeyDown+=ref new TypedEventHandler<CoreWindow^,KeyEventArgs^>(this,&SagaView::onKeyDown);
    }
    virtual void Load(Platform::String^) {
        try {
            renderer_=std::make_unique<xbox::GraphicsHost>();
            renderer_->initialize(reinterpret_cast<IUnknown*>(window_));
            diagnostic("Direct3D 11 / Direct2D initialized");
        } catch(const std::exception& e) {
            diagnostic(std::string("FATAL initialize: ")+e.what());
            renderer_.reset(); closed_=true;
        } catch(...) {
            diagnostic("FATAL initialize: unknown error");
            renderer_.reset(); closed_=true;
        }
    }
    virtual void Run() {
        const auto start=std::chrono::steady_clock::now();
        while(!closed_) {
            window_->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            if(!visible_) {
                pacer_.reset();
                window_->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
                continue;
            }
            pollGamepad();
            const auto now=std::chrono::steady_clock::now();
            const double elapsed=std::chrono::duration<double>(now-start).count();
            try {
                renderer_->draw(menu_,pacer_.observedFps(),elapsed);
                const auto requested=video::internalSize(menu_.settings());
                if(requested.width!=lastRequestedWidth_ || requested.height!=lastRequestedHeight_
                   || renderer_->sceneWidth()!=lastSceneWidth_
                   || renderer_->sceneHeight()!=lastSceneHeight_) {
                    lastRequestedWidth_=requested.width;lastRequestedHeight_=requested.height;
                    lastSceneWidth_=renderer_->sceneWidth();lastSceneHeight_=renderer_->sceneHeight();
                    std::ostringstream info;
                    info<<"resolution requested="<<requested.width<<"x"<<requested.height
                        <<", allocated="<<lastSceneWidth_<<"x"<<lastSceneHeight_
                        <<", backbuffer="<<renderer_->width()<<"x"<<renderer_->height();
                    diagnostic(info.str());
                }
                if(lastMetricsAt_==std::chrono::steady_clock::time_point{} ||
                    now-lastMetricsAt_>=std::chrono::seconds(10)) {
                    std::ostringstream metrics;
                    metrics<<"frame pacing cap="<<menu_.settings().fpsCap
                        <<" actual_loop_fps="<<static_cast<int>(pacer_.observedFps()+0.5f)
                        <<" vsync="<<(menu_.settings().vsync?"on":"off");
                    diagnostic(metrics.str());
                    lastMetricsAt_=now;
                }
                pacer_.wait();
            } catch(const std::exception& e) {
                diagnostic(std::string("FATAL render: ")+e.what());
                closed_=true;
            } catch(...) {
                diagnostic("FATAL render: unknown error");
                closed_=true;
            }
        }
        saveSettings(menu_.settings());
        diagnostic("App Run() completed");
    }
    virtual void Uninitialize() {renderer_.reset();}
private:
    void action(video::MenuAction a) {
        if(menu_.dispatch(a)) {
            pacer_.setCap(menu_.settings().fpsCap);
            pacer_.reset();
            saveSettings(menu_.settings());
        }
    }
    void pollGamepad() {
        const auto devices=Gamepad::Gamepads;
        if(devices->Size==0){previousButtons_=0;return;}
        const auto reading=devices->GetAt(0)->GetCurrentReading();
        const unsigned current=static_cast<unsigned>(reading.Buttons);
        const unsigned pressed=current&~previousButtons_;
        previousButtons_=current;
        if(pressed&bits(GamepadButtons::Menu))action(video::MenuAction::Menu);
        if(pressed&bits(GamepadButtons::B))action(video::MenuAction::Back);
        if(pressed&bits(GamepadButtons::DPadUp))action(video::MenuAction::Up);
        if(pressed&bits(GamepadButtons::DPadDown))action(video::MenuAction::Down);
        if(pressed&bits(GamepadButtons::DPadLeft))action(video::MenuAction::Left);
        if(pressed&bits(GamepadButtons::DPadRight))action(video::MenuAction::Right);
        if(pressed&bits(GamepadButtons::A))action(video::MenuAction::Confirm);
    }
    void onKeyDown(CoreWindow^,KeyEventArgs^ e) {
        switch(e->VirtualKey) {
        case VirtualKey::F1: action(video::MenuAction::Menu);break;
        case VirtualKey::Escape: action(video::MenuAction::Back);break;
        case VirtualKey::Up: action(video::MenuAction::Up);break;
        case VirtualKey::Down: action(video::MenuAction::Down);break;
        case VirtualKey::Left: action(video::MenuAction::Left);break;
        case VirtualKey::Right: action(video::MenuAction::Right);break;
        case VirtualKey::Enter: action(video::MenuAction::Confirm);break;
        default: break;
        }
    }
    void onActivated(CoreApplicationView^,IActivatedEventArgs^) {window_->Activate();}
    void onClosed(CoreWindow^,CoreWindowEventArgs^) {closed_=true;}
    void onVisibility(CoreWindow^,VisibilityChangedEventArgs^ e) {visible_=e->Visible;}
    void onSizeChanged(CoreWindow^,WindowSizeChangedEventArgs^) {
        if(!renderer_)return;
        try {renderer_->resize();diagnostic("CoreWindow resized");}
        catch(const std::exception& e) {diagnostic(std::string("FATAL resize: ")+e.what());closed_=true;}
    }
    void onSuspending(Platform::Object^,SuspendingEventArgs^) {
        saveSettings(menu_.settings());
        if(renderer_)renderer_->trim();
    }
    void onResuming(Platform::Object^,Platform::Object^) {pacer_.reset();}

    CoreWindow^ window_{nullptr};
    std::unique_ptr<xbox::GraphicsHost> renderer_;
    video::GraphicsMenu menu_;
    video::FramePacer pacer_;
    unsigned previousButtons_{0};
    bool closed_{false};
    bool visible_{true};
    int lastRequestedWidth_{0},lastRequestedHeight_{0};
    uint32_t lastSceneWidth_{0},lastSceneHeight_{0};
    std::chrono::steady_clock::time_point lastMetricsAt_{};
};

ref class SagaViewSource sealed : IFrameworkViewSource {
public:
    virtual IFrameworkView^ CreateView() {return ref new SagaView();}
};

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^) {
    CoreApplication::Run(ref new SagaViewSource());
    return 0;
}
