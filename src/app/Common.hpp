#pragma once
// clang-format off
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
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

namespace scrDbgApp
{
    extern bool g_IsEnhanced;
}

#define ENHANCED_TARGET_BUILD "889.22-1.71"
#define LEGACY_TARGET_BUILD "3586.0-1.71"