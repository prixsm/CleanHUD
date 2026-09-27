#include <extensions/Screen.h>
#include "HUDMetrics.h"

namespace
{
    constexpr int StatusBarWidth = 90;

    constexpr float StatusBarSpacing = 18.0f;
}

float HUDMetrics::GetVanillaRightHudX()
{
    const float screenWidth =
        plugin::screen::GetScreenWidth();

    const float screenHeight =
        plugin::screen::GetScreenHeight();

    return screenWidth -
        (screenHeight * (141.0f / 560.0f));
}

float HUDMetrics::ScaleNativeRightX(float nativeX)
{
    const float screenWidth =
        plugin::screen::GetScreenWidth();

    const float screenHeight =
        plugin::screen::GetScreenHeight();

    const float aspectRatio =
        screenWidth / screenHeight;

    constexpr float DefaultWidth = 640.0f;
    constexpr float DefaultAspectRatio = 4.0f / 3.0f;

    const float distanceFromRight =
        DefaultWidth - nativeX;

    const float scaledDistance =
        screenWidth *
        distanceFromRight *
        (1.0f / DefaultWidth) *
        (DefaultAspectRatio / aspectRatio);

    return screenWidth - scaledDistance;
}

int HUDMetrics::GetStatusBarWidth()
{
    return StatusBarWidth;
}

float HUDMetrics::GetStatusBarSpacing()
{
    return StatusBarSpacing;
}