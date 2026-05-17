#pragma once

class Logger
{
public:
    static void Init(const std::string& name)
    {
        m_File.open(name, std::ios::out | std::ios::trunc);
    }

    static void Destroy()
    {
        if (m_File.is_open())
            m_File.close();
    }

    static void Log(const char* file, const char* func, const char* msg)
    {
        if (!m_File.is_open())
            return;

        WritePrefix(file, func);
        m_File << msg << '\n';
        m_File.flush();
    }

    static void Logf(const char* file, const char* func, const char* fmt, ...)
    {
        if (!m_File.is_open())
            return;

        char buffer[1024];

        va_list args;
        va_start(args, fmt);
        _vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);
        va_end(args);

        WritePrefix(file, func);
        m_File << buffer << '\n';
        m_File.flush();
    }

private:
    static void WritePrefix(const char* file, const char* func)
    {
        const auto now = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(now);

        tm tm{};
        localtime_s(&tm, &time);

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

        auto fileName = file;
        for (auto p = file; *p; p++)
        {
            if (*p == '/' || *p == '\\')
                fileName = p + 1;
        }

        m_File << '[' << buffer << ']' << '[' << fileName << "] " << func << ": ";
    }

    static inline std::ofstream m_File;
};

#define LOG(msg) Logger::Log(__FILE__, __func__, msg)
#define LOGF(fmt, ...) Logger::Logf(__FILE__, __func__, fmt, __VA_ARGS__)