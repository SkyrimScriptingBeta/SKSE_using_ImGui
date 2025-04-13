#include "KeyboardShortcuts.h"

#include "EventSink.h"

void KeyboardShortcuts::OnKeyDown(DX_SCAN_CODE_ID scanCode, std::function<void()> callback) { EventSink::GetSingleton()->RegisterKeyDownCallback(scanCode, callback); }
