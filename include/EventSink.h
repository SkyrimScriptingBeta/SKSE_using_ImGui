#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <functional>
#include <unordered_map>
#include <vector>

class EventSink : public RE::BSTEventSink<RE::InputEvent*> {
    std::unordered_map<std::uint32_t, std::vector<std::function<void()>>> _onKeyDownCallbacks;

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*) override;

public:
    static EventSink* GetSingleton();

    static void InstallHooks();

    inline void RegisterKeyDownCallback(std::uint32_t scanCode, std::function<void()> callback) { _onKeyDownCallbacks[scanCode].push_back(callback); }
};
