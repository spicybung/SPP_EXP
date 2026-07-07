#pragma once

#include "SP-P_Engine/Parsers/Model.h"

struct ClumpTEX//структура для загрузки данных текстуры
{
	unsigned int* next_ofst;
	unsigned short	pix_form;
	unsigned short	pix_swizzle;
	unsigned short	width;
	unsigned short	height;
	unsigned int	size;
	unsigned short	colors;
	unsigned char* data;
	char* name[32];

	unsigned char* pal_data;  // Указатель на выделенную память палитры
	unsigned short	pal_form;    // Формат пикселей палитры (обычно 8888)
	unsigned short	pal_count;   // Количество цветов (16 или 256)
};

ClumpTEX *ParserTEX(unsigned char* texture, ClumpTEX* TEX_clump_arg);