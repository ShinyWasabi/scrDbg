#pragma once

namespace rage
{
	class scrProgram
	{
		class _scrProgram
		{
		public:
			char m_Pad1[0x10];
			uint8_t** m_CodeBlocks;
			uint32_t m_GlobalVersion;
			uint32_t m_CodeSize;
			uint32_t m_ArgCount;
			uint32_t m_StaticCount;
			uint32_t m_GlobalCountAndBlock;
			uint32_t m_NativeCount;
			uint64_t* m_Statics;
			uint64_t** m_Globals;
			uint64_t* m_Natives;
			uint32_t m_ProcCount;
			char m_Pad2[0x04];
			const char** m_ProcNames;
			uint32_t m_NameHash;
			uint32_t m_RefCount;
			const char* m_Name;
			const char** m_Strings;
			uint32_t m_StringCount;
			char m_Breakpoints[0x0C];
		};

	public:
		scrProgram(uint64_t address = 0) :
			m_Address(address)
		{
		}

        std::vector<uint8_t> GetFullCode() const;
        uint8_t GetCode(uint32_t index) const;
        void SetCode(uint32_t index, uint8_t byte) const;
        uint32_t GetGlobalVersion() const;
        uint32_t GetCodeSize() const;
        uint32_t GetArgCount() const;
        uint32_t GetStaticCount() const;
        uint32_t GetGlobalCount() const; 
        uint32_t GetGlobalBlockIndex() const; 
        uint32_t GetNativeCount() const;
        uint32_t GetHash() const;
        uint32_t GetRefCount() const;
        std::vector<std::string> GetAllStrings() const;
        std::string GetString(uint32_t index) const;
        uint32_t GetStringCount() const;
        
        static scrProgram GetProgram(uint32_t hash);

		operator bool() const
		{
			return m_Address != 0;
		}

	private:
		uint64_t m_Address;
	};
}