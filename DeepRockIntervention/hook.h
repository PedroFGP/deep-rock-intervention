#pragma once
#include "pch.h"
#include <iomanip>
#include <sstream>

extern void** Address;
extern struct UFont* Font;
extern struct UFont* TitleFont;

bool CheatInit();
void CheatRemove();
