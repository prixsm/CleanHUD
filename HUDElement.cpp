#include "HUDElement.h"
#include "HUDPatch.h"

HUDElement::HUDElement(std::initializer_list<HUDPatch::Call> patches)
    : m_visible(false)
{
    for (auto& patch : patches)
        m_patches.emplace_back(patch);
}

void HUDElement::Show()
{
    if (m_visible)
        return;

    for (auto& patch : m_patches)
        patch.Enable();

    m_visible = true;
}

void HUDElement::Hide()
{
    if (!m_visible)
        return;

    for (auto& patch : m_patches)
        patch.Disable();

    m_visible = false;
}

bool HUDElement::IsVisible() const
{
    return m_visible;
}