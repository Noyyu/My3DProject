#pragma once
#include "pch.h"

using Microsoft::WRL::ComPtr;

bool setUpWindow(UINT width, UINT height, HWND& window, int nCmdShow, HINSTANCE hInstance);

bool setUpCMD();