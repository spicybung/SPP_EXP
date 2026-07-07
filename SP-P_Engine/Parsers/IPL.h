#pragma once

#include <cstdio>
#include <cstring>

//для чтения ipl
struct IPL
{
	unsigned int id;//id объекта
	char names[24];//обнулить,иначе утечки Up:обнулено
	float x_ipl;
	float y_ipl;
	float z_ipl;

	// Оригинальный кватернион (опционально, можно не хранить, если нужна только матрица)
	float qx, qy, qz, qw;
	// Готовая матрица трансформации 4x4 для OpenGL
	float rotation_matrix[16];
	// id группы инстанса
	unsigned short inst_id;
	// кол-во инстанцирований в одной группе
	unsigned short count_instances;
};
extern IPL* IPL_struct;

extern unsigned int countobj;//количество объектов в IPL
extern const unsigned int& ALPHA_START_INDEX;//с какой строки идут объекты с альфой

void MathQuatToMatrix(float qx, float qy, float qz, float qw, float* matrix, float x_ipl, float y_ipl, float z_ipl);
void ParserIPL();