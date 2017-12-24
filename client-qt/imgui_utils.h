#ifndef IMGUI_UTILS_H
#define IMGUI_UTILS_H

#include <imgui.h>
#include <cmath>


inline ImVec2 operator + (const ImVec2& a, const ImVec2& b)
{
    ImVec2 r = a;
    r.x += b.x;
    r.y += b.y;
    return r;
}

inline ImVec2 operator - (const ImVec2& a, const ImVec2& b)
{
    ImVec2 r = a;
    r.x -= b.x;
    r.y -= b.y;
    return r;
}

inline ImVec2 operator * (const ImVec2& a, const ImVec2& b)
{
    ImVec2 r = a;
    r.x *= b.x;
    r.y *= b.y;
    return r;
}

template<typename SCALAR>
ImVec2 operator * (const ImVec2& a, SCALAR b)
{
    ImVec2 r = a;
    r.x *= (float)b;
    r.y *= (float)b;
    return r;
}

float length(ImVec2 in_vec)
{
    return std::sqrt(in_vec.x * in_vec.x + in_vec.y * in_vec.y);
}

ImVec2 normalized(ImVec2 in_vec2)
{
    float norm = 1.0f / length(in_vec2);
    in_vec2.x *= norm;
    in_vec2.y *= norm;
    return in_vec2;
}

void ImGuiBeginWindow(ImVec2 in_pos, ImVec2 in_size, const char* in_title)
{
    bool opened = true;
    ImGui::SetNextWindowPos(in_pos, ImGuiCond_Always);
    ImGui::Begin(in_title, &opened, in_size, 0.5f, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar);
}



#endif // IMGUI_UTILS_H
