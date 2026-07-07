#include "Camera.h"
#include <pspgum.h>
#include <math.h>

int fov = 90;
float front_plane = 2.68f;//2.68 приемлемая передняя плоскость отсечения
int draw_dist = 300;// Дистанция прорисовки
const float PI_OVER_180 = 3.14159265f / 180.0f; // математическая константа

//float m_camera_position_x = 0.0f, m_camera_position_y = 0.0f, m_camera_position_z = 0.0f, move_speed = 2.0f;//если ipl приводили к координатам от 0 0 0
float m_camera_position_x = 2487.0f, m_camera_position_y = -1688.0f, m_camera_position_z = 13.0f, move_speed = 2.0f;//иначе эта строка (раскоментить, верхнюю закоментить)
float m_camera_rotation_y = -90.0f, m_camera_rotation_x = 0.0f;
bool go = false;

void Camera()
{
	// Включаем режим матрицы вида (камеры)
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();

	if (go == true)
	{
		m_camera_position_x -= sinf(m_camera_rotation_x * PI_OVER_180) * move_speed * (-1.0f);
		m_camera_position_y -= cosf(m_camera_rotation_x * PI_OVER_180) * move_speed * (-1.0f);
		m_camera_position_z -= sinf((m_camera_rotation_y - 90.0f) * PI_OVER_180) * move_speed * (-1.0f);
	}
	ScePspFVector3 cam_pos = { -m_camera_position_x, -m_camera_position_y, -m_camera_position_z };


	sceGumRotateX(m_camera_rotation_y * PI_OVER_180);
	sceGumRotateZ(m_camera_rotation_x * PI_OVER_180);
	sceGumTranslate(&cam_pos);
}