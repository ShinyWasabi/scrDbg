#include "scrThread.hpp"
#include "Pointers.hpp"
#include "Process.hpp"

namespace rage
{
	uint32_t scrThread::GetId() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Context) + offsetof(Enhanced::Context, m_ThreadId)
			: offsetof(Legacy, m_Context) + offsetof(Legacy::Context, m_ThreadId);

		return scrDbg::Process::Read<uint32_t>(m_Address + offset);
	}

	uint32_t scrThread::GetProgram() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Context) + offsetof(Enhanced::Context, m_Program)
			: offsetof(Legacy, m_Context) + offsetof(Legacy::Context, m_Program);

		return scrDbg::Process::Read<uint32_t>(m_Address + offset);
	}

	scrThreadState scrThread::GetState() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Context) + offsetof(Enhanced::Context, m_State)
			: offsetof(Legacy, m_Context) + offsetof(Legacy::Context, m_State);

		return scrDbg::Process::Read<scrThreadState>(m_Address + offset);
	}

	void scrThread::SetState(scrThreadState state) const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Context) + offsetof(Enhanced::Context, m_State)
			: offsetof(Legacy, m_Context) + offsetof(Legacy::Context, m_State);

		scrDbg::Process::Write<scrThreadState>(m_Address + offset, state);
	}

	uint32_t scrThread::GetProgramCounter() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Context) + offsetof(Enhanced::Context, m_ProgramCounter)
			: offsetof(Legacy, m_Context) + offsetof(Legacy::Context, m_ProgramCounter);

		return scrDbg::Process::Read<uint32_t>(m_Address + offset);
	}

	uint32_t scrThread::GetFramePointer() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Context) + offsetof(Enhanced::Context, m_FramePointer)
			: offsetof(Legacy, m_Context) + offsetof(Legacy::Context, m_FramePointer);

		return scrDbg::Process::Read<uint32_t>(m_Address + offset);
	}

	uint32_t scrThread::GetStackPointer() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Context) + offsetof(Enhanced::Context, m_StackPointer)
			: offsetof(Legacy, m_Context) + offsetof(Legacy::Context, m_StackPointer);

		return scrDbg::Process::Read<uint32_t>(m_Address + offset);
	}

	float scrThread::GetTimerA() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Context) + offsetof(Enhanced::Context, m_TimerA)
			: offsetof(Legacy, m_Context) + offsetof(Legacy::Context, m_TimerA);

		return scrDbg::Process::Read<float>(m_Address + offset);
	}

	float scrThread::GetTimerB() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Context) + offsetof(Enhanced::Context, m_TimerB)
			: offsetof(Legacy, m_Context) + offsetof(Legacy::Context, m_TimerB);

		return scrDbg::Process::Read<float>(m_Address + offset);
	}

	float scrThread::GetWaitTimer() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Context) + offsetof(Enhanced::Context, m_WaitTimer)
			: offsetof(Legacy, m_Context) + offsetof(Legacy::Context, m_WaitTimer);

		return scrDbg::Process::Read<float>(m_Address + offset);
	}

	uint32_t scrThread::GetStackSize() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Context) + offsetof(Enhanced::Context, m_StackSize)
			: offsetof(Legacy, m_Context) + offsetof(Legacy::Context, m_StackSize);

		return scrDbg::Process::Read<uint32_t>(m_Address + offset);
	}

	scrThreadPriority scrThread::GetPriority() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Context) + offsetof(Enhanced::Context, m_Priority)
			: offsetof(Legacy, m_Context) + offsetof(Legacy::Context, m_Priority);

		return scrDbg::Process::Read<scrThreadPriority>(m_Address + offset);
	}

	uint64_t scrThread::GetStack(uint64_t index) const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Stack)
			: offsetof(Legacy, m_Stack);

		uint64_t base = scrDbg::Process::Read<uint64_t>(m_Address + offset);
		return scrDbg::Process::Read<uint64_t>(base + (index * sizeof(uint64_t)));
	}

	void scrThread::SetStack(uint64_t index, uint64_t value) const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_Stack)
			: offsetof(Legacy, m_Stack);

		uint64_t base = scrDbg::Process::Read<uint64_t>(m_Address + offset);
		scrDbg::Process::Write<uint64_t>(base + (index * sizeof(uint64_t)), value);
	}

	std::string scrThread::GetErrorMessage() const
	{
		std::string message;
		if (scrDbg::g_IsEnhanced)
		{
			size_t offset = offsetof(Enhanced, m_ErrorMessage);
			for (size_t i = 0; i < 128; ++i)
			{
				char c = scrDbg::Process::Read<char>(m_Address + offset + i);
				if (c == '\0')
					break;
				message.push_back(c);
			}
		}
		else
		{
			size_t offset = offsetof(Legacy, m_ErrorMessage);
			uintptr_t ptr = scrDbg::Process::Read<uintptr_t>(m_Address + offset);
			if (!ptr)
				return message;

			for (size_t i = 0; i < 128; ++i)
			{
				char c = scrDbg::Process::Read<char>(ptr + i);
				if (c == '\0')
					break;
				message.push_back(c);
			}
		}

		return message;
	}

	uint32_t scrThread::GetHash() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_ScriptHash)
			: offsetof(Legacy, m_ScriptHash);

		return scrDbg::Process::Read<uint32_t>(m_Address + offset);
	}

	std::string scrThread::GetName() const
	{
		size_t offset = scrDbg::g_IsEnhanced
			? offsetof(Enhanced, m_ScriptName)
			: offsetof(Legacy, m_ScriptName);

		std::string name;
		for (size_t i = 0; i < 64; ++i)
		{
			char c = scrDbg::Process::Read<char>(m_Address + offset + i);
			if (c == '\0')
				break;
			name.push_back(c);
		}

		return name;
	}

	std::vector<scrThread> scrThread::GetThreads()
	{
		std::vector<scrThread> threads;

		uint64_t base = scrDbg::g_Pointers.ScriptThreads.Read<uint64_t>();
		uint16_t count = scrDbg::g_Pointers.ScriptThreads.Add(10).Read<uint16_t>();

		for (uint16_t i = 0; i < count; ++i)
		{
			scrThread thread(scrDbg::Process::Read<uint64_t>(base + i * sizeof(uint64_t)));
			threads.push_back(thread);
		}

		return threads;
	}

	scrThread scrThread::GetThread(uint32_t hash)
	{
		if (!hash)
			return scrThread();

		uint64_t base = scrDbg::g_Pointers.ScriptThreads.Read<uint64_t>();
		uint16_t count = scrDbg::g_Pointers.ScriptThreads.Add(10).Read<uint16_t>();

		for (int i = 0; i < count; i++)
		{
			scrThread thread(scrDbg::Process::Read<uint64_t>(base + i * sizeof(uint64_t)));
			if (thread.GetHash() == hash)
				return thread;
		}

		return scrThread();
	}
}