#include <algorithm>

#include <common.h>
#include <CCamera.h>
#include <CCutsceneMgr.h>
#include <CModelInfo.h>
#include <CDarkel.h>
#include <CPad.h>
#include <CPedIntelligence.h>
#include <CPlayerData.h>
#include <CPlayerInfo.h>
#include <CPlayerPed.h>
#include <CStats.h>
#include <CVehicle.h>
#include <CTaskSimpleSwim.h>
#include <CTaskSimpleJetpack.h>
#include <CWanted.h>
#include <CWeaponInfo.h>
#include <CWorld.h>
#include <eModelID.h>
#include <eTaskType.h>

#include "HUDManager.h"
#include "../Render/HUDMetrics.h"
#include "../Render/HUDRenderer.h"
#include "HUDSettings.h"

HUDManager::HUDManager() :
    mClock(
        HUDComponentType::Clock,
        {
            HUDPatch::Clock
        }),
    mHealth(
        HUDComponentType::Health,
        {
            HUDPatch::Health
        }),
    mArmour(
        HUDComponentType::Armour,
        {
            HUDPatch::Armour
        }),
    mBreath(
        HUDComponentType::Breath,
        {
            HUDPatch::Breath1,
            HUDPatch::Breath2
        }),
    mMoney(
        HUDComponentType::Money,
        {}),
    mWeaponIcon(
        HUDComponentType::Weapon,
        {
            HUDPatch::Weapon
        }),
    mAmmo(
        HUDComponentType::Ammo,
        {
            HUDPatch::Ammo
        }),
    mWanted(
        HUDComponentType::Wanted,
        {},
        false),
    mCellphoneIcon(HUDComponentType::Weapon, {}),
    mJetpackIcon(HUDComponentType::Weapon, {}),
    mSkateboardIcon(HUDComponentType::Weapon, {})
{
}

namespace
{
    static bool IsMeleeWeapon(
        eWeaponType weaponType)
    {
        return weaponType >= WEAPONTYPE_UNARMED &&
            weaponType <= WEAPONTYPE_CANE;
    }

    static bool IsDriveByWeapon(eWeaponType weaponType)
    {
        return weaponType == WEAPONTYPE_MICRO_UZI ||
            weaponType == WEAPONTYPE_TEC9 ||
            weaponType == WEAPONTYPE_MP5;
    }

    static bool IsThrowableWeapon(eWeaponType weaponType)
    {
        switch (weaponType)
        {
        case WEAPONTYPE_GRENADE:
        case WEAPONTYPE_TEARGAS:
        case WEAPONTYPE_MOLOTOV:
        case WEAPONTYPE_SATCHEL_CHARGE:
            return true;
        default:
            return false;
        }
    }

    static bool IsVisionEquipment(eWeaponType type)
    {
        return type == WEAPONTYPE_NIGHTVISION ||
            type == WEAPONTYPE_INFRARED;
    }

    enum class VehicleFireCapability
    {
        None,
        Bicycle,
        ForwardAndSide,
        SideOnly
    };

    static VehicleFireCapability GetVehicleFireCapability(
        const CVehicle& vehicle)
    {
        // Rhino and SWAT van do not use the driver's handheld
        // weapon. Fire trucks DO allow side drive-by shooting.
        switch (vehicle.m_nModelIndex)
        {
        case MODEL_RHINO:
        case MODEL_SWATVAN:
            return VehicleFireCapability::None;
        case MODEL_FIRETRUK:
        case MODEL_FIRELA:
            return VehicleFireCapability::SideOnly;
        default:
            break;
        }

        switch (vehicle.m_nVehicleSubClass)
        {
        case VEHICLE_BMX:
            return VehicleFireCapability::Bicycle;
        case VEHICLE_BIKE:
        case VEHICLE_QUAD:
            return VehicleFireCapability::ForwardAndSide;
        case VEHICLE_AUTOMOBILE:
        case VEHICLE_MTRUCK:
        case VEHICLE_BOAT:
            return VehicleFireCapability::SideOnly;
        default:
            return VehicleFireCapability::None;
        }
    }

    static bool IsLookingToOneSide(CPad& pad)
    {
        return !pad.GetLookBehindForCar() &&
            (pad.GetLookLeft() != pad.GetLookRight());
    }

    static bool AllowsHandheldFireDirection(
        const CVehicle& vehicle, CPad& pad)
    {
        switch (GetVehicleFireCapability(vehicle))
        {
        case VehicleFireCapability::SideOnly:
            return IsLookingToOneSide(pad);
        case VehicleFireCapability::ForwardAndSide:
            // Motorcycles can fire their handheld SMG forwards
            // even when both camera-look buttons are held.
            // Keep the existing rear-view restriction for quads.
            return vehicle.m_nVehicleSubClass == VEHICLE_BIKE ||
                !pad.GetLookBehindForCar();
        default:
            // Pedal-bike HUD remains based on ammo changes only.
            // A bunny-hop/fire button isn't evidence of a shot.
            return false;
        }
    }

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

    return state;
}

HUDManager::AmmoState HUDManager::GetCurrentAmmoState() const
{
    AmmoState state;

    if (!mPlayer)
        return state;

    const CWeapon* weapon = mPlayer->GetWeapon();

    if (!weapon)
        return state;

    state.AmmoInClip = weapon->m_nAmmoInClip;
    state.AmmoTotal = weapon->m_nAmmoTotal;

    return state;
}

bool HUDManager::HasWantedLevel() const
{
    if (!mPlayer || !mPlayer->m_pPlayerData)
        return false;

    const CWanted* wanted =
        mPlayer->m_pPlayerData->m_pWanted;

    if (!wanted)
        return false;

    return wanted->m_nWantedLevel > 0 ||
        wanted->m_nWantedLevelBeforeParole > 0;
}

