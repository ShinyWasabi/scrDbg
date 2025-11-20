#pragma once
#include <Windows.h>
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

extern HINSTANCE g_DllInstance;
extern HANDLE g_MainThread;
extern bool g_IsEnhanced;