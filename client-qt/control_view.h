#ifndef CONTROL_VIEW_H
#define CONTROL_VIEW_H

#include "timing.h"
#include "remote_service.h"
#include "sensor_service.h"
#include "imgui_utils.h"

#include <glm/glm.hpp>

class ControlView
{
    ImVec2 m_motor_speed;
    TimePt m_last_time;
    bool m_mouse_draging;

public:

    ControlView() : m_motor_speed(0, 0), m_last_time(Clock::now()), m_mouse_draging(false)
    {
    }

    void on_imgui()
    {
        float delta_t = delta_time_in_ms(m_last_time) / 1000.0f;

        ImGuiBeginWindow(ImVec2(10, 50), ImVec2(450,550), "Control");

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        {
            ImGui::Text("Left-click and drag to control");

            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
            ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
            canvas_size.y -= 130;
            if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;           // minimum size
            if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
            ImVec2 canvas_center = canvas_pos + canvas_size * 0.5f;

            float pad_size = 32.0f;

            ImGui::InvisibleButton("canvas", canvas_size);

            // draw background
            draw_list->AddRectFilledMultiColor(canvas_pos, canvas_pos + canvas_size, ImColor(50,50,50), ImColor(50,50,60), ImColor(60,60,70), ImColor(50,50,60));
            draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImColor(255,255,255));

            draw_list->AddLine(canvas_center + canvas_size * ImVec2(0,-0.5f), canvas_center + canvas_size * ImVec2(0, 0.5f), IM_COL32(255,255,255,127), 1.0f);
            draw_list->AddLine(canvas_center + canvas_size * ImVec2(-0.5f, 0), canvas_center + canvas_size * ImVec2(0.5f, 0), IM_COL32(255,255,255,127), 1.0f);

            // draw range circles
            ImVec2 rect_size (pad_size, pad_size);

            draw_list->AddRect(canvas_center - rect_size * 1, canvas_center + rect_size * 1, ImColor(255, 255, 63, 31));
            draw_list->AddRect(canvas_center - rect_size * 2, canvas_center + rect_size * 2, ImColor(255, 255, 63, 63));
            draw_list->AddRect(canvas_center - rect_size * 3, canvas_center + rect_size * 3, ImColor(255, 255, 63, 127));
            draw_list->AddRect(canvas_center - rect_size * 4, canvas_center + rect_size * 4, ImColor(255, 255, 63, 255));

            // draw speed indicator
            draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x+canvas_size.x, canvas_pos.y+canvas_size.y));

            if (ImGui::IsItemHovered())
            {
                if (ImGui::IsMouseClicked(0))
                {
                    m_mouse_draging = true;
                }
            }

            if (m_mouse_draging)
            {
                ImVec2 mouse_pos = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
                draw_list->AddLine(canvas_pos + mouse_pos, canvas_center, IM_COL32(255,255,0,255), 2.0f);

                m_motor_speed = (mouse_pos - canvas_size * 0.5f) * (1.0f / pad_size);
            }
            else
            {
                m_motor_speed = m_motor_speed + (ImVec2(0, 0) - m_motor_speed) * (2.0f * delta_t);
            }

            if (ImGui::IsMouseReleased(0))
            {
                m_mouse_draging = false;
            }

            draw_list->AddCircleFilled(canvas_center + m_motor_speed * pad_size, 3.0f, IM_COL32(255,255,0,127));

            draw_list->PopClipRect();

            int16_t l_speed, r_speed;
            get_motor_speed(l_speed, r_speed);

            ImGui::Text("[%.2f, %.2f] =>[%d, %d]", m_motor_speed.x, m_motor_speed.y, l_speed, r_speed);

            SensorService::Readings readings;
            SensorService::get_readings(readings);

            ImGui::Separator();
            ImGui::Text("Rotation : %.1f, %.1f %.1f", readings.rot_x_deg, readings.rot_y_deg, readings.rot_z_deg);
            ImGui::Text("Gyroscope: %.1f, %.1f %.1f", readings.gyro_x_deg, readings.gyro_y_deg, readings.gyro_z_deg);

        }

        ImGui::End();
    }

    void get_motor_speed(int16_t& out_l_speed, int16_t& out_r_speed)
    {
        /*  ---> x
            | (1, 0.5)    (1, 1)      (0.5, 1)
            |
            | (1, 0  )    (0, 0)      (0,   1)
            y
              (-1,-0.5)    (-1, -1)      (-0.5, -1)
        */
        glm::vec2 p(m_motor_speed.x, m_motor_speed.y);
        p = glm::clamp(p * 0.25f, glm::vec2(-1, -1), glm::vec2(1, 1));

        glm::vec2 d0, d1, d2, d3;
        float s, t;

        if (p.x > 0)
        {
            if (p.y > 0)
            {
                s = p.x; // right
                t = p.y; // down

                d0 = glm::vec2( 0, 0);
                d1 = glm::vec2( 0, 1);
                d2 = glm::vec2(-1,-1);
                d3 = glm::vec2(-0.25f, 1);
            }
            else
            {
                s = p.x; // right
                t =-p.y; // up

                d0 = glm::vec2( 0, 0);
                d1 = glm::vec2( 0, 1);
                d2 = glm::vec2( 1, 1);
                d3 = glm::vec2(0.25f, 1);
            }
        }
        else
        {
            if (p.y > 0)
            {
                s =-p.x; // left
                t = p.y; // down

                d0 = glm::vec2( 0, 0);
                d1 = glm::vec2( 1, 0);
                d2 = glm::vec2(-1,-1);
                d3 = glm::vec2(-1,-0.25f);
            }
            else
            {
                s =-p.x; // left
                t =-p.y; // up

                d0 = glm::vec2( 0, 0);
                d1 = glm::vec2( 1, 0);
                d2 = glm::vec2( 1, 1);
                d3 = glm::vec2( 1,0.25f);
            }
        }

        glm::vec2 d = glm::mix(
            glm::mix(d0, d1, s),
            glm::mix(d2, d3, s),
            t);

        d = d * 1024.0f;

        if (p.y > 0)
        {
            // backward, move the same side
            out_l_speed = (int)d.x;
            out_r_speed = (int)d.y;
        }
        else
        {
            // forward, move the opposite side
            out_r_speed = (int)d.x;
            out_l_speed = (int)d.y;
        }
    }
};

#endif // CONTROL_VIEW_H
