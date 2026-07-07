#include "SP-P_Engine/Misc/Keys.h"
#include "SP-P_Engine/Graphics/Draw.h"

bool keys[256];
float thres_hold = 22.0f; // мертвая зона джоя

// Переменная для защиты от "залипания" кнопки
bool last_start_state = true;
SceCtrlData pad_data;

void OnJoystick()
{
    sceCtrlPeekBufferPositive(&pad_data, 1);

    // --- ОТКЛЮЧЕНИЕ / ВКЛЮЧЕНИЕ КУЛЛИНГА (Кнопка START) ---
    bool current_start_state = (pad_data.Buttons & PSP_CTRL_START) != 0;

    // Переключаем только в момент нажатия (когда в прошлом кадре кнопка не была нажата)
    if (current_start_state && !last_start_state) {
        g_enable_culling = !g_enable_culling;
    }
    last_start_state = current_start_state; // Запоминаем состояние для следующего кадра
    // -------------------------------------------------------

    // --- АНАЛОГОВЫЙ СТИК (Вращение камеры) ---
    if ((float)pad_data.Lx > (128.0f + thres_hold)) {
        if (m_camera_rotation_x >= 355.0f) m_camera_rotation_x = 0.0f;
        else m_camera_rotation_x += 5.0f;
    }
    if ((float)pad_data.Lx < (128.0f - thres_hold)) {
        if (m_camera_rotation_x <= 0.0f) m_camera_rotation_x = 355.0f;
        else m_camera_rotation_x -= 5.0f;
    }

    if ((float)pad_data.Ly > (128.0f + thres_hold)) m_camera_rotation_y += 5.0f;
    if ((float)pad_data.Ly < (128.0f - thres_hold)) m_camera_rotation_y -= 5.0f;

    // Ограничение по вертикали
    if (m_camera_rotation_y < -166.0f) m_camera_rotation_y = -170.0f;
    if (m_camera_rotation_y > -34.0f) m_camera_rotation_y = -30.0f;

    // --- КНОПКИ ДЕЙСТВИЙ (Движение) ---
    if (pad_data.Buttons & PSP_CTRL_CROSS) {
        go = true;
        move_speed = 2.0f;
    } else if (pad_data.Buttons & PSP_CTRL_TRIANGLE) {
        go = true;
        move_speed = -2.0f;
    } else {
        go = false;
    }

    // --- D-PAD (Плоскость отсечения и fov) ---
    if (pad_data.Buttons & PSP_CTRL_UP) front_plane += 0.02f;
    if (pad_data.Buttons & PSP_CTRL_DOWN) front_plane -= 0.02f;
    if (pad_data.Buttons & PSP_CTRL_LEFT) --fov;
    if (pad_data.Buttons & PSP_CTRL_RIGHT) ++fov;

    // --- ШИФТЫ (Дальность прорисовки) ---
    // Нажимаете L - уменьшается, R - увеличивается
    if (pad_data.Buttons & PSP_CTRL_LTRIGGER) draw_dist -= 30;
    if (pad_data.Buttons & PSP_CTRL_RTRIGGER) draw_dist += 30;

    // Защита от отрицательной дальности
    if (draw_dist < 100) draw_dist = 100;
}