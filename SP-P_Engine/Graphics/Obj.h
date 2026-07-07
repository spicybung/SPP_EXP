#pragma once

#include "SP-P_Engine/Parsers/IDE.h"
#include "SP-P_Engine/Parsers/IMG.h"
#include "SP-P_Engine/Parsers/Texture.h"

struct OBJ
{
	IMG* IMG_model_ptr;//указатель на обезличенный файл модели из IMG
	IMG* IMG_texture_ptr[32];//32 указателя на обезличенные файлы текстуры из IMG
	ClumpMDL* MDL_clump;//указатель на структуру модели, содержащей указатели на все члены модели (расфасованный IMG_model_ptr)
	//deprecated:
	//ClumpTEX* TEX_clump;//указатель на структуру текстуры, содержащей указатели на все члены текстуры (расфасованный IMG_texture_ptr)
	bool flg_loaded = true;//флаг уникальности текстуры на текстурах-первенцах
	IDE* IDE_struct;//соответствующая строка объекта отрисовки в IDE (хранит ID, model_name, texture_name) 
	IPL* IPL_struct;//соответствующая строка объекта отрисовки в IPL (хранит ID, model_name, x, y, z)
};
extern OBJ* drawlist;

extern ClumpTEX* TEX_clump;

void MasteringOBJ();

