#pragma once

class HUDMetrics
{
public:
    static float GetVanillaRightHudX();
    static float ScaleNativeRightX(float nativeX);

    static constexpr int MoneyRightOffset = -6;

    static constexpr float ArmourBreathNativeXOffset =
        0.5f;

    static int GetStatusBarWidth();
    static float GetStatusBarSpacing();
};