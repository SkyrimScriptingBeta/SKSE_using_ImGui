#include "EventSink.h"

#include <SkyrimScripting/Logging.h>

EventSink* EventSink::GetSingleton() {
    static EventSink singleton;
    return std::addressof(singleton);
}

RE::BSEventNotifyControl EventSink::ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*) {
    if (eventPtr && *eventPtr) {
        auto event = *eventPtr;
        if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kButton) {
            auto* buttonEvent = event->AsButtonEvent();
            if (buttonEvent->IsDown()) {
                auto dxScanCode = buttonEvent->GetIDCode();
                auto it         = _onKeyDownCallbacks.find(dxScanCode);
                if (it != _onKeyDownCallbacks.end()) {
                    Log("Calling {} key down handlers for scan code {}", it->second.size(), dxScanCode);
                    for (const auto& callback : it->second) {
                        callback();
                    }
                }
            }
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}
