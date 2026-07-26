#include "PipeServer.hpp"
#include "pipe/PipeCommands.hpp"

namespace scrDbgLib
{
    bool PipeServer::InitImpl(const std::string& name)
    {
        if (m_PipeHandle != INVALID_HANDLE_VALUE)
            return false;

        m_PipeHandle = CreateNamedPipeA(("\\\\.\\pipe\\" + name).c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 4096, 4096, 0, nullptr);

        return m_PipeHandle != INVALID_HANDLE_VALUE;
    }

    void PipeServer::DestroyImpl()
    {
        if (m_PipeHandle != INVALID_HANDLE_VALUE)
        {
            DisconnectNamedPipe(m_PipeHandle);
            CloseHandle(m_PipeHandle);
            m_PipeHandle = INVALID_HANDLE_VALUE;
        }
    }

    void PipeServer::RunImpl()
    {
        while (true)
        {
            if (!Wait())
                continue;

            uint8_t cmdByte = 0;
            if (!Receive(&cmdByte, sizeof(cmdByte)))
            {
                DisconnectNamedPipe(m_PipeHandle);
                break;
            }

            auto cmd = static_cast<PipeCommands::ePipeCommands>(cmdByte);

            switch (cmd)
            {
            case PipeCommands::ePipeCommands::BREAKPOINT_SET:
            {
                PipeCommands::SetBreakpoint();
                break;
            }
            case PipeCommands::ePipeCommands::BREAKPOINT_EXISTS:
            {
                PipeCommands::BreakpointExists();
                break;
            }
            case PipeCommands::ePipeCommands::BREAKPOINT_RESUME:
            {
                PipeCommands::ResumeBreakpoint();
                break;
            }
            case PipeCommands::ePipeCommands::BREAKPOINT_PAUSE_GAME:
            {
                PipeCommands::SetBreakpointPauseGame();
                break;
            }
            case PipeCommands::ePipeCommands::BREAKPOINT_GET_ACTIVE:
            {
                PipeCommands::GetActiveBreakpoint();
                break;
            }
            case PipeCommands::ePipeCommands::BREAKPOINT_GET_ALL:
            {
                PipeCommands::GetAllBreakpoints();
                break;
            }
            case PipeCommands::ePipeCommands::BREAKPOINT_REMOVE_ALL:
            {
                PipeCommands::RemoveAllBreakpoints();
                break;
            }
            case PipeCommands::ePipeCommands::NATIVE_INVOKE:
            {
                PipeCommands::InvokeNative();
                break;
            }
            case PipeCommands::ePipeCommands::LOGGER_SET_TYPE:
            {
                PipeCommands::SetLoggerType();
                break;
            }
            case PipeCommands::ePipeCommands::LOGGER_SET_SCRIPT:
            {
                PipeCommands::SetLoggerScript();
                break;
            }
            case PipeCommands::ePipeCommands::LOGGER_CLEAR_FILE:
            {
                PipeCommands::ClearLoggerFile();
                break;
            }
            }
        }
    }

    bool PipeServer::Wait()
    {
        if (m_PipeHandle == INVALID_HANDLE_VALUE)
            return false;

        if (ConnectNamedPipe(m_PipeHandle, nullptr))
            return true;

        return GetLastError() == ERROR_PIPE_CONNECTED;
    }

    bool PipeServer::SendImpl(const void* data, size_t size)
    {
        if (m_PipeHandle == INVALID_HANDLE_VALUE)
            return false;

        DWORD written = 0;
        return WriteFile(m_PipeHandle, data, static_cast<DWORD>(size), &written, nullptr) && written == size;
    }

    bool PipeServer::ReceiveImpl(void* data, size_t size)
    {
        if (m_PipeHandle == INVALID_HANDLE_VALUE)
            return false;

        DWORD read = 0;
        return ReadFile(m_PipeHandle, data, static_cast<DWORD>(size), &read, nullptr) && read == size;
    }
}