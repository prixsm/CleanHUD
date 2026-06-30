#pragma once

#include <cstdint>
#include "HUDPatch.h"

class CallPatch
{
public:
    CallPatch(const HUDPatch::Call& patch);

    void Enable();
    void Disable();

    bool IsEnabled() const;

private:
    uintptr_t m_address;
    int32_t   m_relativeCall;
    bool      m_enabled;
};