#pragma once

#include <array>
#include <cstdint>

class FunctionPatch
{
public:
    FunctionPatch(uintptr_t address,
        const void* patchBytes,
        size_t patchSize);

    void Enable();
    void Disable();

    bool IsEnabled() const;

private:
    uintptr_t mAddress;

    std::array<uint8_t, 16> mOriginal{};
    std::array<uint8_t, 16> mPatch{};

    size_t mSize = 0;
    bool mEnabled = false;
};