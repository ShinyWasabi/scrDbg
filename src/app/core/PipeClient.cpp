#include "PipeClient.hpp"

namespace scrDbgApp
{
    bool PipeClient::InitImpl(const std::string& name)
    {
        if (m_PipeHandle != INVALID_HANDLE_VALUE)
            return false;

        const auto start = std::chrono::steady_clock::now();
        const auto end = start + std::chrono::seconds(5);
        while (std::chrono::steady_clock::now() < end)
        {
            m_PipeHandle = CreateFileA(("\\\\.\\pipe\\" + name).c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (m_PipeHandle != INVALID_HANDLE_VALUE)
                return true;

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Timed out
        return false;
    }

    void PipeClient::DestroyImpl()
    {
        if (m_PipeHandle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_PipeHandle);
            m_PipeHandle = INVALID_HANDLE_VALUE;
        }
    }

    bool PipeClient::SendImpl(const void* data, size_t size)
    {
        if (m_PipeHandle == INVALID_HANDLE_VALUE)
            return false;

        DWORD written = 0;
        return WriteFile(m_PipeHandle, data, static_cast<DWORD>(size), &written, nullptr) && written == size;
    }

    bool PipeClient::ReceiveImpl(void* data, size_t size)
    {
        if (m_PipeHandle == INVALID_HANDLE_VALUE)
            return false;

        DWORD read = 0;
        return ReadFile(m_PipeHandle, data, static_cast<DWORD>(size), &read, nullptr) && read == size;
    }
}