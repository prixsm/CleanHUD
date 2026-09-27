#pragma once

class WantedRenderHook
{
public:
    static void Install();
    static void Remove();

    static bool IsRendering();
};