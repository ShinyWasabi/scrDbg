#pragma once
// clang-format off
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")
// clang-format on
#include <array>
#include <atomic>
#include <chrono>
#include <codecvt>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <vector>

#include "Joaat.hpp"
#include "core/Pointer.hpp"
#include "game/Game.hpp"