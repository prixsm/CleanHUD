#pragma once

#include <cstdint>

#include <CPlayerPed.h>

#include "HUDComponent.h"
#include "HUDLayout.h"

class HUDManager
{
public:
    HUDManager();

    bool HasWantedLevel() const;
    bool IsPlayerHUDInitialized() const;
    bool IsUsingVanillaLayout() const;
    bool ShouldDisplayAmmo() const;

    void Initialize();
    void OnGameLoaded();
    void Update();

    static HUDManager& Get();

private:

    bool m_initialized = false;

    struct ActiveWeaponState
    {
        eWeaponType WeaponType = WEAPONTYPE_UNARMED;

        bool operator==(const ActiveWeaponState& rhs) const
        {
            return WeaponType == rhs.WeaponType;
        }

        bool operator!=(const ActiveWeaponState& rhs) const
        {
            return !(*this == rhs);
        }
    };

    struct AmmoState
    {
        unsigned int AmmoInClip = 0;
        unsigned int AmmoTotal = 0;

        bool operator==(const AmmoState& rhs) const
        {
            return AmmoInClip == rhs.AmmoInClip &&
                AmmoTotal == rhs.AmmoTotal;
        }

        bool operator!=(const AmmoState& rhs) const
        {
            return !(*this == rhs);
        }
    };

    enum class BreathState
    {
        Idle,
        Underwater,
        WaitingForRecovery
    };

    ActiveWeaponState GetCurrentWeaponState() const;
    AmmoState GetCurrentAmmoState() const;

    // Player state
    CPlayerPed* mPlayer = nullptr;
    CPlayerPed* mLastPlayer = nullptr;

    // Session state
    bool mSessionInitialized = false;
    bool mShouldIgnoreFirstHealthChange = false;
    bool mIgnoreFirstHealthChange = false;
    bool mNeedsReset = true;
    bool mWasDead = false;

    // HUD components
    HUDComponent mClock;
    HUDComponent mHealth;
    HUDComponent mArmour;
    HUDComponent mBreath;
    HUDComponent mMoney;
    HUDComponent mWeaponIcon;
    HUDComponent mAmmo;
    HUDComponent mWanted;
    
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

    //Breath tracker
    BreathState mBreathState = BreathState::Idle;
    float mHighestBreathValue = 0.0f;
    
    bool mWatchingBreathRecovery = false;
    int mBreathStableFrames = 0;

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

    bool IsBreathRecoveryComplete(float currentBreath);

    uint32_t mVisibleTime = 4000;

    void ResetSessionState();
    void SyncTrackedValues();

    void UpdateHealth();
    void UpdateArmour();
    void UpdateBreath();
    void UpdateMoney();
    void UpdateWeaponIcon();
    void UpdateAmmo();
    void UpdateWanted();

    void BuildPlayerHUDLayout();

    void HideAll();
};