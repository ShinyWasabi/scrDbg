#pragma once
#include <Windows.h>
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Joaat.hpp"
#include "game/Game.hpp"
#include "scrDbg.h"

namespace scrDbgLib
{
    extern HINSTANCE g_DllInstance;
    extern HANDLE g_MainThread;
}