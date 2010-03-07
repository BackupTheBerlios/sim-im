#ifndef IDLEUI_H
#define IDLEUI_H
#pragma once
#include <windows.h>

BOOL IdleUIInit();
void IdleUITerm();
DWORD IdleUIGetLastInputTime();

#endif
