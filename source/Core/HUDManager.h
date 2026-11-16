#pragma once

#include <cstdint>

#include <CPlayerPed.h>

#include "HUDComponent.h"
#include "../Layout/HUDLayout.h"

// Identify the HUD groups on either side of a spacing slider.
// Preview only items the game can actually show.
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
    bool ShouldSuppressNativeWeaponSprite() const;
    bool ShouldDrawJetpackIcon() const;
    // Use the weapon slot for the jetpack only when hands or
    // brass knuckles are selected and the phone is not shown.
    bool ShouldReplaceWeaponWithJetpack() const;
    bool ShouldDrawSkateboardIcon() const;
    bool ShouldReplaceWeaponWithSkateboard() const;
    std::uint8_t GetSkateboardIconAlpha() const;
    int GetSkateboardIconModel() const;
    int GetCustomWeaponIconModel() const;
    bool IsUsingVanillaLayout() const;
    bool ShouldDisplayAmmo() const;
    bool ShouldDisplayArmour() const;

    void OnGameLoaded();
    void RestoreVanillaElements();
    void Update();

    // Briefly show affected HUD items when a spacing slider moves.
    void PreviewSpacing(
        HUDSpacingPreviewGroup first,
        HUDSpacingPreviewGroup second);

    // Show the equipped jetpack while adjusting its position,
    // including in Minimal mode.
    void PreviewJetpackPosition();
    // Preview the skateboard icon only while actually riding it.
    void PreviewSkateboardPosition();

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
    // Track weapon switches separately from ammunition use.
    bool mWeaponChangedThisFrame = false;
    // After a real drive-by shot, allow that weapon's look preview
    // for the current vehicle.
    const CVehicle* mConfirmedDriveByVehicle = nullptr;
    eWeaponType mConfirmedDriveByWeapon = WEAPONTYPE_UNARMED;

    // Keep a requested clock preview visible for its normal duration.
    VisibilityTimer mClockSpacingPreview;
    // Show the jetpack when equipped; the phone follows its active task.
    bool mJetpackWasEquipped = false;
    bool mSkateboardWasRiding = false;
    // The phone animation can finish before its task is cleared.
    bool mCellphonePuttingAway = false;

    // HUD components
    HUDComponent mClock;
    HUDComponent mHealth;
    HUDComponent mArmour;
    HUDComponent mBreath;
    HUDComponent mMoney;
    HUDComponent mWeaponIcon;
    HUDComponent mAmmo;
    HUDComponent mWanted;
    // Extra icons fade independently without replacing GTA draw calls.
    HUDComponent mCellphoneIcon;
    HUDComponent mJetpackIcon;
    HUDComponent mSkateboardIcon;

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
    void UpdateSkateboardIcon();
    bool IsRidingConfiguredSkateboard() const;
    // Keep the weapon and ammo visible together when equipment changes.
    void ShowWeaponAndAmmoForEquipment(std::uint32_t duration);
    void UpdateAmmo();
    void UpdateVehicleFiring();
    bool IsVanillaPreviewHeld() const;
    bool IsWeaponAiming() const;
    bool HasEmptyVehicleAmmo() const;
    void UpdateWanted();

    void PreviewSpacingGroup(HUDSpacingPreviewGroup group);

    void BuildPlayerHUDLayout();
    void BuildMinimalHUDLayout();

    void HideAll();
};