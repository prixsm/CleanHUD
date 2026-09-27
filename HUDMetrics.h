#pragma once

class HUDMetrics
{
public:
    static float GetVanillaRightHudX();
    static float ScaleNativeRightX(float nativeX);

    static constexpr int MoneyRightOffset = -6;

    static int GetStatusBarWidth();
    static float GetStatusBarSpacing();
};