#pragma once

namespace scrDbg
{
	class Pointer;

	class Scanner
	{
	public:
		using ScanFunc = std::function<void(Pointer)>;

		void Add(const std::string& name, const std::string& pattern, const ScanFunc& func);
		bool Scan();

		static std::optional<uint64_t> ScanPattern(const std::string& pattern);

	private:
		struct Pattern
		{
			std::string m_Name;
			std::string m_Pattern;
			ScanFunc m_Func;
		};

		std::vector<Pattern> m_Patterns;
	};
}