bool HUDManager::IsInitialHealthTrackingBlocked() const
{
    if (!mPlayer)
        return true;

    CPad* pad =
        mPlayer->GetPadFromPlayer();

    if (!pad)
        return true;

    const bool cutsceneActive =
        CCutsceneMgr::ms_cutsceneProcessing ||
        pad->bPlayerSafeForCutscene;

    const bool controlsDisabled =
        pad->DisablePlayerControls != 0;

    const bool screenNotReady =
        TheCamera.m_bFading ||
        TheCamera.m_fFadeAlpha > 0.0f;

    return
        cutsceneActive ||
        controlsDisabled ||
        screenNotReady;
}

bool HUDManager::IsPlayerHUDInitialized() const
{
    return mSessionInitialized;
}

bool HUDManager::IsUsingVanillaLayout() const
{
    return
        HUDSettings::Get().Mode == HUDMode::Minimal ||
        !mSessionInitialized ||
        mVanillaLayoutLatched ||
        (GetAsyncKeyState(VK_TAB) & 0x8000);
}

bool HUDManager::ShouldDisplayAmmo() const
{
    if (!mPlayer)
        return false;

    const CWeapon* weapon =
        mPlayer->GetWeapon();

    if (!weapon)
        return false;

    constexpr unsigned int MaxClip = 9999;

    const eWeaponType weaponType =
        weapon->m_eWeaponType;

    const CWeaponInfo* weaponInfo =
        CWeaponInfo::GetWeaponInfo(
            weaponType);

    if (!weaponInfo)
        return false;

    if (IsMeleeWeapon(weaponType))
        return false;

    if (weapon->m_nAmmoTotal -
        weapon->m_nAmmoInClip >= MaxClip)
    {
        return false;
    }

    if (CDarkel::FrenzyOnGoing())
        return false;

    switch (weaponType)
    {
    case WEAPONTYPE_DETONATOR:
    case WEAPONTYPE_PARACHUTE:
        return false;

    default:
        break;
    }

    if (weaponInfo->m_nWeaponFire ==
        WEAPON_FIRE_USE)
    {
        return false;
    }

    return true;
}

bool HUDManager::ShouldDisplayArmour() const
{
    return mPlayer &&
        mPlayer->m_fArmour > 1.0f;
}

void HUDManager::RestoreVanillaElements()
{
    // During startup, loading, or an uninitialized session
    // always allow the game's original draw calls to run.
    // Restoring the calls does NOT force GTA to draw HUD items
    // when its own game state would normally hide them.
    //
    // CleanHUD takes ownership of the enabled elements once
    // health/session initialization has completed.
    mClock.Update(true);
    mHealth.Update(true);
    mArmour.Update(true);
    mBreath.Update(true);
    mWeaponIcon.Update(true);
    mAmmo.Update(true);
}

void HUDManager::RestoreDisabledElements()
{
    const auto& settings = HUDSettings::Get();

    // During managed gameplay, only elements explicitly disabled
    // in the INI should remain controlled by vanilla drawing.
    if (!settings.Clock)
        mClock.Update(true);
    if (!settings.Health)
        mHealth.Update(true);
    if (!settings.Armour)
        mArmour.Update(true);
    if (!settings.Breath)
        mBreath.Update(true);
    if (!settings.Weapon)
        mWeaponIcon.Update(true);
    if (!settings.Ammo)
        mAmmo.Update(true);
}

void HUDManager::OnGameLoaded()
{
    // The engine can render before FindPlayerPed() becomes
    // available, so return control to vanilla immediately.
    RestoreVanillaElements();
    HUDRenderer::Get().Reset();
    mSessionInitialized = false;
    mVanillaLayoutLatched = false;
    mValidVehicleFireInputLastFrame = false;
    mWeaponChangedThisFrame = false;
    mClockSpacingPreview.Reset();
    mCellphoneIcon.Reset();
    mJetpackIcon.Reset();
    mSkateboardIcon.Reset();
    mJetpackWasEquipped = false;
    mSkateboardWasRiding = false;
    mCellphonePuttingAway = false;

    mPlayer = nullptr;
    mLastPlayer = nullptr;

    mNeedsReset = true;
}

