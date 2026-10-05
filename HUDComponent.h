#pragma once

#include "HUDLayout.h"
#include "HUDPatch.h"
#include "HUDPatchGroup.h"
#include "VisibilityTimer.h"

class HUDComponent
{
public:

    HUDComponent();
    HUDComponent(HUDComponentType type, std::initializer_list<HUDPatch::Call>);
    HUDComponentType GetType() const;

    bool IsVisible() const;

    void ShowFor(uint32_t);

    void ForceVisible();

    void Hide();

    void Reset();

    void Show();

    void Update();

private:

    HUDComponentType mType;
    HUDPatchGroup mPatches;
    VisibilityTimer mTimer;
};