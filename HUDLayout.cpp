#include <algorithm>

#include "HUDComponent.h"
#include "HUDLayout.h"
#include "HUDSettings.h"

namespace
{
    // Ordering is independent of HUDComponentType's enum values.
    constexpr std::array<HUDComponentType, 8> StackOrder = {
        HUDComponentType::Clock,
        HUDComponentType::Money,
        HUDComponentType::Wanted,
        HUDComponentType::Weapon,
        HUDComponentType::Ammo,
        HUDComponentType::Armour,
        HUDComponentType::Breath,
        HUDComponentType::Health
    };

    std::size_t StackIndex(HUDComponentType type)
    {
        const auto found =
            std::find(StackOrder.begin(), StackOrder.end(), type);

        return static_cast<std::size_t>(
            found - StackOrder.begin());
    }
}

void HUDLayout::Reset()
{
    // Remember last frame's rendered entries before rebuilding.
    mPreviouslyPresent.fill(false);

    for (const auto& item : mItems)
    {
        mPreviouslyPresent[StackIndex(item.Type)] = true;
    }

    mItems.clear();
    mCurrentY = mAnchorY;
}

void HUDLayout::ResetHistory()
{
    mPreviouslyPresent.fill(false);
    mSlideOnEntry.fill(false);
    mStackProgress.fill(0.0f);
    mItems.clear();
    mCurrentY = mAnchorY;
}

void HUDLayout::PrepareStack(
    const std::array<HUDComponent*, 8>& components)
{
    const auto& settings = HUDSettings::Get();

    for (std::size_t i = 0; i < components.size(); ++i)
    {
        const HUDComponent& component = *components[i];

        const bool present =
            settings.IsElementEnabled(component.GetType()) &&
            component.IsVisible();

        if (!present)
        {
            mSlideOnEntry[i] = false;
            mStackProgress[i] = 0.0f;
            continue;
        }

        if (!mPreviouslyPresent[i])
        {
            // Only a new top-of-stack item may push an already
            // visible stack downward. New items in the middle
            // or below existing items fade at their fixed
            // positions without entering from above.
            const bool hasVisibleAbove = std::any_of(
                mPreviouslyPresent.begin(),
                mPreviouslyPresent.begin() + i,
                [](bool visible) { return visible; });

            const bool hasVisibleBelow = std::any_of(
                mPreviouslyPresent.begin() + i + 1,
                mPreviouslyPresent.end(),
                [](bool visible) { return visible; });

            mSlideOnEntry[i] = settings.SlideStack &&
                !hasVisibleAbove && hasVisibleBelow;
        }

        // Keep the last occupied space while fading out.
        // Lower items must not slide upward or jump down
        // as the departing element changes its alpha.
        if (!mSlideOnEntry[i])
        {
            mStackProgress[i] = 1.0f;
        }
        else if (!component.IsFadingOut())
        {
            mStackProgress[i] =
                std::max(mStackProgress[i],
                    component.GetLayoutProgress());
        }
    }
}

float HUDLayout::GetStackProgress(HUDComponentType type) const
{
    return mStackProgress[StackIndex(type)];
}

void HUDLayout::SetAnchor(float x, float y)
{
    mAnchorX = x;
    mAnchorY = y;
}

void HUDLayout::Add(
    HUDComponent& component,
    float height,
    float offsetY,
    float spacingAfter,
    float offsetX)
{
    if (!HUDSettings::Get().IsElementEnabled(
        component.GetType()) ||
        !component.IsVisible())
    {
        return;
    }

    HUDLayoutItem item;

    item.Component = &component;

    item.Position =
    {
        mAnchorX + offsetX,
        mCurrentY + offsetY
    };

    item.Type = component.GetType();
    item.Height = height;

    mItems.push_back(item);

    mCurrentY +=
        (height + spacingAfter) *
        GetStackProgress(component.GetType());
}

const std::vector<HUDLayoutItem>& HUDLayout::GetItems() const
{
    return mItems;
}
