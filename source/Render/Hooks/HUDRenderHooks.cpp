#include "HUDRenderHooks.h"

#include "AmmoRenderHook.h"
#include "ArmourRenderHook.h"
#include "BreathRenderHook.h"
#include "ClockRenderHook.h"
#include "HealthRenderHook.h"
#include "MoneyRenderHook.h"
#include "WantedRenderHook.h"
#include "WeaponRenderHook.h"

void HUDRenderHooks::Install()
{
    AmmoRenderHook::Install();
    ArmourRenderHook::Install();
    BreathRenderHook::Install();
    ClockRenderHook::Install();
    HealthRenderHook::Install();
    MoneyRenderHook::Install();
    WantedRenderHook::Install();
    WeaponRenderHook::Install();
}