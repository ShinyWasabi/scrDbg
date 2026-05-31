#include "VMLogger.hpp"

namespace scrDbgLib
{
    bool VMLogger::InitImpl(const char* filename)
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
        m_Thread = std::thread(&VMLogger::LoggerThread, this);
        return true;
    }

    void VMLogger::DestroyImpl()
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

        m_LogType = VMLogType::NONE;
        m_ScriptHash = 0;
    }

    void VMLogger::SetLogTypeImpl(VMLogType type)
    {
        m_LogType = type;
    }

    void VMLogger::SetScriptHashImpl(uint32_t hash)
    {
        m_ScriptHash = hash;
    }

    bool VMLogger::ShouldLogImpl(VMLogType type, uint32_t hash)
    {
        return m_LogType == type && (m_ScriptHash == ALL_SCRIPTS_HASH || m_ScriptHash == hash);
    }

    void VMLogger::ClearImpl()
    {
        if (!m_Running)
            return;

        std::lock_guard<std::mutex> lock(m_Mutex);
        m_File.close();
        m_File.open(m_FilePath, std::ios::out | std::ios::trunc);
    }

    void VMLogger::LogImpl(const char* message)
    {
        if (!m_Running)
            return;

        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Queue.push(std::string(message));
        m_CV.notify_one();
    }

    void VMLogger::LogfImpl(const char* fmt, va_list args)
    {
        if (!m_Running)
            return;

        char buffer[1024];

        _vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);

        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Queue.push(std::string(buffer));
        m_CV.notify_one();
    }

    void VMLogger::LoggerThread()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_CV.wait(lock, [this] {
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