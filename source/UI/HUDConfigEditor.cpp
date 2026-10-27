#include "HUDConfigEditor.h"

#include <cstdint>

#include <windows.h>
#include <commctrl.h>
#include <d3d9.h>

#include <plugin.h>
#include <RenderWare.h>
#include <CPad.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>

#include "../Core/HUDSettings.h"

#pragma comment(lib, "comctl32.lib")

// Forward declaration intentionally omitted by Dear ImGui's backend header.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
    constexpr UINT_PTR SubclassId = 0x43485544; // 'CHUD'

    bool gOpen = false;
    bool gInitialized = false;
    bool gWasF10Down = false;
    bool gDirty = false;
    bool gHasSaveResult = false;
    bool gLastSaveSucceeded = false;

    HWND gWindow = nullptr;
    IDirect3DDevice9* gDevice = nullptr; // borrowed RenderWare device
    CPad* gInputPad = nullptr;
    unsigned short gPreviousDisableControls = 0;

    bool IsMouseMessage(UINT message)
    {
        return (message >= WM_MOUSEFIRST &&
            message <= WM_MOUSELAST) ||
            message == WM_MOUSEWHEEL ||
            message == WM_MOUSEHWHEEL;
    }

    bool IsKeyboardMessage(UINT message)
    {
        return (message >= WM_KEYFIRST &&
            message <= WM_KEYLAST) ||
            message == WM_CHAR ||
            message == WM_SYSCHAR;
    }

    LRESULT CALLBACK EditorSubclassProc(
        HWND hwnd, UINT message, WPARAM wParam,
        LPARAM lParam, UINT_PTR, DWORD_PTR)
    {
        if (gOpen && ImGui::GetCurrentContext())
        {
            if (ImGui_ImplWin32_WndProcHandler(
                hwnd, message, wParam, lParam))
            {
                return 1;
            }

            const ImGuiIO& io = ImGui::GetIO();

            if ((IsMouseMessage(message) && io.WantCaptureMouse) ||
                (IsKeyboardMessage(message) && io.WantCaptureKeyboard))
            {
                return 1;
            }
        }

        return DefSubclassProc(hwnd, message, wParam, lParam);
    }

    void ReleaseInput()
    {
        if (gInputPad)
        {
            gInputPad->DisablePlayerControls =
                gPreviousDisableControls;
            gInputPad = nullptr;
        }

        if (gWindow)
            RemoveWindowSubclass(
                gWindow, EditorSubclassProc, SubclassId);
    }

    bool Initialize()
    {
        if (gInitialized)
            return true;

        auto* device = reinterpret_cast<IDirect3DDevice9*>(
            RwD3D9GetCurrentD3DDevice());

        if (!device)
            return false;

        D3DDEVICE_CREATION_PARAMETERS parameters = {};
        if (FAILED(device->GetCreationParameters(&parameters)) ||
            !parameters.hFocusWindow)
        {
            return false;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr; // No extra imgui.ini file.
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        io.MouseDrawCursor = true;

        ImGui::StyleColorsDark();

        if (!ImGui_ImplWin32_Init(parameters.hFocusWindow))
        {
            ImGui::DestroyContext();
            return false;
        }

        if (!ImGui_ImplDX9_Init(device))
        {
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            return false;
        }

        gDevice = device;
        gWindow = parameters.hFocusWindow;
        gInitialized = true;
        return true;
    }

    void Close()
    {
        gOpen = false;
        ReleaseInput();
    }

    void Open()
    {
        if (!Initialize() || !gWindow)
            return;

        if (!SetWindowSubclass(
            gWindow, EditorSubclassProc, SubclassId, 0))
        {
            return;
        }

        gOpen = true;
        ReleaseCapture();
        ClipCursor(nullptr);
    }

    void Shutdown()
    {
        Close();

        if (!gInitialized)
            return;

        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        gDevice = nullptr;
        gWindow = nullptr;
        gInitialized = false;
    }

    void OnDeviceLost()
    {
        if (gInitialized)
            ImGui_ImplDX9_InvalidateDeviceObjects();
    }

    void OnDeviceReset()
    {
        if (gInitialized)
            ImGui_ImplDX9_CreateDeviceObjects();
    }

    void CheckHotkey()
    {
        const bool pressed = (GetAsyncKeyState(VK_F10) & 0x8000) != 0;

        if (pressed && !gWasF10Down)
        {
            if (gOpen)
                Close();
            else
                Open();
        }

        gWasF10Down = pressed;
    }

    void CheckBox(const char* label, bool& value)
    {
        if (ImGui::Checkbox(label, &value))
            gDirty = true;
    }

    void EditMillis(
        const char* label, std::uint32_t& value,
        int maximum)
    {
        int amount = static_cast<int>(value);

        if (ImGui::DragInt(label, &amount, 1.0f, 0, maximum,
            "%d ms", ImGuiSliderFlags_AlwaysClamp))
        {
            value = static_cast<std::uint32_t>(amount);
            gDirty = true;
        }
    }

    void EditSpacing(const char* label, float& value)
    {
        if (ImGui::DragFloat(
            label, &value, 0.25f, -200.0f, 500.0f,
            "%.2f", ImGuiSliderFlags_AlwaysClamp))
        {
            gDirty = true;
        }
    }

    void DrawEditor()
    {
        HUDSettings& settings = HUDSettings::Get();

        ImGui::SetNextWindowSize(ImVec2(440, 650),
            ImGuiCond_FirstUseEver);

        bool keepOpen = gOpen;

        if (ImGui::Begin("CleanHUD - Live Editor",
            &keepOpen, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::TextUnformatted(
                "Changes apply immediately. Save to keep them.");
            ImGui::Separator();

            int mode = settings.Mode == HUDMode::Stack ? 0 : 1;
            if (ImGui::Combo("HUD mode", &mode,
                "Stack\0Minimal\0"))
            {
                settings.Mode =
                    mode == 0 ? HUDMode::Stack : HUDMode::Minimal;
                gDirty = true;
            }

            CheckBox("Always show clock", settings.AlwaysShowClock);

            if (ImGui::CollapsingHeader("Visibility",
                ImGuiTreeNodeFlags_DefaultOpen))
            {
                EditMillis("Fade in", settings.FadeInMs, 2000);
                EditMillis("Fade out", settings.FadeOutMs, 2000);
                EditMillis("Show duration", settings.VisibilityMs, 60000);
            }

            if (ImGui::CollapsingHeader("Animation"))
            {
                CheckBox("Slide stack", settings.SlideStack);
                ImGui::TextDisabled(
                    "Sliding applies only in Stack mode.");
            }

            if (ImGui::CollapsingHeader("Elements"))
            {
                CheckBox("Clock", settings.Clock);
                CheckBox("Money", settings.Money);
                CheckBox("Wanted", settings.Wanted);
                CheckBox("Health", settings.Health);
                CheckBox("Armour", settings.Armour);
                CheckBox("Breath", settings.Breath);
                CheckBox("Weapon", settings.Weapon);
                CheckBox("Ammo", settings.Ammo);
                ImGui::TextDisabled(
                    "Unchecked elements are drawn by the game.");
            }

            if (ImGui::CollapsingHeader("Weapon / Ammo"))
            {
                CheckBox("Show on foot aim", settings.ShowOnFootAim);
                CheckBox("Show on throwable aim",
                    settings.ShowOnThrowableAim);
                CheckBox("Show on vehicle look",
                    settings.ShowOnVehicleLook);
            }

            if (ImGui::CollapsingHeader("Advanced spacing",
                ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::TextDisabled(
                    "Stack mode only. Drag or Ctrl+click to type.");

                ImGui::SeparatorText("Clock");
                EditSpacing("Clock to Money",
                    settings.ClockToMoneySpacing);
                EditSpacing("Clock to Wanted",
                    settings.ClockToWantedSpacing);
                EditSpacing("Clock to Status Bar",
                    settings.ClockToStatusBarSpacing);
                EditSpacing("Clock to Weapon",
                    settings.ClockToWeaponSpacing);

                ImGui::SeparatorText("Money");
                EditSpacing("Money to Wanted",
                    settings.MoneyToWantedSpacing);
                EditSpacing("Money to Status Bar",
                    settings.MoneyToStatusBarSpacing);
                EditSpacing("Money to Weapon",
                    settings.MoneyToWeaponSpacing);

                ImGui::SeparatorText("Wanted");
                EditSpacing("Wanted to Status Bar",
                    settings.WantedToStatusBarSpacing);
                EditSpacing("Wanted to Weapon",
                    settings.WantedToWeaponSpacing);

                ImGui::SeparatorText("Status Bars");
                EditSpacing("Status Bar to Weapon",
                    settings.StatusBarToWeaponSpacing);

                ImGui::SeparatorText("Individual elements");
                EditSpacing("Weapon spacing",
                    settings.WeaponSpacing);
                EditSpacing("Ammo spacing",
                    settings.AmmoSpacing);
                EditSpacing("Armour spacing",
                    settings.ArmourSpacing);
                EditSpacing("Breath spacing",
                    settings.BreathSpacing);
                EditSpacing("Health spacing",
                    settings.HealthSpacing);
            }

            ImGui::Separator();

            if (ImGui::Button("Save to CleanHUD.ini"))
            {
                gLastSaveSucceeded = settings.Save();
                gHasSaveResult = true;

                if (gLastSaveSucceeded)
                    gDirty = false;
            }

            ImGui::SameLine();

            if (ImGui::Button("Reload from disk"))
            {
                settings.Load();
                gDirty = false;
                gHasSaveResult = false;
            }

            ImGui::SameLine();

            if (ImGui::Button("Close"))
                keepOpen = false;

            if (gHasSaveResult)
            {
                ImGui::TextUnformatted(gLastSaveSucceeded
                    ? "Saved successfully."
                    : "Save failed; check file permissions.");
            }
            else if (gDirty)
            {
                ImGui::TextUnformatted(
                    "Unsaved changes are active in-game.");
            }
        }

        ImGui::End();

        if (!keepOpen)
            Close();
    }

    void Draw()
    {
        CheckHotkey();

        if (!gOpen || !gInitialized)
            return;

        // GTA may recapture the pointer on successive frames.
        // The editor uses the software ImGui cursor.
        ClipCursor(nullptr);

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        DrawEditor();

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }
}

namespace HUDConfigEditor
{
    void UpdateInput()
    {
        if (!gOpen)
        {
            if (gInputPad)
            {
                gInputPad->DisablePlayerControls =
                    gPreviousDisableControls;
                gInputPad = nullptr;
            }

            return;
        }

        CPad* pad = CPad::GetPad(0);

        if (!pad)
            return;

        if (gInputPad != pad)
        {
            if (gInputPad)
            {
                gInputPad->DisablePlayerControls =
                    gPreviousDisableControls;
            }

            gInputPad = pad;
            gPreviousDisableControls =
                pad->DisablePlayerControls;
        }

        // Temporarily prevent shooting/moving while interacting
        // with the editor. Restore the game's previous value
        // immediately on closing.
        pad->DisablePlayerControls = 1;
    }

    void Install()
    {
        using namespace plugin;

        Events::drawingEvent += []()
        {
            Draw();
        };

        Events::d3dLostEvent += []()
        {
            OnDeviceLost();
        };

        Events::d3dResetEvent += []()
        {
            OnDeviceReset();
        };

        Events::shutdownRwEvent += []()
        {
            Shutdown();
        };
    }
}
