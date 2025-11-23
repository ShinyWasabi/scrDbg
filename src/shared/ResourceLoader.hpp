#pragma once

namespace scrDbgShared
{
    struct NativesBin
    {
    private:
        static inline std::vector<std::string> Names;
        static inline std::unordered_map<uint64_t, std::string_view> Map;

    public:
        static bool Load(HMODULE module);

        static std::string_view GetNameByHash(uint64_t hash)
        {
            auto it = Map.find(hash);
            return it != Map.end() ? it->second : std::string_view{};
        }
    };
}