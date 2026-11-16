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

    // Items ordered from top to bottom.
    void PrepareStack(
        const std::array<HUDComponent*, 8>& components);

    float GetStackProgress(HUDComponentType type) const;
    bool UsesFinalEntryPosition(HUDComponentType type) const;

    void SetAnchor(float x, float y);

    // Leave space for an item that GTA still draws itself.
    void ReserveSpace(float height, float spacingAfter);

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

    // Check which items were already visible before sliding the stack.
    std::array<bool, ElementCount> mPreviouslyPresent = {};
    std::array<bool, ElementCount> mSlideOnEntry = {};
    std::array<bool, ElementCount> mHoldEntrancePosition = {};
    std::array<float, ElementCount> mStackProgress = {};
    // Status bars slide in, and the stack moves to make room.
    std::array<float, ElementCount> mStatusEntryProgress = {};

    std::vector<HUDLayoutItem> mItems;
};