#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <d3d11.h>
#include <dxgi.h>

#include <atomic>

#include "SE_AE_VR.h"

/*
    SE, AE, VR                                    SE             AE            VR

    builder->AddCall<D3DInitHook, 5, 14>          (75595, 0x9,   77226, 0x275,  0xDC5530, 0x9);
    builder->AddCall<DXGIPresentHook, 5, 14>      (75461, 0x9,   77246, 0x9,    0xDBBDD0, 0x15);
    builder->AddCall<ProcessInputQueueHook, 5, 14>(67315, 0x7B,  68617, 0x7B,   0xC519E0, 0x81);
*/

void ShowUI(bool show);
void ToggleUI();
bool IsUIVisible();
void ShowMouseCursor(bool show);

struct RenderManager {
    // Main installation function that sets up all hooks
    static void Install();

    // Window procedure hook to handle ImGui input
    struct WndProcHook {
        static LRESULT        thunk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static inline WNDPROC func;
    };

    // DirectX 11 initialization hook
    struct D3DInitHook {
        static void                                    thunk();
        static inline REL::Relocation<decltype(thunk)> func;

        // SE and AE addresses for the D3D initialization function
        static constexpr auto id     = REL::ID(SE_AE_VR(75595, 77226, 0xDC5530));
        static constexpr auto offset = SE_AE_VR(0x9, 0x275, 0x9);

        static inline std::atomic<bool> initialized = false;
    };

    // SwapChain Present hook for rendering ImGui
    struct DXGIPresentHook {
        static void                                    thunk(std::uint32_t a_p1);
        static inline REL::Relocation<decltype(thunk)> func;

        // SE and AE addresses for the Present function
        static constexpr auto id     = REL::ID(SE_AE_VR(75461, 77246, 0xDBBDD0));
        static constexpr auto offset = SE_AE_VR(0x9, 0x9, 0x15);
    };

    // Called each frame to render ImGui content
    static void Draw();

    // DirectX device and context pointers
    static inline ID3D11Device*        device  = nullptr;
    static inline ID3D11DeviceContext* context = nullptr;

    // Prevent instantiation
    RenderManager() = delete;
};