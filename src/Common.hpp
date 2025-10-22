#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <string_view>
#include <thread>
#include <vector>
#include <stack>
#include <unordered_set>

namespace scrDbg
{
	extern bool g_IsEnhanced;
	extern bool g_BreakpointsSupported;
}

#define ENHANCED_TARGET_BUILD "889.22-1.71"
#define LEGACY_TARGET_BUILD "3586.0-1.71"