#pragma once

#include <cstdint>
#include <CPlayerPed.h>
#include "HUDElement.h"
#include "HUDComponent.h"

class HUDManager
{
public:
    HUDManager();

    bool IsPlayerHUDInitialized() const;
    void Initialize();
    void OnGameLoaded();
    void Update();

    static HUDManager& Get();

private:

    bool m_initialized = false;
    
    struct ActiveWeaponState
    {
        eWeaponType WeaponType = WEAPONTYPE_UNARMED;

        unsigned int AmmoInClip = 0;

        unsigned int AmmoTotal = 0;

        bool operator==(const ActiveWeaponState& rhs) const
        {
            return WeaponType == rhs.WeaponType &&
                AmmoInClip == rhs.AmmoInClip &&
                AmmoTotal == rhs.AmmoTotal;
        }

        bool operator!=(const ActiveWeaponState& rhs) const
        {
            return !(*this == rhs);
        }
    };

    ActiveWeaponState GetCurrentWeaponState() const;

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
    HUDComponent mHealth;
    HUDComponent mArmour;
    HUDComponent mMoney;

    HUDComponent mWeaponIcon;
    HUDComponent mAmmo;

    HUDComponent mClock;
    HUDComponent mBreath;

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

    // Weapon tracker
    ActiveWeaponState mLastWeaponState;
    bool mWaitingForStableWeapon = false;
    int mWeaponStableFrames = 0;

    // Ammo tracker
    bool mWaitingForStableAmmo = false;
    int mAmmoStableFrames = 0;

    bool UpdateHealthInitialization(float currentHealth);
    bool UpdateArmourInitialization(float currentArmour);
    bool UpdateMoneyInitialization(int currentMoney);
    bool UpdateWeaponInitialization(const ActiveWeaponState& current);

    bool HasHealthChanged(float currentHealth);
    bool HasArmourChanged(float currentArmour);
    bool HasMoneyChanged(int currentMoney);
    bool HasWeaponChanged(const ActiveWeaponState& current);

    uint32_t mCurrentTime = 0;
    uint32_t mVisibleTime = 4000;

    void ResetSessionState();
    void SyncTrackedValues();

    void UpdateHealth();
    void UpdateArmour();
    void UpdateBreath();
    void UpdateMoney();
    void UpdateWeaponIcon();
    void UpdateAmmo();

    void HideAll();
};