void HUDManager::Update()
{
    mPlayer = FindPlayerPed();

    if (!mPlayer)
    {
        mSessionInitialized = false;
        mLastPlayer = nullptr;
        mValidVehicleFireInputLastFrame = false;
        mWeaponChangedThisFrame = false;
        mCellphoneIcon.Reset();
        mJetpackIcon.Reset();
        mSkateboardIcon.Reset();
        mJetpackWasEquipped = false;
        mSkateboardWasRiding = false;
        mCellphonePuttingAway = false;

        // Returning from gameplay to a frontend or loading
        // screen must not leave our draw-call NOPs in place.
        RestoreVanillaElements();
        HUDRenderer::Get().Reset();
        mLayout.ResetHistory();
        return;
    }

    const auto& settings = HUDSettings::Get();

    mVisibleTime = settings.VisibilityMs;

    const bool tabHeld =
        GetAsyncKeyState(VK_TAB) & 0x8000;

    if (tabHeld)
    {
        mVanillaLayoutLatched = true;
    }

    const auto origin =
        HUDRenderer::Get().GetAnchor();

    mLayout.SetAnchor(
        origin.X,
        origin.Y);
    mLayout.Reset();

    if (mNeedsReset)
    {
        ResetSessionState();
        mNeedsReset = false;
    }

    if (mPlayer != mLastPlayer)
    {
        mLastPlayer = mPlayer;
        mSessionInitialized = false;
        mNeedsReset = true;
        return;
    }

    bool isDead = (mPlayer->m_fHealth <= 0.0f);

    if (mWasDead && !isDead)
    {
        mSessionInitialized = false;
        mNeedsReset = true;
        mWasDead = false;
        return;
    }

    mWasDead = isDead;

    const bool wasSessionInitialized = mSessionInitialized;
    UpdateHealth();

    if (!mSessionInitialized)
    {
        // Keep the restored game draw calls available until
        // the player/health stabilizes. DrawPlayerInfo's
        // initialization gate prevents the loading HUD flash.
        RestoreVanillaElements();
        HUDRenderer::Get().Reset();
        mLayout.ResetHistory();
        return;
    }

    if (!wasSessionInitialized)
    {
        // The startup fallback used Update(true), leaving
        // components partly/fully visible even though the HUD
        // draw was suppressed. The earlier working build began
        // each managed session with HideAll() instead.
        //
        // Clear those animation/timer states before constructing
        // CleanHUD's first visible gameplay layout.
        HideAll();
        RestoreDisabledElements();
        mLayout.ResetHistory();
    }

    if (tabHeld || !settings.Clock || settings.AlwaysShowClock)
        mClock.Show();
    else if (!mClockSpacingPreview.Visible())
        mClock.Hide();

    UpdateArmour();
    UpdateBreath();
    UpdateMoney();
    mWeaponChangedThisFrame = false;
    UpdateWeaponIcon();
    UpdateAmmo();
    UpdateSpecialIcons();
    UpdateSkateboardIcon();
    mCellphoneIcon.Update();
    mJetpackIcon.Update();
    mSkateboardIcon.Update();
    UpdateVehicleFiring();
    UpdateWanted();

    // Aim-only visibility fades on release. Shot timers remain
    // active rather than being erased by HUDComponent::Hide().
    const bool weaponAiming = IsWeaponAiming();

    mClock.Update(!settings.Clock || settings.AlwaysShowClock);

    // Disabled CleanHUD elements retain vanilla draw calls.
    // HUDLayout::Add excludes them from CleanHUD's stack.
    mHealth.Update(tabHeld || !settings.Health);
    mArmour.Update(tabHeld || !settings.Armour);
    mBreath.Update(tabHeld || !settings.Breath);
    mMoney.Update(tabHeld || !settings.Money);
    mWeaponIcon.Update(
        tabHeld || weaponAiming || !settings.Weapon);
    mAmmo.Update(
        tabHeld || !settings.Ammo || weaponAiming);

    mWanted.Update(!settings.Wanted);

    if (HasEmptyVehicleAmmo())
    {
        if (settings.Weapon)
            mWeaponIcon.Reset();

        if (settings.Ammo)
            mAmmo.Reset();
    }

    // Prepare one-way stack displacement from what was already
    // visible in the preceding frame. Order must match layout.
    if (settings.Mode == HUDMode::Stack)
    {
        mLayout.PrepareStack({
            &mClock, &mMoney, &mWanted, &mArmour,
            &mBreath, &mHealth, &mWeaponIcon, &mAmmo
        });
        BuildPlayerHUDLayout();
    }
    else
    {
        // Minimal mode uses the same components and timers,
        // but does not shift any of GTA's HUD coordinates.
        mLayout.ResetHistory();
        BuildMinimalHUDLayout();
    }

    HUDRenderer::Get().Build(mLayout);

    if (!tabHeld)
    {
        const bool tabFadeOutActive =
            mClock.IsFadingOut() ||
            mMoney.IsFadingOut() ||
            mHealth.IsFadingOut() ||
            mArmour.IsFadingOut() ||
            mBreath.IsFadingOut() ||
            mWeaponIcon.IsFadingOut() ||
            mAmmo.IsFadingOut();

        if (!tabFadeOutActive)
        {
            mVanillaLayoutLatched = false;
        }
    }
}

void HUDManager::ResetSessionState()
{
    HideAll();
    RestoreVanillaElements();
    mLayout.ResetHistory();
    mValidVehicleFireInputLastFrame = false;
    mWeaponChangedThisFrame = false;

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

    mAmmoStableFrames = 0;
    mWaitingForStableAmmo = true;

    SyncTrackedValues();
}

void HUDManager::SyncTrackedValues()
{
    mLastHealth = mPlayer->m_fHealth;
    mLastArmour = mPlayer->m_fArmour;
    mLastMoney = CWorld::Players[CWorld::PlayerInFocus].m_nMoney;

    mWasDead = (mPlayer->m_fHealth <= 0.0f);
    mLastWeaponState = GetCurrentWeaponState();
    mLastAmmoState = GetCurrentAmmoState();
}

bool HUDManager::UpdateHealthInitialization(float currentHealth)
{
    if (!mWaitingForStableHealth)
        return true;

    if (IsInitialHealthTrackingBlocked())
    {
        mSessionInitialized = false;

        mHealthStableFrames = 0;

        mLastHealth =
            currentHealth;

        mPreviousHealth =
            currentHealth;

        mHealth.Hide();
        mHealth.Update();

        return false;
    }

    if (currentHealth == mPreviousHealth)
    {
        mHealthStableFrames++;

        if (mHealthStableFrames >= 2)
        {
            mWaitingForStableHealth = false;
            mSessionInitialized = true;
        }
    }
    else
    {
        mHealthStableFrames = 0;
    }

    mLastHealth =
        currentHealth;

    mPreviousHealth =
        currentHealth;

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

    return false;
}

