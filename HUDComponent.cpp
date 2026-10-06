#include <algorithm>
#include <cmath>

#include <CTimer.h>

#include "HUDComponent.h"
#include "HUDLayout.h"

HUDComponent::HUDComponent(
    HUDComponentType type,
    std::initializer_list<HUDPatch::Call> patches,
    bool animated)
    :
    mType(type),
    mPatches(patches),
    mAnimated(animated)
{
}

HUDComponent::HUDComponent()
    :
    mType(HUDComponentType::Health),
    mPatches({}),
    mAnimated(true)
{
}

HUDComponentType HUDComponent::GetType() const
{
    return mType;
}

bool HUDComponent::IsFadingOut() const
{
    return
        mAnimated &&
        !mTargetVisible &&
        mVisibility > 0.0f;
}

bool HUDComponent::IsVisible() const
{
    return mPatches.IsVisible();
}

float HUDComponent::GetVisibility() const
{
    return mVisibility;
}

float HUDComponent::GetLayoutProgress() const
{
    const float t =
        std::clamp(
            mVisibility,
            0.0f,
            1.0f);

    /*
     * Smoothstep.
     * We'll use this for stack movement later.
     */
    return
        t * t *
        (3.0f - 2.0f * t);
}

uint8_t HUDComponent::GetAlpha() const
{
    return static_cast<uint8_t>(
        std::lround(
            std::clamp(
                mVisibility,
                0.0f,
                1.0f) *
            255.0f));
}

void HUDComponent::ShowFor(
    uint32_t milliseconds)
{
    mTimer.ShowFor(milliseconds);
}

void HUDComponent::Show()
{
    mTimer.Show();
}

void HUDComponent::Hide()
{
    mTimer.Hide();

    /*
     * Non-animated components retain the old
     * immediate-hide behavior.
     *
     * This is what Wanted will use.
     */
    if (!mAnimated)
    {
        mTargetVisible = false;
        mVisibility = 0.0f;

        mPatches.Hide();
    }
}

void HUDComponent::Reset()
{
    mTimer.Reset();

    mTargetVisible = false;
    mVisibility = 0.0f;

    mLastAnimationTime = 0;
    mAnimationInitialized = false;

    mPatches.Hide();
}

void HUDComponent::Update(
    bool forceVisible)
{
    const bool targetVisible =
        mTimer.Visible() ||
        forceVisible;

    mTargetVisible =
        targetVisible;

    /*
     * Wanted and any future non-animated component
     * use the original binary visibility behavior.
     */
    if (!mAnimated)
    {
        mVisibility =
            targetVisible
            ? 1.0f
            : 0.0f;

        if (targetVisible)
            mPatches.Show();
        else
            mPatches.Hide();

        return;
    }

    const uint32_t now =
        CTimer::m_snTimeInMilliseconds;

    if (!mAnimationInitialized)
    {
        mLastAnimationTime = now;
        mAnimationInitialized = true;
    }

    const uint32_t deltaMs =
        now - mLastAnimationTime;

    mLastAnimationTime = now;

    const float duration =
        targetVisible
        ? FadeInTime
        : FadeOutTime;

    const float step =
        duration > 0.0f
        ? static_cast<float>(deltaMs) /
        duration
        : 1.0f;

    if (targetVisible)
    {
        mVisibility =
            std::min(
                1.0f,
                mVisibility + step);
    }
    else
    {
        mVisibility =
            std::max(
                0.0f,
                mVisibility - step);
    }

    /*
     * During fade-out the GTA draw call must remain
     * enabled until alpha reaches zero.
     */
    if (targetVisible ||
        mVisibility > 0.0f)
    {
        mPatches.Show();
    }
    else
    {
        mPatches.Hide();
    }
}