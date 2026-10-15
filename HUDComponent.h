#pragma once

#include <cstdint>

#include "HUDLayout.h"
#include "HUDPatch.h"
#include "HUDPatchGroup.h"
#include "VisibilityTimer.h"

class HUDComponent
{
public:
    HUDComponent();

    HUDComponent(
        HUDComponentType type,
        std::initializer_list<HUDPatch::Call> patches,
        bool animated = true);

    HUDComponentType GetType() const;

    bool IsFadingOut() const;
    bool IsVisible() const;

    float GetVisibility() const;
    float GetLayoutProgress() const;
    uint8_t GetAlpha() const;

    void ShowFor(uint32_t milliseconds);
    void Show();
    void Hide();
    void Reset();

    void Update(
        bool forceVisible = false);

private:
    HUDComponentType mType;

    HUDPatchGroup mPatches;
    VisibilityTimer mTimer;

    bool mAnimated = true;
    bool mTargetVisible = false;

    float mVisibility = 0.0f;

    uint32_t mLastAnimationTime = 0;
    bool mAnimationInitialized = false;

};