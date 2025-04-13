#include <SkyrimScripting/Plugin.h>

#include "EventSink.h"
#include "ImGuiHooks.h"
#include "KeyboardShortcuts.h"

SKSEPlugin_Entrypoint {
    // Register the DXGI and D3D hooks right away
    RenderManager::Install();
}

SKSEPlugin_OnInputLoaded {
    // Register the keyboard listerner when the input system is loaded
    RE::BSInputDeviceManager::GetSingleton()->AddEventSink(EventSink::GetSingleton());

    // And register the keyboard shortcuts
    KeyboardShortcuts::OnKeyDown(DX_SCAN_CODE::PAGE_UP, []() {
        Log("Toggling the UI");
        ToggleUI();
        if (IsUIVisible()) {
            ShowMouseCursor(true);
        } else {
            ShowMouseCursor(false);
        }
    });
}
