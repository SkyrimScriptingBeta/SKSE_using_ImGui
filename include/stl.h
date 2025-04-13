#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

namespace stl {
    template <class T>
    void write_thunk_call() {
        auto&                                 trampoline = SKSE::GetTrampoline();
        const REL::Relocation<std::uintptr_t> hook{REL::ID(T::id), T::offset};
        T::func = trampoline.write_call<5>(hook.address(), T::thunk);
    }
}
