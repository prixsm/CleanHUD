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

void HUDLayout::Add(HUDComponent& component, float height)
{
    if (!component.IsVisible())
        return;

    HUDLayoutItem item;

    item.Component = &component;

    item.Position =
    {
        mAnchorX,
        mCurrentY
    };

    item.Type = component.GetType();
    item.Height = height;

    mItems.push_back(item);

    mCurrentY +=
        height +
        HUDLayoutConstants::StackSpacing;
}

const std::vector<HUDLayoutItem>& HUDLayout::GetItems() const
{
    return mItems;
}