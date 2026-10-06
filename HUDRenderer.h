#pragma once

#include <cstdint>

class HUDLayout;

class HUDRenderer
{
public:
    static HUDRenderer& Get();

    void Reset();
    void Build(const HUDLayout& layout);

    void SetClockPosition(int x, int y, uint8_t alpha);
    void SetWeaponPosition(int x, int y, uint8_t alpha);
    void SetAmmoPosition(int x, int y, uint8_t alpha);
    void SetHealthPosition(int x, int y, uint8_t alpha);
    void SetArmourPosition(int x, int y, uint8_t alpha);
    void SetBreathPosition(int x, int y, uint8_t alpha);
    void SetMoneyPosition(int x, int y, uint8_t alpha);
    void SetWantedPosition(int x, int y, uint8_t alpha);

    struct HUDAnchor
    {
        float X = 0.0f;
        float Y = 0.0f;
    };

    HUDAnchor GetAnchor() const;
    void SetAnchor(float x, float y);

    struct HUDRenderPosition
    {
        int X = 0;
        int Y = 0;

        uint8_t Alpha = 255;

        bool Valid = false;
    };

    bool ShouldOverridePosition() const;
    HUDRenderPosition GetClockPosition() const;
    HUDRenderPosition GetWeaponPosition() const;
    HUDRenderPosition GetAmmoPosition() const;
    HUDRenderPosition GetHealthPosition() const;
    HUDRenderPosition GetArmourPosition() const;
    HUDRenderPosition GetBreathPosition() const;
    HUDRenderPosition GetMoneyPosition() const;
    HUDRenderPosition GetWantedPosition() const;

private:
    HUDRenderer() = default;
    HUDAnchor mAnchor;

    HUDRenderPosition mClock;
    HUDRenderPosition mWeapon;
    HUDRenderPosition mAmmo;
    HUDRenderPosition mHealth;
    HUDRenderPosition mArmour;
    HUDRenderPosition mBreath;
    HUDRenderPosition mMoney;
    HUDRenderPosition mWanted;
};