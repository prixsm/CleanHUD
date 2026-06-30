#include <common.h>
#include <CPlayerInfo.h>
#include <CPlayerPed.h>
#include <CTimer.h>
#include <CWorld.h>
#include "HUDManager.h"

HUDManager::HUDManager() :
    mClock(),
    mHealth({
    HUDPatch::Health
        }),
    mArmour({
    HUDPatch::Armour
        }),
    mBreath(),
    mMoney({
    HUDPatch::Money
        }),
    mWeaponIcon({
    HUDPatch::Weapon
        }),
    mAmmo()
{
}

HUDManager::ActiveWeaponState HUDManager::GetCurrentWeaponState() const
{
    ActiveWeaponState state;

    if (!mPlayer)
        return state;

    const CWeapon* weapon = mPlayer->GetWeapon();

    if (!weapon)
        return state;

    state.WeaponType = weapon->m_eWeaponType;
    state.AmmoInClip = weapon->m_nAmmoInClip;
    state.AmmoTotal = weapon->m_nAmmoTotal;

    return state;
}

bool HUDManager::IsPlayerHUDInitialized() const
{
    return mSessionInitialized;
}

void HUDManager::Initialize()
{
    if (m_initialized)
        return;

    m_initialized = true;
}

void HUDManager::OnGameLoaded()
{
    mPlayer = FindPlayerPed();

    if (!mPlayer)
        return;

    mLastPlayer = mPlayer;

    mSessionInitialized = false;
    mShouldIgnoreFirstHealthChange = false;
    mNeedsReset = true;
}

void HUDManager::Update()
{
    mPlayer = FindPlayerPed();

    if (!mPlayer)
        return;

    Initialize();

    if (mNeedsReset)
    {
        ResetSessionState();
        mNeedsReset = false;
    }

    if (mPlayer != mLastPlayer)
    {
        mLastPlayer = mPlayer;
        mSessionInitialized = false;
        mShouldIgnoreFirstHealthChange = true;
        mNeedsReset = true;
        return;
    }

    bool isDead = (mPlayer->m_fHealth <= 0.0f);

    if (mWasDead && !isDead)
    {
        mSessionInitialized = false;
        mShouldIgnoreFirstHealthChange = true;
        mNeedsReset = true;
        mWasDead = false;
        return;
    }

    mWasDead = isDead;

    // Update HUD components
    UpdateHealth();
    UpdateArmour();
    UpdateMoney();
    UpdateWeaponIcon();
    //UpdateAmmo();
    //UpdateBreath();
    //UpdateClock();
}

void HUDManager::ResetSessionState()
{
    SyncTrackedValues();

    mPreviousHealth = mLastHealth;
    mHealthStableFrames = 0;
    mWaitingForStableHealth = true;

    mPreviousArmour = mLastArmour;
    mArmourStableFrames = 0;
    mWaitingForStableArmour = true;

    mPreviousMoney = mLastMoney;
    mMoneyStableFrames = 0;
    mWaitingForStableMoney = true;

    mWeaponStableFrames = 0;
    mWaitingForStableWeapon = true;

    HideAll();
}

void HUDManager::SyncTrackedValues()
{
    mLastHealth = mPlayer->m_fHealth;
    mLastArmour = mPlayer->m_fArmour;
    mLastMoney = CWorld::Players[CWorld::PlayerInFocus].m_nMoney;

    mWasDead = (mPlayer->m_fHealth <= 0.0f);
    mLastWeaponState = GetCurrentWeaponState();
    // mLastAmmo
    // mLastBreath
}

bool HUDManager::UpdateHealthInitialization(float currentHealth)
{
    if (!mWaitingForStableHealth)
        return true;

    if (currentHealth == mPreviousHealth)
    {
        mHealthStableFrames++;

        if (mHealthStableFrames >= 2)
        {
            mWaitingForStableHealth = false;

            if (mShouldIgnoreFirstHealthChange)
                mIgnoreFirstHealthChange = true;

            mShouldIgnoreFirstHealthChange = false;

            mSessionInitialized = true;
        }
    }
    else
    {
        mHealthStableFrames = 0;
    }

    mLastHealth = currentHealth;
    mPreviousHealth = currentHealth;

    mHealth.Update();

    return false;
}

