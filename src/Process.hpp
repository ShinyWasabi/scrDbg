#pragma once

namespace scrDbg
{
	class Process
	{
	public:
		static bool Init(const wchar_t* processName)
		{
			return GetInstance().InitImpl(processName);
		}

		static void Destroy()
		{
			GetInstance().DestroyImpl();
		}

		static uintptr_t GetBaseAddress()
		{
			return GetInstance().m_BaseAddress;
		}

		static size_t GetSize()
		{
			return GetInstance().m_Size;
		}

		static bool IsRunning()
		{
			return GetInstance().IsRunningImpl();
		}

		static bool IsModuleLoaded(const wchar_t* moduleName)
		{
			return GetInstance().IsModuleLoadedImpl(moduleName);
		}

		static bool ReadRaw(uintptr_t base, void* out, size_t size)
		{
			return GetInstance().ReadRawImpl(base, out, size);
		}

		static bool WriteRaw(uintptr_t base, const void* out, size_t size)
		{
			return GetInstance().WriteRawImpl(base, out, size);
		}

		template <typename T>
		static T Read(uintptr_t base)
		{
			T value{};
			ReadRaw(base, &value, sizeof(T));
			return value;
		}

		template <typename T>
		static void Write(uintptr_t base, const T& value)
		{
			WriteRaw(base, &value, sizeof(T));
		}

	private:
		static Process& GetInstance()
		{
			static Process instance;
			return instance;
		}

		bool InitImpl(const wchar_t* processName);
		void DestroyImpl();
		bool IsRunningImpl();
		bool IsModuleLoadedImpl(const wchar_t* moduleName);
		bool ReadRawImpl(uintptr_t base, void* value, size_t size);
		bool WriteRawImpl(uintptr_t base, const void* value, size_t size);

		HANDLE m_Handle;
		uintptr_t m_BaseAddress;
		size_t m_Size;
	};
}