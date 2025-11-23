#pragma once

namespace scrDbgLib
{
    class ScriptLogger
    {
    public:
        enum LogType
        {
            LOG_TYPE_NONE,
            LOG_TYPE_FRAME_TIME,
            LOG_TYPE_NATIVE_CALLS,
            LOG_TYPE_STATIC_WRITES,
            LOG_TYPE_GLOBAL_WRITES
        };

        static bool Init(const char* filename);
        static void Destroy();

        static void SetLogType(LogType type);
        static LogType GetLogType();
        static void SetScriptHash(std::uint32_t hash);
        static uint32_t GetScriptHash();

        static void Clear();
        static void Log(LogType type, std::uint32_t hash, const char* message);
        static void Logf(LogType type, std::uint32_t hash, const char* fmt, ...);

        static void LoggerThread();

    private:
        static inline std::ofstream m_File;
        static inline std::string m_FilePath;
        static inline LogType m_LogType;
        static inline std::uint32_t m_ScriptHash;
        static inline std::atomic<bool> m_Running;
        static inline std::queue<std::string> m_Queue;
        static inline std::mutex m_Mutex;
        static inline std::condition_variable m_CV;
        static inline std::thread m_Thread;
    };
}