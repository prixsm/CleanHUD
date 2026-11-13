#pragma once

#include <cstdint>

#include <CPlayerPed.h>

#include "HUDComponent.h"
#include "../Layout/HUDLayout.h"

// A preview target describes the two groups bordering one spacing value.
// The editor names specific targets; HUDManager decides what GTA can
// actually render without altering weapon, breath, armour or wanted state.
enum class HUDSpacingPreviewGroup
{
    Clock,
    Money,
    Wanted,
    StatusBars,
    Weapon,
    Ammo,
    Armour,
    Breath,
    Health
};

class HUDManager
{
public:
    HUDManager();

    bool HasWantedLevel() const;
    bool IsInitialHealthTrackingBlocked() const;
    bool IsPlayerHUDInitialized() const;
    bool ShouldDrawCellphoneIcon() const;
    std::uint8_t GetCellphoneIconAlpha() const;
    std::uint8_t GetJetpackIconAlpha() const;
    bool ShouldSuppressNativeFist() const;
    bool ShouldDrawJetpackIcon() const;
    bool ShouldDrawJetpackBesideWeapon() const;
    bool ShouldDrawJetpackAsWeaponIcon() const;
    int GetCustomWeaponIconModel() const;
    bool IsUsingVanillaLayout() const;
    bool ShouldDisplayAmmo() const;
    bool ShouldDisplayArmour() const;

    void OnGameLoaded();
    void RestoreVanillaElements();
    void Update();

    // Called on the main thread by SleekConfig when a spacing value
    // changes. Refreshes the normal per-element visibility timers.
    void PreviewSpacing(
        HUDSpacingPreviewGroup first,
        HUDSpacingPreviewGroup second);

    static HUDManager& Get();

private:

    struct ActiveWeaponState
    {
        eWeaponType WeaponType = WEAPONTYPE_UNARMED;
    };

    struct AmmoState
    {
        unsigned int AmmoInClip = 0;
        unsigned int AmmoTotal = 0;

        bool operator==(
            const AmmoState& rhs) const
        {
            return
                AmmoInClip == rhs.AmmoInClip &&
                AmmoTotal == rhs.AmmoTotal;
        }
    };

    ActiveWeaponState GetCurrentWeaponState() const;
    AmmoState GetCurrentAmmoState() const;

    // Player state
    CPlayerPed* mPlayer = nullptr;
    CPlayerPed* mLastPlayer = nullptr;

    // Session state
    bool mSessionInitialized = false;
    bool mNeedsReset = true;
    bool mVanillaLayoutLatched = false;
    bool mWasDead = false;
    // One-frame input grace covers game-process/weapon-update ordering.
    bool mValidVehicleFireInputLastFrame = false;
    bool mWeaponChangedThisFrame = false;

    // Only used to stop AlwaysShowClock=0 from hiding a preview
    // before its ordinary mClock.ShowFor() timer expires.
    VisibilityTimer mClockSpacingPreview;
    // Jetpack is an on-equip event, unlike the continuously held phone task.
    bool mJetpackWasEquipped = false;
    bool mJetpackDropped = false;
    std::uint32_t mJetpackEquippedAt = 0;

    // HUD components
    HUDComponent mClock;
    HUDComponent mHealth;
    HUDComponent mArmour;
    HUDComponent mBreath;
    HUDComponent mMoney;
    HUDComponent mWeaponIcon;
    HUDComponent mAmmo;
    HUDComponent mWanted;
    // Independent alphas; no native draw-call patches for supplemental icons.
    HUDComponent mCellphoneIcon;
    HUDComponent mJetpackIcon;

    HUDLayout mLayout;

    // Health tracker
    float mLastHealth = 0.0f;
    float mPreviousHealth = 0.0f;

    bool mWaitingForStableHealth = false;
    int mHealthStableFrames = 0;

    // Armour tracker
    float mLastArmour = 0.0f;
    float mPreviousArmour = 0.0f;

    bool mWaitingForStableArmour = false;
    int mArmourStableFrames = 0;

    // Money tracker
    int mLastMoney = 0;
    int mPreviousMoney = 0;

    bool mWaitingForStableMoney = false;
    int mMoneyStableFrames = 0;

    // Weapon Icon tracker
    ActiveWeaponState mLastWeaponState;
    bool mWaitingForStableWeapon = false;
    int mWeaponStableFrames = 0;

    // Ammo tracker
    AmmoState mLastAmmoState;
    bool mWaitingForStableAmmo = false;
    int mAmmoStableFrames = 0;

    bool UpdateHealthInitialization(float currentHealth);
    bool UpdateArmourInitialization(float currentArmour);
    bool UpdateMoneyInitialization(int currentMoney);
    bool UpdateWeaponInitialization(const ActiveWeaponState& current);
    bool UpdateAmmoInitialization(const AmmoState& current);

    bool HasHealthChanged(float currentHealth);
    bool HasArmourChanged(float currentArmour);
    bool HasMoneyChanged(int currentMoney);
    bool HasWeaponChanged(const ActiveWeaponState& current);
    bool HasAmmoChanged(const AmmoState& current);

    uint32_t mVisibleTime = 4000;

    void ResetSessionState();
    void RestoreDisabledElements();
    void SyncTrackedValues();

    void UpdateHealth();
    void UpdateArmour();
    void UpdateBreath();
    void UpdateMoney();
    void UpdateWeaponIcon();
    void UpdateSpecialIcons();
    void UpdateAmmo();
    void UpdateVehicleFiring();
    bool IsValidVehicleFireInput() const;
    bool IsWeaponAiming() const;
    bool HasEmptyVehicleAmmo() const;
    void UpdateWanted();

    void PreviewSpacingGroup(HUDSpacingPreviewGroup group);

    void BuildPlayerHUDLayout();
    void BuildMinimalHUDLayout();

    void HideAll();
};