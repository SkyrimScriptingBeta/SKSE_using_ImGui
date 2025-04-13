#pragma once

#include <functional>

#include "DxScanCodes.h"

namespace KeyboardShortcuts {
    void        OnKeyDown(DX_SCAN_CODE_ID scanCode, std::function<void()> callback);
    inline void OnKeyDown(DX_SCAN_CODE scanCode, std::function<void()> callback) { OnKeyDown(static_cast<DX_SCAN_CODE_ID>(scanCode), callback); }
}
