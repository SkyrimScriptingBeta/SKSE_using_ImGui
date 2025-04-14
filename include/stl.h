#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

namespace stl {
    template <class T>
    void write_thunk_call() {
        auto& trampoline = SKSE::GetTrampoline();
#ifdef ENABLE_SKYRIM_VR
        const REL::Relocation<std::uintptr_t> hook{T::vr_address};
#else
        const REL::Relocation<std::uintptr_t> hook{REL::ID(T::id), T::offset};
#endif
        T::func = trampoline.write_call<5>(hook.address(), T::thunk);
    }
}
