#ifndef CONNECTION_VIEW_H
#define CONNECTION_VIEW_H

#include "imgui_utils.h"
#include "remote_service.h"
#include <kart_commands.h>

class ConnectionView
{
public:
    int m_remote_ip[4];
    int m_remote_port;

    ConnectionView()
    {
        m_remote_ip[0] = 192;
        m_remote_ip[1] = 168;
        m_remote_ip[2] = 11;
        m_remote_ip[3] = 9;

        m_remote_port = 8080;

    }

    void on_imgui_ip(const char* in_label, unsigned char* inout_value, const char* in_tail)
    {
        int val = (int)*inout_value;
        ImGui::DragInt(in_label, &val, 1, 0, 255);
        ImGui::SameLine();
        ImGui::Text(in_tail);
        ImGui::SameLine();

        *inout_value = (unsigned char)val;
    }

    void on_imgui()
    {
        ImGuiBeginWindow(ImVec2(500, 50), ImVec2(400,250), "ConnectView");

        // Status
        ImGui::Text("Motor");

        switch(RemoteService::get_state())
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
            RemoteService::connect(ip, m_remote_port);
        }

        ImGui::SameLine();

        if (ImGui::Button("Disconnect", ImVec2(100, 50)))
        {
            RemoteService::disconnect();
        }

        ImGui::Separator();

        // Debug
        ImGui::Text("Debug");
        if (ImGui::Button("Motor Fwd", ImVec2(80, 20)))
        {
            CmdMotorSetSpeed cmd(512, 512);
            RemoteService::send_cmd(cmd);
        }
        ImGui::SameLine();
        if (ImGui::Button("Motor Bck", ImVec2(80, 20)))
        {
            CmdMotorSetSpeed cmd(-512, -512);
            RemoteService::send_cmd(cmd);
        }
        ImGui::SameLine();
        if (ImGui::Button("Motor Stop", ImVec2(80, 20)))
        {
            CmdMotorSetSpeed cmd(0, 0);
            RemoteService::send_cmd(cmd);
        }

        ImGui::End();
    }
};

#endif // CONNECTION_VIEW_H
