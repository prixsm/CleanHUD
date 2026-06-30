#include <plugin.h>
#include "../HUDManager.h"
#include "../PlayerHUDHook.h"

using namespace plugin;

class Plugin
{
public:

    Plugin()
    {   
        PlayerHUDHook::Install();

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