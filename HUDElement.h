#pragma once

#include <vector>
#include "HUDPatch.h"
#include "CallPatch.h"

class HUDElement
{
public:

    HUDElement(std::initializer_list<HUDPatch::Call> patches);

    void Show();
    void Hide();

    bool IsVisible() const;

private:

    std::vector<CallPatch> m_patches;

    bool m_visible;
};