bool HUDManager::UpdateAmmoInitialization(const AmmoState& current)
{
    if (!mWaitingForStableAmmo)
        return true;

    if (current == mLastAmmoState)
    {
        mAmmoStableFrames++;

        if (mAmmoStableFrames >= 2)
            mWaitingForStableAmmo = false;
    }
    else
    {
        mAmmoStableFrames = 0;
        mLastAmmoState = current;
    }

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

bool HUDManager::HasAmmoChanged(const AmmoState& current)
{
    if (current == mLastAmmoState)
        return false;

    mLastAmmoState = current;

    return true;
}

void HUDManager::UpdateHealth()
{
    const float currentHealth =
        mPlayer->m_fHealth;

    if (!HUDSettings::Get().Health)
    {
        // Disabled Health is vanilla from startup, not a
        // tracked CleanHUD element awaiting stabilization.
        mLastHealth = currentHealth;
        mPreviousHealth = currentHealth;
        mWaitingForStableHealth = false;
        mSessionInitialized = true;
        return;
    }

    if (!UpdateHealthInitialization(
        currentHealth))
    {
        return;
    }

    const bool healthChanged =
        HasHealthChanged(currentHealth);

    const bool criticallyLow =
        static_cast<int>(currentHealth) < 10;

    if (criticallyLow)
    {
        mHealth.Show();
        return;
    }

    if (healthChanged)
    {
        mHealth.ShowFor(
            mVisibleTime);
    }
}

void HUDManager::UpdateArmour()
{
    float currentArmour = mPlayer->m_fArmour;

    if (!UpdateArmourInitialization(currentArmour))
        return;

    if (HasArmourChanged(currentArmour))
        mArmour.ShowFor(mVisibleTime);
}

void HUDManager::UpdateBreath()
{
    if (!mPlayer ||
        !mPlayer->m_pPlayerData)
    {
        mBreath.Hide();
        return;
    }

    auto* data =
        mPlayer->m_pPlayerData;

    const float maxBreath =
        CStats::GetFatAndMuscleModifier(
            STAT_MOD_AIR_IN_LUNG);

    const bool breathDepleted =
        data->m_fBreath < maxBreath;

    bool underwater = false;

    if (mPlayer->m_pIntelligence)
    {
        if (auto* swim =
            mPlayer->m_pIntelligence->GetTaskSwim())
        {
            underwater =
                swim->m_nSwimState ==
                SWIM_DIVE_UNDERWATER ||
                swim->m_nSwimState ==
                SWIM_UNDERWATER_SPRINTING;
        }
    }

    if (underwater || breathDepleted)
    {
        mBreath.Show();
    }
    else
    {
        mBreath.Hide();
    }
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
}

void HUDManager::UpdateWeaponIcon()
{
    ActiveWeaponState current = GetCurrentWeaponState();

    if (!UpdateWeaponInitialization(current))
        return;

    if (HasWeaponChanged(current))
    {
        mWeaponChangedThisFrame = true;

        // Goggles behave like other equipment on selection:
        // only their aim-button HUD trigger is suppressed.
        mWeaponIcon.ShowFor(mVisibleTime);
        mAmmo.ShowFor(mVisibleTime);
    }
}

void HUDManager::UpdateAmmo()
{
    const AmmoState previous = mLastAmmoState;
    const AmmoState current = GetCurrentAmmoState();

    if (!UpdateAmmoInitialization(current))
        return;

    if (!HasAmmoChanged(current))
        return;

    const CWeapon* weapon = mPlayer->GetWeapon();

    if (!weapon)
        return;

    if (!mPlayer->bInVehicle)
    {
        mAmmo.ShowFor(mVisibleTime);
        mWeaponIcon.ShowFor(mVisibleTime);
        return;
    }

    const CVehicle* vehicle = mPlayer->m_pVehicle;

    if (!vehicle)
        return;

    const auto capability = GetVehicleFireCapability(*vehicle);

    if (capability == VehicleFireCapability::Bicycle)
    {
        // Existing pedal-bike path: no inference from Fire,
        // which also performs bunny hops.
        mAmmo.ShowFor(mVisibleTime);
        mWeaponIcon.ShowFor(mVisibleTime);
        return;
    }

    if (capability == VehicleFireCapability::None)
        return;

    const bool consumedAmmo =
        current.AmmoInClip < previous.AmmoInClip ||
        current.AmmoTotal < previous.AmmoTotal;

    // A weapon change or reload isn't a confirmed drive-by.
    // Allow the previous frame's valid input because input
    // and weapon processing can occur on different ticks.
    if (!mWeaponChangedThisFrame &&
        consumedAmmo &&
        IsDriveByWeapon(weapon->m_eWeaponType) &&
        (IsValidVehicleFireInput() ||
            mValidVehicleFireInputLastFrame))
    {
        mAmmo.ShowFor(mVisibleTime);
        mWeaponIcon.ShowFor(mVisibleTime);
    }
}

bool HUDManager::IsValidVehicleFireInput() const
{
    if (!mPlayer || !mPlayer->bInVehicle ||
        !mPlayer->m_pVehicle)
    {
        return false;
    }

    const CWeapon* weapon = mPlayer->GetWeapon();

    if (!weapon ||
        !IsDriveByWeapon(weapon->m_eWeaponType) ||
        weapon->m_nAmmoTotal == 0)
    {
        return false;
    }

    CPad* pad = mPlayer->GetPadFromPlayer();

    return pad && !pad->DisablePlayerControls &&
        pad->GetCarGunFired() != 0 &&
        AllowsHandheldFireDirection(
            *mPlayer->m_pVehicle, *pad);
}

void HUDManager::UpdateVehicleFiring()
{
    const bool validInput = IsValidVehicleFireInput();
    const CWeapon* weapon = mPlayer->GetWeapon();

    // The weapon's FIRING state confirms a handheld shot,
    // including shots not deducted from total ammunition.
    // This does not trigger for fire-button-only input.
    if (validInput && weapon &&
        weapon->m_nState == WEAPONSTATE_FIRING)
    {
        mWeaponIcon.ShowFor(mVisibleTime);
        mAmmo.ShowFor(mVisibleTime);
    }

    mValidVehicleFireInputLastFrame = validInput;
}

bool HUDManager::IsWeaponAiming() const
{
    if (mWaitingForStableWeapon ||
        mWaitingForStableAmmo ||
        !ShouldDisplayAmmo())
    {
        return false;
    }

    const CWeapon* weapon = mPlayer->GetWeapon();

    if (!weapon || weapon->m_nAmmoTotal == 0 ||
        IsVisionEquipment(weapon->m_eWeaponType))
    {
        return false;
    }

    CPad* pad = mPlayer->GetPadFromPlayer();

    if (!pad || pad->DisablePlayerControls)
        return false;

    const auto& settings = HUDSettings::Get();

    if (!mPlayer->bInVehicle)
    {
        return settings.ShowOnFootAim &&
            (settings.ShowOnThrowableAim ||
                !IsThrowableWeapon(weapon->m_eWeaponType)) &&
            pad->GetTarget();
    }

    const CVehicle* vehicle = mPlayer->m_pVehicle;

    // Side-looking preview remains opt-in. Bikes/quadbikes,
    // ordinary cars and boats can preview an eligible SMG;
    // aircraft, vehicle weapons and pedal bikes cannot.
    return settings.ShowOnVehicleLook &&
        vehicle &&
        IsDriveByWeapon(weapon->m_eWeaponType) &&
        GetVehicleFireCapability(*vehicle) !=
            VehicleFireCapability::Bicycle &&
        IsLookingToOneSide(*pad) &&
        AllowsHandheldFireDirection(*vehicle, *pad);
}

bool HUDManager::HasEmptyVehicleAmmo() const
{
    if (!mPlayer || !mPlayer->bInVehicle ||
        !ShouldDisplayAmmo())
    {
        return false;
    }

    const CWeapon* weapon = mPlayer->GetWeapon();

    return weapon &&
        weapon->m_nAmmoTotal == 0 &&
        weapon->m_nAmmoInClip == 0;
}

void HUDManager::UpdateWanted()
{
    if (HasWantedLevel())
        mWanted.Show();
    else
        mWanted.Hide();
}

void HUDManager::BuildMinimalHUDLayout()
{
    // Keep only visibility/alpha data in HUDRenderer. Hooks
    // use IsUsingVanillaLayout() to retain original GTA coords.
    // No stack height or spacing is reserved in this mode.
    mLayout.Add(mClock, 0.0f, 0.0f, 0.0f);
    mLayout.Add(mMoney, 0.0f, 0.0f, 0.0f);
    mLayout.Add(mWanted, 0.0f, 0.0f, 0.0f);
    mLayout.Add(mArmour, 0.0f, 0.0f, 0.0f);
    mLayout.Add(mBreath, 0.0f, 0.0f, 0.0f);
    mLayout.Add(mHealth, 0.0f, 0.0f, 0.0f);
    mLayout.Add(mWeaponIcon, 0.0f, 0.0f, 0.0f);

    if (ShouldDisplayAmmo())
        mLayout.Add(mAmmo, 0.0f, 0.0f, 0.0f);
}

void HUDManager::BuildPlayerHUDLayout()
{
    constexpr float ClockHeight = 18.0f;
    constexpr float MoneyHeight = 18.0f;
    constexpr float WantedHeight = 18.0f;
    const auto& settings = HUDSettings::Get();
    const float statusBarHeight =
        HUDMetrics::GetStatusBarSpacing();

    const bool ammoDisplayed =
        settings.Ammo && ShouldDisplayAmmo();

    const bool armourDisplayed =
        settings.Armour && ShouldDisplayArmour();

    const bool moneyDisplayed =
        settings.Money && mMoney.IsVisible();

    const bool wantedDisplayed =
        settings.Wanted && mWanted.IsVisible();

    const bool statusDisplayed =
        (armourDisplayed && mArmour.IsVisible()) ||
        (settings.Breath && mBreath.IsVisible()) ||
        (settings.Health && mHealth.IsVisible());

    const bool weaponDisplayed =
        (settings.Weapon && mWeaponIcon.IsVisible()) ||
        (ammoDisplayed && mAmmo.IsVisible());

    // Resolve the spacing to whichever group is next
    // in the visible stack, not the previous Weapon-first order.
    float moneySpacing = 0.0f;

    if (wantedDisplayed)
        moneySpacing = settings.MoneyToWantedSpacing;
    else if (statusDisplayed)
        moneySpacing = settings.MoneyToStatusBarSpacing;
    else if (weaponDisplayed)
        moneySpacing = settings.MoneyToWeaponSpacing;

    float wantedSpacing = 0.0f;

    if (wantedDisplayed)
    {
        if (statusDisplayed)
            wantedSpacing = settings.WantedToStatusBarSpacing;
        else if (weaponDisplayed)
            wantedSpacing = settings.WantedToWeaponSpacing;
    }

    // The Clock is a fixed top-of-stack anchor. Its trailing
    // gap belongs to whichever group currently sits directly
    // below it, including when Clock=0 reserves vanilla space.
    // When no group is visible the gap has no visible effect.
    float clockSpacing = 0.0f;

    if (moneyDisplayed)
        clockSpacing = settings.ClockToMoneySpacing;
    else if (wantedDisplayed)
        clockSpacing = settings.ClockToWantedSpacing;
    else if (statusDisplayed)
        clockSpacing = settings.ClockToStatusBarSpacing;
    else if (weaponDisplayed)
        clockSpacing = settings.ClockToWeaponSpacing;

    if (!settings.Clock)
    {
        mLayout.ReserveSpace(
            ClockHeight,
            clockSpacing);
    }
    else
    {
        mLayout.Add(
            mClock,
            ClockHeight,
            0.0f,
            clockSpacing);
    }

    mLayout.Add(
        mMoney,
        MoneyHeight,
        -7.0f,
        moneySpacing);

    mLayout.Add(
        mWanted,
        WantedHeight,
        0.0f,
        wantedSpacing);

    // Armour, Breath and Health are now above Weapon/Ammo.
    // The final visible bar owns the bar-to-weapon gap.
    const bool armourShown =
        armourDisplayed && mArmour.IsVisible();

    const bool breathShown =
        settings.Breath && mBreath.IsVisible();

    const bool healthShown =
        settings.Health && mHealth.IsVisible();

    if (armourDisplayed)
    {
        mLayout.Add(
            mArmour,
            statusBarHeight,
            0.0f,
            weaponDisplayed && armourShown &&
                !breathShown && !healthShown
                ? settings.StatusBarToWeaponSpacing
                : settings.ArmourSpacing);
    }

    mLayout.Add(
        mBreath,
        statusBarHeight,
        0.0f,
        weaponDisplayed && breathShown && !healthShown
            ? settings.StatusBarToWeaponSpacing
            : settings.BreathSpacing);

    mLayout.Add(
        mHealth,
        statusBarHeight,
        0.0f,
        weaponDisplayed && healthShown
            ? settings.StatusBarToWeaponSpacing
            : 0.0f);

    // Weapon and Ammo share a group origin. Weapon reserves the
    // configured distance to Ammo rather than using a fake height
    // and cancelling trailing spacing.
    mLayout.Add(
        mWeaponIcon,
        settings.WeaponToAmmoVerticalOffset,
        settings.WeaponAmmoOffsetY,
        0.0f,
        settings.WeaponAmmoOffsetX);

    if (ammoDisplayed)
    {
        const float weaponReservation =
            settings.Weapon
            ? mLayout.GetStackProgress(HUDComponentType::Weapon)
            : 1.0f;

        // Preserve the existing one-way slide behavior when Ammo
        // and Weapon enter the stack at different fade progress.
        const float ammoPositionCorrection =
            mLayout.UsesFinalEntryPosition(HUDComponentType::Ammo)
            ? 0.0f
            : settings.WeaponToAmmoVerticalOffset *
                (1.0f - weaponReservation);

        // Ammo is the final item: its trailing reservation is unused.
        mLayout.Add(
            mAmmo,
            0.0f,
            settings.WeaponAmmoOffsetY + ammoPositionCorrection,
            0.0f,
            settings.WeaponAmmoOffsetX);
    }
}

void HUDManager::PreviewSpacing(
    HUDSpacingPreviewGroup first,
    HUDSpacingPreviewGroup second)
{
    // SleekConfig callbacks run on the game thread. Never surface HUD
    // components during new-game/load startup or frontend transitions.
    if (!mSessionInitialized || !mPlayer || mNeedsReset ||
        HUDSettings::Get().Mode != HUDMode::Stack)
    {
        return;
    }

    PreviewSpacingGroup(first);

    if (second != first)
        PreviewSpacingGroup(second);
}

void HUDManager::PreviewJetpackPosition()
{
    const auto& settings = HUDSettings::Get();

    // Do not invent an equipped jetpack or revive one being dropped.
    // Unlike Stack spacing previews, this also works in Minimal mode.
    if (!mSessionInitialized || !mPlayer || mNeedsReset ||
        !settings.ShowHiddenWeaponIcons ||
        !mPlayer->m_pIntelligence ||
        settings.VisibilityMs == 0)
    {
        return;
    }

    // Only a real, unfinished jetpack task can be previewed. This
    // rejects both midair F presses (which don't end that task) and
    // old finished tasks waiting for the engine to detach them.
    const CTaskSimpleJetPack* task =
        mPlayer->m_pIntelligence->GetTaskJetPack();
    if (!task || task->m_bIsFinished)
        return;

    mJetpackIcon.ShowFor(settings.VisibilityMs);
    ShowWeaponAndAmmoForEquipment(settings.VisibilityMs);
}

void HUDManager::ShowWeaponAndAmmoForEquipment(
    std::uint32_t duration)
{
    // Keep weapon and ammo on the same visibility window when
    // riding a skateboard or equipping a jetpack.
    mWeaponIcon.ShowFor(duration);

    if (HUDSettings::Get().Ammo && ShouldDisplayAmmo())
        mAmmo.ShowFor(duration);
}

void HUDManager::PreviewSkateboardPosition()
{
    const auto& settings = HUDSettings::Get();
    if (!mSessionInitialized || !mPlayer || mNeedsReset ||
        settings.VisibilityMs == 0 ||
        !IsRidingConfiguredSkateboard())
    {
        return;
    }

    mSkateboardIcon.ShowFor(settings.VisibilityMs);
    ShowWeaponAndAmmoForEquipment(settings.VisibilityMs);
}

void HUDManager::PreviewSpacingGroup(HUDSpacingPreviewGroup group)
{
    const auto& settings = HUDSettings::Get();
    const std::uint32_t duration = settings.VisibilityMs;

    // Respect a zero-duration configuration and user-disabled elements.
    if (duration == 0)
        return;

    switch (group)
    {
    case HUDSpacingPreviewGroup::Clock:
        if (settings.Clock)
        {
            if (!settings.AlwaysShowClock)
            {
                mClockSpacingPreview.ShowFor(duration);
                mClock.ShowFor(duration);
            }
            // If AlwaysShowClock is on, the regular update already
            // keeps the clock in view. Clock=0 uses vanilla drawing.
        }
        break;

    case HUDSpacingPreviewGroup::Money:
        if (settings.Money)
            mMoney.ShowFor(duration);
        break;

    case HUDSpacingPreviewGroup::Wanted:
        // The game only draws real wanted stars for an active wanted
        // level. Do not fake a wanted level or reserve invisible space
        // when the player has no stars.
        if (settings.Wanted && HasWantedLevel())
            mWanted.Show();
        break;

    case HUDSpacingPreviewGroup::StatusBars:
        // Health is the reliable visible reference bar; only include
        // Armour/Breath when the game can actually display them.
        PreviewSpacingGroup(HUDSpacingPreviewGroup::Health);
        PreviewSpacingGroup(HUDSpacingPreviewGroup::Armour);
        PreviewSpacingGroup(HUDSpacingPreviewGroup::Breath);
        break;

    case HUDSpacingPreviewGroup::Weapon:
        if (!HasEmptyVehicleAmmo())
        {
            if (settings.Weapon)
                mWeaponIcon.ShowFor(duration);

            if (settings.Ammo && ShouldDisplayAmmo())
                mAmmo.ShowFor(duration);
        }
        break;

    case HUDSpacingPreviewGroup::Ammo:
        if (settings.Ammo && ShouldDisplayAmmo() &&
            !HasEmptyVehicleAmmo())
        {
            mAmmo.ShowFor(duration);
        }
        break;

    case HUDSpacingPreviewGroup::Armour:
        if (settings.Armour && ShouldDisplayArmour())
            mArmour.ShowFor(duration);
        break;

    case HUDSpacingPreviewGroup::Breath:
        // Outside underwater/depleted-air gameplay the vanilla
        // breath bar is not drawable; do not create an empty slot.
        if (settings.Breath && mBreath.IsVisible())
            mBreath.ShowFor(duration);
        break;

    case HUDSpacingPreviewGroup::Health:
        if (settings.Health)
            mHealth.ShowFor(duration);
        break;
    }
}

bool HUDManager::IsRidingConfiguredSkateboard() const
{
    const auto& settings = HUDSettings::Get();
    return settings.ShowHiddenWeaponIcons &&
        settings.ShowSkateboardRidingIcon &&
        mSessionInitialized && mPlayer &&
        mPlayer->bInVehicle && mPlayer->m_pVehicle &&
        settings.SkateboardVehicleModelId > 0 &&
        static_cast<std::uint32_t>(
            mPlayer->m_pVehicle->m_nModelIndex) ==
        settings.SkateboardVehicleModelId;
}

std::uint8_t HUDManager::GetSkateboardIconAlpha() const
{
    const auto& settings = HUDSettings::Get();
    return settings.ShowHiddenWeaponIcons &&
        settings.ShowSkateboardRidingIcon && mSessionInitialized
        ? mSkateboardIcon.GetAlpha() : 0;
}

int HUDManager::GetSkateboardIconModel() const
{
    return static_cast<int>(HUDSettings::Get().SkateboardIconModelId);
}

bool HUDManager::ShouldDrawSkateboardIcon() const
{
    if (GetSkateboardIconAlpha() == 0 || !mPlayer)
        return false;

    const auto modelId = GetSkateboardIconModel();
    if (modelId <= 0 || modelId >= CModelInfo::ms_modelInfoCount ||
        !CModelInfo::GetModelInfo(modelId))
    {
        return false;
    }

    // After dismount, the CLEO script can restore the same board as
    // a held weapon. Avoid drawing two copies during its fade-out.
    const CWeapon* weapon = mPlayer->GetWeapon();
    if (!IsRidingConfiguredSkateboard() && weapon &&
        weapon->m_eWeaponType != WEAPONTYPE_UNARMED &&
        mPlayer->m_nWeaponModelId == modelId)
    {
        return false;
    }

    return true;
}

bool HUDManager::ShouldReplaceWeaponWithSkateboard() const
{
    if (!ShouldDrawSkateboardIcon() ||
        ShouldDrawCellphoneIcon() ||
        ShouldDrawJetpackIcon() || !mPlayer)
    {
        return false;
    }

    const CWeapon* weapon = mPlayer->GetWeapon();
    if (!weapon)
        return false;

    return weapon->m_eWeaponType == WEAPONTYPE_UNARMED ||
        weapon->m_eWeaponType == WEAPONTYPE_BRASSKNUCKLE;
}

std::uint8_t HUDManager::GetCellphoneIconAlpha() const
{
    return HUDSettings::Get().ShowHiddenWeaponIcons && mSessionInitialized
        ? mCellphoneIcon.GetAlpha() : 0;
}

std::uint8_t HUDManager::GetJetpackIconAlpha() const
{
    // An unequip ends the *timer*, not the drawn fade-out.
    // The alpha remains live until HUDComponent::Update reaches zero.
    return HUDSettings::Get().ShowHiddenWeaponIcons &&
        mSessionInitialized ? mJetpackIcon.GetAlpha() : 0;
}

bool HUDManager::ShouldDrawCellphoneIcon() const
{
    return GetCellphoneIconAlpha() != 0;
}

bool HUDManager::ShouldDrawJetpackIcon() const
{
    return GetJetpackIconAlpha() != 0;
}

bool HUDManager::ShouldReplaceWeaponWithJetpack() const
{
    if (!ShouldDrawJetpackIcon() || !mPlayer)
        return false;

    const CWeapon* weapon = mPlayer->GetWeapon();
    if (!weapon)
        return false;

    // Only truly unarmed slots are replaced. Throwables, melee,
    // utility equipment and custom weapons have their own weapon
    // icons, so keep them and place the jetpack alongside.
    // A visible cellphone already occupies the main icon slot.
    const eWeaponType type = weapon->m_eWeaponType;
    return !ShouldDrawCellphoneIcon() &&
        (type == WEAPONTYPE_UNARMED ||
         type == WEAPONTYPE_BRASSKNUCKLE);
}

bool HUDManager::ShouldSuppressNativeFist() const
{
    return ShouldDrawCellphoneIcon() ||
        ShouldReplaceWeaponWithJetpack() ||
        ShouldReplaceWeaponWithSkateboard();
}

bool HUDManager::ShouldSuppressNativeWeaponSprite() const
{
    if (!mPlayer)
        return false;

    const CWeapon* weapon = mPlayer->GetWeapon();
    if (!weapon)
        return false;

    if (ShouldReplaceWeaponWithJetpack() ||
        ShouldReplaceWeaponWithSkateboard())
        return true;

    // During a phone task, suppress only knuckles; firearm icons
    // continue through GTA's ordinary weapon rendering path.
    return ShouldDrawCellphoneIcon() &&
        weapon->m_eWeaponType == WEAPONTYPE_BRASSKNUCKLE;
}

int HUDManager::GetCustomWeaponIconModel() const
{
    if (!HUDSettings::Get().ShowHiddenWeaponIcons ||
        !mSessionInitialized || !mPlayer)
    {
        return 0;
    }

    const CWeapon* weapon = mPlayer->GetWeapon();

    if (!weapon || !mPlayer->m_pWeaponObject)
        return 0;

    const eWeaponType weaponType = weapon->m_eWeaponType;

    // Values above UNUSED are GTA damage/event types, not safe
    // weapon-info indices. Unarmed has no held weapon icon.
    if (weaponType <= WEAPONTYPE_UNARMED ||
        weaponType > WEAPONTYPE_UNUSED)
    {
        return 0;
    }

    // GTA tracks the model actually attached to the player's hand,
    // including when a weapon mod swaps Cane/Shovel for a skateboard.
    const int heldModelId = mPlayer->m_nWeaponModelId;

    if (heldModelId <= 0 ||
        heldModelId >= CModelInfo::ms_modelInfoCount ||
        !CModelInfo::GetModelInfo(heldModelId))
    {
        return 0;
    }

    // Native CHud and WeaponIconsTXD already draw the regular weapon
    // model's icon. Only supplement a DIFFERENT held model, so
    // ordinary weapons are never drawn twice.
    const CWeaponInfo* info = CWeaponInfo::GetWeaponInfo(
        weaponType, mPlayer->GetWeaponSkill());

    if (!info || heldModelId == info->m_nModelId ||
        heldModelId == info->m_nModelId2)
    {
        return 0;
    }

    return heldModelId;
}

void HUDManager::UpdateSpecialIcons()
{
    const auto& settings = HUDSettings::Get();

    if (!settings.ShowHiddenWeaponIcons || !mPlayer ||
        !mPlayer->m_pIntelligence)
    {
        mCellphoneIcon.Reset();
        mJetpackIcon.Reset();
        mJetpackWasEquipped = false;
        mCellphonePuttingAway = false;
        return;
    }

    CPedIntelligence* intelligence = mPlayer->m_pIntelligence;
    const CTaskSimpleJetPack* jetpackTask =
        intelligence->GetTaskJetPack();

    // GTA ignores the exit button while airborne. The jetpack task
    // itself is the source of truth: only start fading when the game
    // has marked the task finished or actually removed the task.
    const bool jetpackEquipped =
        jetpackTask && !jetpackTask->m_bIsFinished;

    if (!jetpackEquipped)
    {
        mJetpackIcon.Hide();
    }
    else if (!mJetpackWasEquipped)
    {
        // Normal CleanHUD visibility starts once per actual equip,
        // not on every frame of continued flight.
        mJetpackIcon.ShowFor(mVisibleTime);
        ShowWeaponAndAmmoForEquipment(mVisibleTime);
    }
    mJetpackWasEquipped = jetpackEquipped;

    // PHONE_OUT may still belong to the parent phone-use task.
    // Stop extending the icon when this task begins, but allow the
    // usual CleanHUD FadeOutMs instead of resetting its alpha.
    const bool puttingPhoneAway =
        intelligence->FindTaskByType(TASK_SIMPLE_PHONE_OUT) != nullptr;
    const bool anyPhoneTask = puttingPhoneAway ||
        intelligence->FindTaskByType(TASK_COMPLEX_USE_MOBILE_PHONE) ||
        intelligence->FindTaskByType(TASK_SIMPLE_PHONE_TALK) ||
        intelligence->FindTaskByType(TASK_SIMPLE_PHONE_IN);

    if (puttingPhoneAway)
        mCellphonePuttingAway = true;
    else if (!anyPhoneTask)
        mCellphonePuttingAway = false;

    // Keep the fade running after PHONE_OUT, even if its parent
    // complex task remains attached for another few frames.
    const bool cellphoneEquipped = anyPhoneTask &&
        !mCellphonePuttingAway;

    if (cellphoneEquipped)
    {
        // Hold the phone visible during the active task, not afterward.
        mCellphoneIcon.ShowFor(mVisibleTime);
        mWeaponIcon.ShowFor(mVisibleTime);
    }
    else
    {
        mCellphoneIcon.Hide();
    }
}

void HUDManager::UpdateSkateboardIcon()
{
    const auto& settings = HUDSettings::Get();

    if (!settings.ShowHiddenWeaponIcons ||
        !settings.ShowSkateboardRidingIcon)
    {
        mSkateboardIcon.Reset();
        mSkateboardWasRiding = false;
        return;
    }

    const bool riding = IsRidingConfiguredSkateboard();
    if (riding && !mSkateboardWasRiding)
    {
        mSkateboardIcon.ShowFor(mVisibleTime);
        ShowWeaponAndAmmoForEquipment(mVisibleTime);
    }
    else if (!riding)
    {
        // Leaving this vehicle ends the ride, not the FadeOutMs
        // animation. A held skateboard can take over its own slot.
        mSkateboardIcon.Hide();
    }

    mSkateboardWasRiding = riding;
}

void HUDManager::HideAll()
{
    mClockSpacingPreview.Reset();
    mCellphoneIcon.Reset();
    mJetpackIcon.Reset();
    mSkateboardIcon.Reset();
    mJetpackWasEquipped = false;
    mSkateboardWasRiding = false;
    mCellphonePuttingAway = false;
    mClock.Reset();
    mHealth.Reset();
    mArmour.Reset();
    mBreath.Reset();
    mMoney.Reset();
    mWeaponIcon.Reset();
    mAmmo.Reset();
    mWanted.Reset();
}

HUDManager& HUDManager::Get()
{
    static HUDManager instance;
    return instance;
}