#include "HUDRenderHooks.h"

#include "AmmoRenderHook.h"
#include "ArmourRenderHook.h"
#include "BreathRenderHook.h"
#include "HealthRenderHook.h"
#include "MoneyRenderHook.h"
#include "WantedRenderHook.h"
#include "WeaponRenderHook.h"

void HUDRenderHooks::Install()
{
    ArmourRenderHook::Install();
    BreathRenderHook::Install();
    HealthRenderHook::Install();
    MoneyRenderHook::Install();
    WantedRenderHook::Install();
    WeaponRenderHook::Install();
    AmmoRenderHook::Install();
}