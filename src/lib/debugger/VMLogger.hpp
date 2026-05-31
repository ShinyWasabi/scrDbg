#pragma once

namespace scrDbgLib
{
    enum class VMLogType
    {
        NONE,
        FRAME_TIME,
        FUNCTION_CALLS,
        NATIVE_CALLS,
        STATIC_WRITES,
        GLOBAL_WRITES
    };

    class VMLogger
    {
    public:
        static bool Init(const char* filename)
        {
            return GetInstance().InitImpl(filename);
        }

        static void Destroy()
        {
            GetInstance().DestroyImpl();
        }

        static void SetLogType(VMLogType type)
        {
            GetInstance().SetLogTypeImpl(type);
        }

        static void SetScriptHash(uint32_t hash)
        {
            GetInstance().SetScriptHashImpl(hash);
        }

        static bool ShouldLog(VMLogType type, uint32_t hash)
        {
            return GetInstance().ShouldLogImpl(type, hash);
        }

        static void Clear()
        {
            GetInstance().ClearImpl();
        }

        static void Log(const char* message)
        {
            GetInstance().Log(message);
        }

        static void Logf(const char* fmt, ...)
        {
            va_list args;
            va_start(args, fmt);

            GetInstance().LogfImpl(fmt, args);

            va_end(args);
        }

    private:
        static VMLogger& GetInstance()
        {
            static VMLogger instance;
            return instance;
        }

        bool InitImpl(const char* filename);
        void DestroyImpl();

        void SetLogTypeImpl(VMLogType type);
        void SetScriptHashImpl(uint32_t hash);
        bool ShouldLogImpl(VMLogType type, uint32_t hash);

        void ClearImpl();
        void LogImpl(const char* message);
        void LogfImpl(const char* fmt, va_list args);

        void LoggerThread();

        static constexpr uint32_t ALL_SCRIPTS_HASH = 0;

        std::ofstream m_File;
        std::string m_FilePath;
        VMLogType m_LogType = VMLogType::NONE;
        uint32_t m_ScriptHash = ALL_SCRIPTS_HASH;
        std::atomic<bool> m_Running;
        std::queue<std::string> m_Queue;
        std::mutex m_Mutex;
        std::condition_variable m_CV;
        std::thread m_Thread;
    };
}