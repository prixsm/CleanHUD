#include "HUDRenderHooks.h"

#include "ArmourRenderHook.h"
#include "BreathRenderHook.h"
#include "HealthRenderHook.h"
#include "MoneyRenderHook.h"
#include "WantedRenderHook.h"

void HUDRenderHooks::Install()
{
    ArmourRenderHook::Install();
    BreathRenderHook::Install();
    HealthRenderHook::Install();
    MoneyRenderHook::Install();
    WantedRenderHook::Install();
}