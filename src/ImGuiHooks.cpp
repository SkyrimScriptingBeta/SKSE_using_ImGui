#include "ImGuiHooks.h"

#include <SkyrimScripting/Logging.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <atomic>

#include "DxScanCodes.h"
#include "stl.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::atomic<bool> _showUI{false};

void ShowUI(bool show) {
    // Set the UI visibility flag
    _showUI.store(show);
}

void ToggleUI() {
    // Toggle the UI visibility flag
    _showUI.store(!_showUI.load());
}

bool IsUIVisible() {
    // Return the current UI visibility state
    return _showUI.load();
}

void ShowMouseCursor(bool show) {
    // Set the mouse cursor visibility in ImGui
    ImGui::GetIO().MouseDrawCursor = show;
}

void RenderManager::Install() {
    Log("Installing RenderManager hooks...");

    SKSE::AllocTrampoline(14 * 3);

    // Write the thunk calls
    stl::write_thunk_call<RenderManager::D3DInitHook>();
    stl::write_thunk_call<RenderManager::DXGIPresentHook>();
    stl::write_thunk_call<RenderManager::ProcessInputQueueHook>();

    Log("RenderManager hooks installed");
}

// WndProc hook implementation to handle ImGui input
LRESULT RenderManager::WndProcHook::thunk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Forward events to ImGui
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) return true;

    // Reset input when focus is lost
    if (uMsg == WM_KILLFOCUS) {
        ImGui::GetIO().ClearInputCharacters();
        ImGui::GetIO().ClearInputKeys();
    }

    // Call the original window procedure
    return func(hWnd, uMsg, wParam, lParam);
}

// D3D initialization hook implementation
void RenderManager::D3DInitHook::thunk() {
    // Call the original function first
    func();

    Log("RenderManager: Initializing...");

    // Get the render manager and data
    const auto* renderer = RE::BSGraphics::Renderer::GetSingleton();
    if (!renderer) {
        Error("Cannot find render. Initialization failed!");
        return;
    }

    const auto swapChain = (IDXGISwapChain*)renderer->data.renderWindows[0].swapChain;
    if (!swapChain) {
        SKSE::log::error("couldn't find swapChain");
        return;
    }

    // Get the swap chain description
    Log("Getting swapchain desc...");
    DXGI_SWAP_CHAIN_DESC sd{};
    if (swapChain->GetDesc(std::addressof(sd)) < 0) {
        Error("IDXGISwapChain::GetDesc failed.");
        return;
    }

    // Store device and context
    DXGI_SWAP_CHAIN_DESC desc{};
    if (FAILED(swapChain->GetDesc(std::addressof(desc)))) {
        SKSE::log::error("IDXGISwapChain::GetDesc failed.");
        return;
    }
    const auto device  = (ID3D11Device*)renderer->data.forwarder;
    const auto context = (ID3D11DeviceContext*)renderer->data.context;

    // Initialize ImGui
    Log("Initializing ImGui...");
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Set up ImGui configuration
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable keyboard controls

    // Initialize ImGui platform backends
    if (!ImGui_ImplWin32_Init(sd.OutputWindow)) {
        Error("ImGui initialization failed (Win32)");
        return;
    }

    if (!ImGui_ImplDX11_Init(device, context)) {
        Error("ImGui initialization failed (DX11)");
        return;
    }

    Log("ImGui initialized successfully");

    // Hook the window procedure for input
    WndProcHook::func = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(sd.OutputWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcHook::thunk)));

    if (!WndProcHook::func) Error("SetWindowLongPtrA failed!");

    // Mark initialization as complete
    initialized.store(true);

    Log("RenderManager: Initialization complete");
}

// Present hook implementation for rendering ImGui
void RenderManager::DXGIPresentHook::thunk(std::uint32_t a_p1) {
    // Call the original function
    func(a_p1);

    // Don't render if initialization is not complete
    if (!D3DInitHook::initialized.load()) return;

    if (_showUI) {
        // Start a new ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Draw our UI

        Draw();

        // Render ImGui
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}

// Draw function - here's where you define your UI
void RenderManager::Draw() {
    if (!_showUI.load()) return;  // Don't draw if the UI is hidden

    // Create a simple window with "Hello, world!" and a button
    ImGui::Begin("SKSE ImGui Demo");

    ImGui::Text("Hello, world from ImGui in SKSE!");

    static bool buttonClicked = false;
    if (ImGui::Button("Click Me!")) {
        buttonClicked = !buttonClicked;
    }

    if (buttonClicked) {
        ImGui::Text("Button was clicked!");
    }

    ImGui::End();
}

void RenderManager::ProcessInputQueueHook::thunk(RE::BSTEventSink<RE::InputEvent*>* a_eventSink, RE::InputEvent* const* eventPtr) {
    if (_showUI) {
        if (eventPtr && *eventPtr) {
            auto event = *eventPtr;
            if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kButton) {
                auto* buttonEvent = event->AsButtonEvent();
                if (buttonEvent->IsDown()) {
                    auto dxScanCode = buttonEvent->GetIDCode();
                    if (dxScanCode == static_cast<DX_SCAN_CODE_ID>(DX_SCAN_CODE::PAGE_UP)) {  // hard coded here :)
                        // Hide the UI and mouse cursor
                        ShowMouseCursor(false);
                        ShowUI(false);
                    }
                }
            }
        }

        constexpr RE::InputEvent* const dummy[] = {nullptr};
        func(a_eventSink, dummy);
    } else {
        func(a_eventSink, eventPtr);
    }
}
