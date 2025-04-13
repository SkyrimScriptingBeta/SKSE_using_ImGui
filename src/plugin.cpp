#include "plugin.h"

#include <SkyrimScripting/Plugin.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT
ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Helper function to write the thunk call
namespace stl {
    template <class T>
    void write_thunk_call() {
        auto&                                 trampoline = SKSE::GetTrampoline();
        const REL::Relocation<std::uintptr_t> hook{REL::ID(T::id), T::offset};
        T::func = trampoline.write_call<5>(hook.address(), T::thunk);
    }
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
    WndProcHook::func = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(
        sd.OutputWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcHook::thunk)
    ));

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

// Draw function - here's where you define your UI
void RenderManager::Draw() {
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

SKSEPlugin_Entrypoint {
    Log("Setting up RenderManager...");

    SKSE::AllocTrampoline(14 * 2);

    // Write the thunk calls
    stl::write_thunk_call<RenderManager::D3DInitHook>();
    stl::write_thunk_call<RenderManager::DXGIPresentHook>();

    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* msg) {
        if (msg->type == SKSE::MessagingInterface::kDataLoaded &&
            RenderManager::D3DInitHook::initialized) {
            ImGui::GetIO().MouseDrawCursor = true;
        }
    });

    Log("RenderManager installed");
}
