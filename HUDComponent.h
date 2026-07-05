#pragma once

#include "HUDElement.h"
#include "VisibilityTimer.h"

class HUDComponent
{
public:

    HUDComponent();
    HUDComponent(std::initializer_list<HUDPatch::Call>);

    bool IsVisible() const;

    void ShowFor(uint32_t);

    void Hide();

    void Reset();

    void Show();

    void Update();

private:

    HUDElement mElement;
    VisibilityTimer mTimer;
};