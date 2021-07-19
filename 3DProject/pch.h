#pragma once

#include <Windows.h>
#include <string>
#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <array>
#include <fcntl.h>
#include <io.h>
#include "BufferHelpers.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "SimpleMath.h"
#include "stb_image.h"
#include <comdef.h>
#include <d3d11.h>
#include <unordered_map>

namespace sm = DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;