#pragma once

//для чтения ide
struct IDE
{
	unsigned int id;//на стадии рабочего билда поправить на unsigned short
	char m_names[24];//имя модели
	char t_names[24];//имя соответствующей текстуры
};
extern IDE* IDE_struct;

extern unsigned int count_of_ids;

void ParserIDE();