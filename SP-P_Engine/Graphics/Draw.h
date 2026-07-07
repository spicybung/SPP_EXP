#pragma once

#include "SP-P_Engine/Misc/Keys.h" // Включает зависимости управления и Camera.h

extern int g_draw_calls_this_frame; // логирование кол-ва вызовов отрисовки для текущего кадра
extern bool g_enable_culling; // флаг вкл/выкл софтварного куллинга+отсечения по дальности
extern unsigned int list[262144]; // память под команды GE

void PrecalculateMatrices();
void InitRenderStates();
void OnDraw();
