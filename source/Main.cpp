#include <plugin.h>
#include "../source/Core/HUDManager.h"
#include "../source/Core/HUDSettings.h"
#include "../source/PlayerHUDHook.h"
#include "../source/Render/Hooks/HUDRenderHooks.h"

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