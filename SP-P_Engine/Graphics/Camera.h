#pragma once

#define GU_VIEW			(1) // что-бы не тащить pspgu.h

extern int fov; // fov камеры
extern float front_plane; // передняя плоскость отсечения
extern int draw_dist; // дальность прорисовки

// Позиция и повороты камеры:
extern float m_camera_position_x, m_camera_position_y, m_camera_position_z, move_speed;
extern float m_camera_rotation_y, m_camera_rotation_x;
extern bool go; // флаг движения

void Camera();