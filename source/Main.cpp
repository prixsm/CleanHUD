#include <plugin.h>
#include "../HUDManager.h"
#include "../HUDSettings.h"
#include "../PlayerHUDHook.h"
#include "../HUDRenderHooks.h"

using namespace plugin;

class Plugin
{
public:

    Plugin()
    {
        HUDSettings::Get().Load();
        HUDManager::Get().RestoreVanillaElements();

        PlayerHUDHook::Install();
        HUDRenderHooks::Install();

        Events::reInitGameEvent += []()
            {
                HUDManager::Get().OnGameLoaded();
            };
        
        Events::gameProcessEvent += []()
            {
                HUDManager::Get().Update();
            };
    }
};

Plugin gPlugin;