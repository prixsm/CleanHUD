#pragma once

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

    void SetAnchor(float x, float y);

    void Add(HUDComponent& component, float height);

    const std::vector<HUDLayoutItem>& GetItems() const;

private:

    float mAnchorX = 0.0f;
    float mAnchorY = 0.0f;

    float mCurrentY = 0.0f;

    std::vector<HUDLayoutItem> mItems;
};