#include "HUDComponent.h"
#include "HUDLayout.h"

HUDComponent::HUDComponent(HUDComponentType type, std::initializer_list<HUDPatch::Call> patches)
    : mType(type), mPatches(patches)
{
}

HUDComponentType HUDComponent::GetType() const
{
    return mType;
}

HUDComponent::HUDComponent()
    :
    mType(HUDComponentType::Health),
    mPatches({})
{
}

bool HUDComponent::IsVisible() const
{
    return mPatches.IsVisible();
}

void HUDComponent::ShowFor(uint32_t milliseconds)
{
    mTimer.ShowFor(milliseconds);
}

void HUDComponent::Hide()
{
    mTimer.Hide();
    mPatches.Hide();
}

void HUDComponent::Reset()
{
    Hide();
}

void HUDComponent::Show()
{
    mTimer.Show();
}

void HUDComponent::Update()
{
    if (mTimer.Visible())
        mPatches.Show();
    else
        mPatches.Hide();
}