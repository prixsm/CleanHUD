#include "HUDComponent.h"
#include "HUDLayout.h"

void HUDLayout::Reset()
{
    mItems.clear();

    mCurrentY = mAnchorY;
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
    if (!component.IsVisible())
        return;

    const float layoutProgress =
        component.GetLayoutProgress();

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
        layoutProgress;
}

const std::vector<HUDLayoutItem>& HUDLayout::GetItems() const
{
    return mItems;
}