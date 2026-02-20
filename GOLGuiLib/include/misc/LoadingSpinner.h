#ifndef __LoadingSpinner_h__
#define __LoadingSpinner_h__

#include <imgui/imgui.h>

namespace gol
{
    void LoadingSpinner(const char* label, float radius, float thickness, ImU32 color);
}

#endif