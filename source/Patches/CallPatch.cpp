#include "CallPatch.h"

#include <plugin.h>

using namespace plugin;

CallPatch::CallPatch(const HUDPatch::Call& patch)
    :
    m_address(patch.Address),
    m_relativeCall(patch.RelativeCall),
    m_enabled(true)
{
    Disable();
}

void CallPatch::Enable()
{
    if (m_enabled)
        return;

    patch::SetUChar(m_address, 0xE8);
    patch::SetInt(m_address + 1, m_relativeCall);

    m_enabled = true;
}

void CallPatch::Disable()
{
    if (!m_enabled)
        return;

    patch::Nop(m_address, 5);

    m_enabled = false;
}

bool CallPatch::IsEnabled() const
{
    return m_enabled;
}