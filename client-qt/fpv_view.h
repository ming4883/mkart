#ifndef FPV_VIEW_H
#define FPV_VIEW_H

#include "imgui_utils.h"
#include "fpv_service.h"

class FpvView
{
public:
    int m_remote_ip[4];
    int m_remote_port;

    FpvView()
    {
        m_remote_ip[0] = 192;
        m_remote_ip[1] = 168;
        m_remote_ip[2] = 11;
        m_remote_ip[3] = 87;

        m_remote_port = 8080;
    }

    void on_imgui()
    {
        ImGuiBeginWindow(ImVec2(50, 50), ImVec2(1400,900), "FPVView");

        ImGui::Text("FPV");
        // Status

        switch(FpvService::get_state())
        {
            case Tcp::state_disconnected:
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Disconnected");
                break;
            case Tcp::state_connecting:
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "Connecting");
                break;
            case Tcp::state_connected:
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Connected");
                break;
        }

        // Remote Address
        ImGui::SliderInt4("##remote_ip", m_remote_ip, 0, 255);
        ImGui::SameLine();
        ImGui::Text("：");
        ImGui::SameLine();

        ImGui::PushItemWidth(90);
        ImGui::InputInt("##remote_port", &m_remote_port, 1, 10, 0);
        ImGui::PopItemWidth();

        ImGui::Separator();

        // Actions
        if (ImGui::Button("Connect", ImVec2(100, 50)))
        {
            unsigned char ip[4] = {
                (unsigned char)m_remote_ip[0],
                (unsigned char)m_remote_ip[1],
                (unsigned char)m_remote_ip[2],
                (unsigned char)m_remote_ip[3]
            };
            FpvService::connect(ip, m_remote_port);
        }

        ImGui::SameLine();

        if (ImGui::Button("Disconnect", ImVec2(100, 50)))
        {
            FpvService::disconnect();
        }

        ImGui::SameLine();

        bool capture = FpvService::is_capturing();
        if (capture)
        {
            if (ImGui::Button("Stop Capture", ImVec2(100, 50)))
                FpvService::set_capturing(false);
        }
        else
        {
            if (ImGui::Button("Start Capture", ImVec2(100, 50)))
                FpvService::set_capturing(true);
        }

        ImGui::SameLine();

        if (ImGui::Button("Remote Exit", ImVec2(100, 50)))
        {
            FpvService::remote_exit();
        }

        ImGui::Separator();

        // Debug
        ImGui::Text("Debug");

        unsigned int tex_id = FpvService::get_texture_id();

        if (0 != tex_id)
        {
            unsigned int w = FpvService::get_texture_width();
            unsigned int h = FpvService::get_texture_height();
            float recv_time = FpvService::get_avg_recv_time();

            ImGui::Text("tex id = %d, %d x %d, %.1f ms", tex_id, w, h, recv_time);
            ImGui::Image(reinterpret_cast<ImTextureID>(tex_id), ImVec2(w * 2, h * 2));
        }

        ImGui::End();
    }
};

#endif // FPV_VIEW_H
