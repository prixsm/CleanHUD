#include <algorithm>

#include "../Core/HUDComponent.h"
#include "HUDLayout.h"
#include "../Core/HUDSettings.h"

namespace
{
    // Ordering is independent of HUDComponentType's enum values.
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

        // Status bars were previously drawn immediately at their
        // final Y while only the entries below them moved. Give each
        // bar a short one-way entrance slide tied to its fade-in;
        // keep its last progress if the fade reverses.
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
            // New entries reserve their final slot. Status bars
            // also animate visually as they enter this slot.
            mHoldEntrancePosition[i] = true;

            // The permanently displayed Clock is the fixed
            // anchor above the dynamic stack. Do not count it
            // as an old stack member blocking new top entries.
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

            // Only an insertion above the entire old stack
            // is allowed to move the previously visible stack.
            mSlideOnEntry[i] = settings.SlideStack &&
                !hasVisibleAbove && hasVisibleBelow;
        }

        // Preserve the occupied space during fade-out.
        // Disappearance never runs a reverse slide.
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

        // A component from an earlier frame is now part of
        // the old stack, so a new top entry may move it.
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
    // This is a fixed layout slot, not a fading component.
    // Both cursors must move equally so entering components
    // and already-visible components share the same origin.
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

    // The stack space itself still advances through mStackProgress.
    // Move an entering status bar visually from one bar height above
    // its settled slot, without changing spacing or sliding it back
    // during fade-out.
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
