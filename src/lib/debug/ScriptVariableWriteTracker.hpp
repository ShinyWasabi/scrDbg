#pragma once

namespace scrDbgLib
{
    class ScriptVariableWriteTracker
    {
    public:
        static void Begin(std::uint32_t index, bool isGlobal);
        static void AddFieldOffset(std::uint32_t offset);
        static void AddArrayIndex(std::uint32_t index, std::uint32_t size);
        static void Finalize(std::uint32_t hash, const char* name, std::uint32_t pc, std::int32_t value, bool isStruct = false);
        static void Break();
        static bool ShouldBreak(std::uint8_t op);

    private:
        static inline bool m_IsActive;
        static inline bool m_IsGlobal;
        static inline std::uint32_t m_VariableIndex;
        static inline char m_PathBuf[256];
        static inline std::size_t m_PathLen;
    };
}