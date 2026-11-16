#include <algorithm>

#include "../Core/HUDComponent.h"
#include "HUDLayout.h"
#include "../Core/HUDSettings.h"

namespace
{
    // Follow the visible top-to-bottom order, not the enum order.
    constexpr std::array<HUDComponentType, 8> StackOrder = {
        HUDComponentType::Clock,
        HUDComponentType::Money,
        HUDComponentType::Wanted,
        HUDComponentType::Armour,
        HUDComponentType::Breath,
        HUDComponentType::Health,
        HUDComponentType::Weapon,
        HUDComponentType::Ammo
    };

    std::size_t StackIndex(HUDComponentType type)
    {
        const auto found =
            std::find(StackOrder.begin(), StackOrder.end(), type);

        return static_cast<std::size_t>(
            found - StackOrder.begin());
    }

    bool IsStatusBar(HUDComponentType type)
    {
        return type == HUDComponentType::Health ||
            type == HUDComponentType::Armour ||
            type == HUDComponentType::Breath;
    }
}

void HUDLayout::Reset()
{
    mPreviouslyPresent.fill(false);

    for (const auto& item : mItems)
    {
        mPreviouslyPresent[StackIndex(item.Type)] = true;
    }

    mItems.clear();
    mCurrentY = mAnchorY;
    mFinalY = mAnchorY;
}

void HUDLayout::ResetHistory()
{
    mPreviouslyPresent.fill(false);
    mSlideOnEntry.fill(false);
    mHoldEntrancePosition.fill(false);
    mStackProgress.fill(0.0f);
    mStatusEntryProgress.fill(0.0f);
    mItems.clear();
    mCurrentY = mAnchorY;
    mFinalY = mAnchorY;
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
            mHoldEntrancePosition[i] = false;
            mStackProgress[i] = 0.0f;
            mStatusEntryProgress[i] = 0.0f;
            continue;
        }

        // Slide status bars into place as they appear, without
        // moving them backward during fade-out.
        if (settings.SlideStack && IsStatusBar(component.GetType()))
        {
            if (!mPreviouslyPresent[i])
                mStatusEntryProgress[i] = 0.0f;

            if (!component.IsFadingOut())
                mStatusEntryProgress[i] = std::max(
                    mStatusEntryProgress[i],
                    component.GetLayoutProgress());
        }
        else
        {
            mStatusEntryProgress[i] = 1.0f;
        }

        if (!mPreviouslyPresent[i])
        {
            // Make room for new items; status bars slide into that room.
            mHoldEntrancePosition[i] = true;

            // The always-visible clock stays fixed above the stack.
            // It should not prevent other items from sliding in.
            const auto dynamicStackStart =
                i > 0 && settings.AlwaysShowClock && settings.Clock
                ? mPreviouslyPresent.begin() + 1
                : mPreviouslyPresent.begin();

            const bool hasVisibleAbove = std::any_of(
                dynamicStackStart,
                mPreviouslyPresent.begin() + i,
                [](bool visible) { return visible; });

            const bool hasVisibleBelow = std::any_of(
                mPreviouslyPresent.begin() + i + 1,
                mPreviouslyPresent.end(),
                [](bool visible) { return visible; });

            // Move existing items only when something appears above them.
            mSlideOnEntry[i] = settings.SlideStack &&
                !hasVisibleAbove && hasVisibleBelow;
        }

        // Keep the space during fade-out; do not slide items back.
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

    for (std::size_t i = 0; i < components.size(); ++i)
    {
        if (!mHoldEntrancePosition[i])
            continue;

        const HUDComponent& component = *components[i];
        bool newlyInsertedAbove = false;
        bool upperSlotStillGrowing = false;

        for (std::size_t j = 0; j < i; ++j)
        {
            const bool upperPresent =
                settings.IsElementEnabled(components[j]->GetType()) &&
                components[j]->IsVisible();

            if (!upperPresent)
                continue;

            if (!mPreviouslyPresent[j] && mSlideOnEntry[j])
                newlyInsertedAbove = true;

            if (mStackProgress[j] < 1.0f)
                upperSlotStillGrowing = true;
        }

        // Items already visible may move down for a new top item.
        if ((mPreviouslyPresent[i] && newlyInsertedAbove) ||
            (component.GetVisibility() >= 1.0f &&
                !upperSlotStillGrowing))
        {
            mHoldEntrancePosition[i] = false;
        }
    }
}

float HUDLayout::GetStackProgress(HUDComponentType type) const
{
    return mStackProgress[StackIndex(type)];
}

bool HUDLayout::UsesFinalEntryPosition(HUDComponentType type) const
{
    return mHoldEntrancePosition[StackIndex(type)];
}

void HUDLayout::SetAnchor(float x, float y)
{
    mAnchorX = x;
    mAnchorY = y;
}

void HUDLayout::ReserveSpace(float height, float spacingAfter)
{
    // Reserve a fixed space so incoming and visible items align.
    const float advance = height + spacingAfter;

    mCurrentY += advance;
    mFinalY += advance;
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
    item.Type = component.GetType();

    // Slide a new status bar down into place without changing
    // the gaps or reversing its movement during fade-out.
    const float entryShift =
        HUDSettings::Get().SlideStack && IsStatusBar(item.Type)
        ? height * (1.0f -
            mStatusEntryProgress[StackIndex(item.Type)])
        : 0.0f;

    item.Position =
    {
        mAnchorX + offsetX,
        (UsesFinalEntryPosition(item.Type)
            ? mFinalY : mCurrentY) + offsetY - entryShift
    };

    item.Height = height;
    mItems.push_back(item);

    mCurrentY +=
        (height + spacingAfter) *
        GetStackProgress(component.GetType());

    mFinalY += height + spacingAfter;
}

const std::vector<HUDLayoutItem>& HUDLayout::GetItems() const
{
    return mItems;
}
