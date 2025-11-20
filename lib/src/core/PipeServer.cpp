#include "PipeServer.hpp"
#include "PipeCommands.hpp"
#include "debug/ScriptBreakpoint.hpp"

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

        std::uint8_t cmdByte = 0;
        if (!Receive(&cmdByte, sizeof(cmdByte)))
        {
            DisconnectNamedPipe(m_PipeHandle);
            break;
        }

        auto cmd = static_cast<ePipeCommands>(cmdByte);
        switch (cmd)
        {
        case ePipeCommands::BREAKPOINT_SET:
        {
            PipeBreakpointSet args{};
            Receive(&args, sizeof(args));

            if (args.Set)
                ScriptBreakpoint::Add(args.Script, args.Pc);
            else
                ScriptBreakpoint::Remove(args.Script, args.Pc);
            break;
        }
        case ePipeCommands::BREAKPOINT_EXISTS:
        {
            PipeBreakpoint args{};
            Receive(&args, sizeof(args));

            bool result = ScriptBreakpoint::Exists(args.Script, args.Pc);
            Send(&result, sizeof(result));
            break;
        }
        case ePipeCommands::BREAKPOINT_RESUME:
        {
            ScriptBreakpoint::Resume();
            break;
        }
        case ePipeCommands::BREAKPOINT_PAUSE_GAME:
        {
            bool pause = false;
            Receive(&pause, sizeof(pause));

            ScriptBreakpoint::SetPauseGame(pause);
            break;
        }
        case ePipeCommands::BREAKPOINT_GET_ACTIVE:
        {
            auto active = ScriptBreakpoint::GetActive();

            bool hasActive = active.has_value();
            Send(&hasActive, sizeof(hasActive));

            if (active)
            {
                PipeBreakpoint entry{active->first, active->second};
                Send(&entry, sizeof(entry));
            }
            break;
        }
        case ePipeCommands::BREAKPOINT_GET_ALL:
        {
            auto bps = ScriptBreakpoint::GetAll();

            uint32_t count = static_cast<uint32_t>(bps.size());
            Send(&count, sizeof(count));

            for (const auto& bp : bps)
            {
                PipeBreakpoint entry{bp.first, bp.second};
                Send(&entry, sizeof(entry));
            }
            break;
        }
        case ePipeCommands::BREAKPOINT_REMOVE_ALL:
        {
            ScriptBreakpoint::RemoveAll();
            break;
        }
        default:
            // Do nothing?
            break;
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

bool PipeServer::Send(const void* data, size_t size)
{
    if (m_PipeHandle == INVALID_HANDLE_VALUE)
        return false;

    DWORD written = 0;
    return WriteFile(m_PipeHandle, data, static_cast<DWORD>(size), &written, nullptr) && written == size;
}

bool PipeServer::Receive(void* data, size_t size)
{
    if (m_PipeHandle == INVALID_HANDLE_VALUE)
        return false;

    DWORD read = 0;
    return ReadFile(m_PipeHandle, data, static_cast<DWORD>(size), &read, nullptr) && read == size;
}