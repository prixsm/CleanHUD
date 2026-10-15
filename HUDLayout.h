#pragma once

#include <array>
#include <cstddef>
#include <vector>

class HUDComponent;

struct HUDPosition
{
    float X = 0.0f;
    float Y = 0.0f;
};

enum class HUDComponentType
{
    Clock,
    Health,
    Armour,
    Breath,
    Money,
    Weapon,
    Ammo,
    Wanted
};

struct HUDLayoutItem
{
    HUDComponentType Type;

    HUDComponent* Component;

    HUDPosition Position;

    float Height;
};

namespace HUDLayoutConstants
{
    inline constexpr float StackSpacing = 8.0f;
}

class HUDLayout
{
public:

    HUDLayout() = default;

    void Reset();
    void ResetHistory();

    // Components in actual top-to-bottom layout order.
    void PrepareStack(
        const std::array<HUDComponent*, 8>& components);

    float GetStackProgress(HUDComponentType type) const;
    bool UsesFinalEntryPosition(HUDComponentType type) const;

    void SetAnchor(float x, float y);

    void Add(
        HUDComponent& component,
        float height,
        float offsetY = 0.0f,
        float spacingAfter = HUDLayoutConstants::StackSpacing,
        float offsetX = 0.0f);

    const std::vector<HUDLayoutItem>& GetItems() const;

private:

    float mAnchorX = 0.0f;
    float mAnchorY = 0.0f;

    float mCurrentY = 0.0f;
    float mFinalY = 0.0f;

    static constexpr std::size_t ElementCount = 8;

    // Previous frame's visible entries determine whether a
    // newly inserted item is above something already shown.
    std::array<bool, ElementCount> mPreviouslyPresent = {};
    std::array<bool, ElementCount> mSlideOnEntry = {};
    std::array<bool, ElementCount> mHoldEntrancePosition = {};
    std::array<float, ElementCount> mStackProgress = {};

    std::vector<HUDLayoutItem> mItems;
};