bool HUDManager::UpdateArmourInitialization(float currentArmour)
{
    if (!mWaitingForStableArmour)
        return true;

    if (currentArmour == mPreviousArmour)
    {
        mArmourStableFrames++;

        if (mArmourStableFrames >= 2)
        {
            mWaitingForStableArmour = false;
        }
    }
    else
    {
        mArmourStableFrames = 0;
    }

    mLastArmour = currentArmour;
    mPreviousArmour = currentArmour;

    mArmour.Update();

    return false;
}

bool HUDManager::UpdateMoneyInitialization(int currentMoney)
{
    if (!mWaitingForStableMoney)
        return true;

    if (currentMoney == mPreviousMoney)
    {
        mMoneyStableFrames++;

        if (mMoneyStableFrames >= 2)
            mWaitingForStableMoney = false;
    }
    else
    {
        mMoneyStableFrames = 0;
    }

    mLastMoney = currentMoney;
    mPreviousMoney = currentMoney;

    mMoney.Update();

    return false;
}

bool HUDManager::UpdateWeaponInitialization(const ActiveWeaponState& current)
{
    if (!mWaitingForStableWeapon)
        return true;

    if (current.WeaponType == mLastWeaponState.WeaponType)
    {
        mWeaponStableFrames++;

        if (mWeaponStableFrames >= 2)
            mWaitingForStableWeapon = false;
    }
    else
    {
        mWeaponStableFrames = 0;

        mLastWeaponState = current;
    }

    mWeaponIcon.Update();

    return false;
}

bool HUDManager::HasHealthChanged(float currentHealth)
{
    if (fabs(currentHealth - mLastHealth) <= 0.01f)
        return false;

    mLastHealth = currentHealth;
    return true;
}

bool HUDManager::HasArmourChanged(float currentArmour)
{
    if (fabs(currentArmour - mLastArmour) <= 0.01f)
        return false;

    mLastArmour = currentArmour;

    return true;
}

bool HUDManager::HasMoneyChanged(int currentMoney)
{
    if (currentMoney == mLastMoney)
        return false;

    mLastMoney = currentMoney;

    return true;
}

bool HUDManager::HasWeaponChanged(const ActiveWeaponState& current)
{
    if (current.WeaponType == mLastWeaponState.WeaponType)
        return false;

    mLastWeaponState = current;

    return true;
}

void HUDManager::UpdateHealth()
{
    float currentHealth = mPlayer->m_fHealth;

    if (!UpdateHealthInitialization(currentHealth))
        return;

    if (HasHealthChanged(currentHealth))
    {
        if (mIgnoreFirstHealthChange)
        {
            mIgnoreFirstHealthChange = false;

            mHealth.Update();
            return;
        }

        mHealth.ShowFor(mVisibleTime);
    }

    mHealth.Update();
}

void HUDManager::UpdateArmour()
{
    float currentArmour = mPlayer->m_fArmour;

    if (!UpdateArmourInitialization(currentArmour))
        return;

    if (HasArmourChanged(currentArmour))
        mArmour.ShowFor(mVisibleTime);

    mArmour.Update();
}

void HUDManager::UpdateMoney()
{
    const auto& playerInfo = CWorld::Players[CWorld::PlayerInFocus];

    int currentMoney = playerInfo.m_nMoney;
    int displayedMoney = playerInfo.m_nDisplayMoney;

    if (!UpdateMoneyInitialization(currentMoney))
        return;

    if (HasMoneyChanged(currentMoney))
    {
        mMoney.ShowFor(mVisibleTime);
    }

    if (displayedMoney != currentMoney)
    {
        mMoney.ShowFor(mVisibleTime);
    }

    mMoney.Update();
}

void HUDManager::UpdateWeaponIcon()
{
    ActiveWeaponState current = GetCurrentWeaponState();

    if (!UpdateWeaponInitialization(current))
        return;

    if (HasWeaponChanged(current))
        mWeaponIcon.ShowFor(mVisibleTime);

    mWeaponIcon.Update();
}

void HUDManager::HideAll()
{
    mClock.Reset();
    mHealth.Reset();
    mArmour.Reset();
    mBreath.Reset();
    mMoney.Reset();
    mWeaponIcon.Reset();
    mAmmo.Reset();
}

HUDManager& HUDManager::Get()
{
    static HUDManager instance;
    return instance;
}