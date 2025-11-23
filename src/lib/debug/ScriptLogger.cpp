#include "ScriptLogger.hpp"

namespace scrDbgLib
{
    bool ScriptLogger::Init(const char* filename)
    {
        char dllPath[MAX_PATH];
        if (GetModuleFileNameA(g_DllInstance, dllPath, MAX_PATH) == 0)
            return false;

        std::filesystem::path path = std::filesystem::path(dllPath).parent_path() / filename;
        m_FilePath = path.string();

        m_File.open(m_FilePath, std::ios::out | std::ios::trunc);
        if (!m_File.is_open())
            return false;

        m_Running = true;
        m_Thread = std::thread(LoggerThread);
        return true;
    }

    void ScriptLogger::Destroy()
    {
        if (!m_Running)
            return;

        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Running = false;
        }
        m_CV.notify_all();

        if (m_Thread.joinable())
            m_Thread.join();

        if (m_File.is_open())
            m_File.close();

        m_LogType = LogType::LOG_TYPE_NONE;
        m_ScriptHash = 0;
    }

    void ScriptLogger::SetLogType(LogType type)
    {
        m_LogType = type;
    }

    ScriptLogger::LogType ScriptLogger::GetLogType()
    {
        return m_LogType;
    }

    void ScriptLogger::SetScriptHash(std::uint32_t hash)
    {
        m_ScriptHash = hash;
    }

    uint32_t ScriptLogger::GetScriptHash()
    {
        return m_ScriptHash;
    }

    void ScriptLogger::Clear()
    {
        if (!m_Running)
            return;

        std::lock_guard<std::mutex> lock(m_Mutex);
        m_File.close();
        m_File.open(m_FilePath, std::ios::out | std::ios::trunc);
    }

    void ScriptLogger::Log(LogType type, std::uint32_t hash, const char* message)
    {
        if (!m_Running || m_LogType != type || (m_ScriptHash != 0 && m_ScriptHash != hash))
            return;

        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Queue.push(std::string(message));
        m_CV.notify_one();
    }

    void ScriptLogger::Logf(LogType type, std::uint32_t hash, const char* fmt, ...)
    {
        if (!m_Running || m_LogType != type || (m_ScriptHash != 0 && m_ScriptHash != hash))
            return;

        char buffer[1024];

        va_list args;
        va_start(args, fmt);
        _vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);
        va_end(args);

        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Queue.push(std::string(buffer));
        m_CV.notify_one();
    }

    void ScriptLogger::LoggerThread()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_CV.wait(lock, [] {
                return !m_Queue.empty() || !m_Running;
            });

            if (!m_Running && m_Queue.empty())
                break;

            while (!m_Queue.empty())
            {
                std::string msg = std::move(m_Queue.front());
                m_Queue.pop();
                lock.unlock();

                if (m_File.is_open())
                    m_File << msg << '\n';

                lock.lock();
            }

            if (m_File.is_open())
                m_File.flush();
        }
    }
}