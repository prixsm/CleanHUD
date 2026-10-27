#include <plugin.h>
#include "Core/HUDManager.h"
#include "Core/HUDSettings.h"
#include "PlayerHUDHook.h"
#include "UI/SleekConfigIntegration.h"
#include "Render/Hooks/HUDRenderHooks.h"

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
        
        // GTA SA exposes shutdownRwEvent, not shutdownEngineEvent.
        // Detach before RenderWare shutdown; the adapter will register
        // again on the game thread if the engine restarts.
        Events::shutdownRwEvent += []()
            {
                SleekConfigIntegration::Detach();
            };

        Events::gameProcessEvent += []()
            {
                SleekConfigIntegration::TryAttach();
                HUDManager::Get().Update();
            };
    }
};

Plugin gPlugin;