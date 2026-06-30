#include "HUDComponent.h"

HUDComponent::HUDComponent(std::initializer_list<HUDPatch::Call> patches)
    : mElement(patches)
{
}

HUDComponent::HUDComponent()
    : mElement({})
{
}

bool HUDComponent::IsVisible() const
{
    return mElement.IsVisible();
}

void HUDComponent::ShowFor(uint32_t milliseconds)
{
    mTimer.ShowFor(milliseconds);
}

void HUDComponent::Hide()
{
    mElement.Hide();
}

void HUDComponent::Reset()
{
    mTimer.Reset();
    mElement.Hide();
}

void HUDComponent::Update()
{
    if (mTimer.Visible())
        mElement.Show();
    else
        mElement.Hide();
}