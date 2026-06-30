#include "FunctionPatch.h"

FunctionPatch::FunctionPatch(
    uintptr_t address,
    const void* patchBytes,
    size_t patchSize)
    :
    mAddress(address),
    mSize(patchSize)
{
    memcpy(
        mOriginal.data(),
        reinterpret_cast<void*>(address),
        patchSize);

    memcpy(
        mPatch.data(),
        patchBytes,
        patchSize);
}

void FunctionPatch::Enable()
{
    if (mEnabled)
        return;

    patch::SetRaw(
        reinterpret_cast<void*>(mAddress),
        mPatch.data(),
        mSize);

    mEnabled = true;
}

void FunctionPatch::Disable()
{
    if (!mEnabled)
        return;

    patch::SetRaw(
        reinterpret_cast<void*>(mAddress),
        mOriginal.data(),
        mSize);

    mEnabled = false;
}