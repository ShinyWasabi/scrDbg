#pragma once
#include <Windows.h>
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <stack>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

// clang-format off
#include "scrDbg.h"
#include "util/Joaat.hpp"
#include "resources/NativeDB.hpp"
#include "resources/x86Injector.hpp"
#include "game/Game.hpp"
// clang-format on

using namespace scrDbg;

namespace scrDbgLib
{
    extern HINSTANCE g_DllInstance;
    extern HANDLE g_MainThread;
}