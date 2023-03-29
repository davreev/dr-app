#include <dr/app/shim/imgui.hpp>

namespace dr
{

void ImGuiStyles::set_default(ImGuiStyle& style)
{
    style.WindowRounding = 5.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowTitleAlign = {0.5f, 0.5f};
    
    style.PopupRounding = 5.0f;
    style.PopupBorderSize = 1.0f;

    style.FrameRounding = 3.0f;
    style.FrameBorderSize = 0.0f;

    style.GrabRounding = 2.0f;

    style.IndentSpacing = 10.0f;

    ImGuiColors::set_low_contrast_dark(style.Colors);
}

void ImGuiColors::set_low_contrast_dark(ImVec4* const colors)
{
    colors[ImGuiCol_Text] = {1.0f, 1.0f, 1.0f, 0.7f};
    colors[ImGuiCol_TextDisabled] = {1.0f, 1.0f, 1.0f, 0.3f};

    colors[ImGuiCol_WindowBg] = {0.2f, 0.2f, 0.2f, 0.95f};
    colors[ImGuiCol_ChildBg] = {0.2f, 0.2f, 0.2f, 0.95f};
    colors[ImGuiCol_PopupBg] = {0.2f, 0.2f, 0.2f, 0.95f};

    colors[ImGuiCol_Border] = {0.3f, 0.3f, 0.3f, 1.0f};
    colors[ImGuiCol_BorderShadow] = {0.0f, 0.0f, 0.0f, 0.0f};

    colors[ImGuiCol_FrameBg] = {0.15f, 0.15f, 0.15f, 0.8f};
    colors[ImGuiCol_FrameBgHovered] = {0.15f, 0.15f, 0.15f, 0.8f};
    colors[ImGuiCol_FrameBgActive] = {0.15f, 0.15f, 0.15f, 0.8f};

    colors[ImGuiCol_TitleBg] = {0.3f, 0.3f, 0.3f, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = {0.3f, 0.3f, 0.3f, 1.0f};
    colors[ImGuiCol_TitleBgActive] = {0.3f, 0.3f, 0.3f, 1.0f};

    colors[ImGuiCol_MenuBarBg] = {0.3f, 0.3f, 0.3f, 1.0f};
    
    colors[ImGuiCol_ScrollbarBg] = {0.15f, 0.15f, 0.15f, 0.8f};
    colors[ImGuiCol_ScrollbarGrab] = {0.3f, 0.3f, 0.3f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabHovered] = {0.4f, 0.4f, 0.4f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabActive] = {0.45f, 0.45f, 0.45f, 1.0f};

    colors[ImGuiCol_CheckMark] = {1.0f, 1.0f, 1.0f, 0.7f};

    colors[ImGuiCol_SliderGrab] = {0.3f, 0.3f, 0.3f, 1.0f};
    colors[ImGuiCol_SliderGrabActive] = {0.45f, 0.45f, 0.45f, 1.0f};

    colors[ImGuiCol_Button] = {0.3f, 0.3f, 0.3f, 1.0f};
    colors[ImGuiCol_ButtonHovered] = {0.4f, 0.4f, 0.4f, 1.0f};
    colors[ImGuiCol_ButtonActive] = {0.45f, 0.45f, 0.45f, 1.0f};
    
    colors[ImGuiCol_Header] = {0.3f, 0.3f, 0.3f, 1.0f};
    colors[ImGuiCol_HeaderHovered] = {0.4f, 0.4f, 0.4f, 1.0f};
    colors[ImGuiCol_HeaderActive] = {0.45f, 0.45f, 0.45f, 1.0f};
    
    colors[ImGuiCol_Separator] = {0.3f, 0.3f, 0.3f, 1.0f};
    colors[ImGuiCol_SeparatorHovered] = {0.4f, 0.4f, 0.4f, 1.0f};
    colors[ImGuiCol_SeparatorActive] = {0.45f, 0.45f, 0.45f, 1.0f};
    
    colors[ImGuiCol_ResizeGrip] = {0.3f, 0.3f, 0.3f, 1.0f};
    colors[ImGuiCol_ResizeGripHovered] = {0.4f, 0.4f, 0.4f, 1.0f};
    colors[ImGuiCol_ResizeGripActive] = {0.45f, 0.45f, 0.45f, 1.0f};

    colors[ImGuiCol_Tab] = {0.3f, 0.3f, 0.3f, 1.0f};
    colors[ImGuiCol_TabHovered] = {0.4f, 0.4f, 0.4f, 1.0f};
    colors[ImGuiCol_TabActive] = {0.4f, 0.4f, 0.4f, 1.0f};
    colors[ImGuiCol_TabUnfocused] = {0.3f, 0.3f, 0.3f, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = {0.4f, 0.4f, 0.4f, 1.0f};

    colors[ImGuiCol_PlotLines] = {1.0f, 1.0f, 1.0f, 0.3f};
    colors[ImGuiCol_PlotLinesHovered] = {1.0f, 1.0f, 1.0f, 0.7f};

    colors[ImGuiCol_PlotHistogram] = {1.0f, 1.0f, 1.0f, 0.3f};
    colors[ImGuiCol_PlotHistogramHovered] = {1.0f, 1.0f, 1.0f, 0.7f};

    colors[ImGuiCol_TextSelectedBg] = {0.4f, 0.4f, 0.4f, 1.0f};

    colors[ImGuiCol_DragDropTarget] = {0.4f, 0.4f, 0.4f, 1.0f};

    colors[ImGuiCol_NavHighlight] = {1.0f, 1.0f, 1.0f, 0.7f};
    colors[ImGuiCol_NavWindowingHighlight] = {1.0f, 1.0f, 1.0f, 0.7f};
    colors[ImGuiCol_NavWindowingDimBg] = {0.15f, 0.15f, 0.15f, 0.3f};

    colors[ImGuiCol_ModalWindowDimBg] = {0.15f, 0.15f, 0.15f, 0.3f};
}

} // namespace dr