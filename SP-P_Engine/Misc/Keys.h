#pragma once

#include "SP-P_Engine/Graphics/Camera.h"
#include <pspctrl.h>

// Кнопки
extern bool keys[256];
extern float thres_hold; // мертвая зона джоя

void OnJoystick();