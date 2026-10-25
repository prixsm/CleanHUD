#include "HUDPatch.h"
#include "HUDPatchGroup.h"

HUDPatchGroup::HUDPatchGroup(std::initializer_list<HUDPatch::Call> patches)
    : m_visible(false)
{
    for (auto& patch : patches)
        m_patches.emplace_back(patch);
}

void HUDPatchGroup::Show()
{
    if (m_visible)
        return;

    for (auto& patch : m_patches)
        patch.Enable();

    m_visible = true;
}

void HUDPatchGroup::Hide()
{
    if (!m_visible)
        return;

    for (auto& patch : m_patches)
        patch.Disable();

    m_visible = false;
}

bool HUDPatchGroup::IsVisible() const
{
    return m_visible;
}