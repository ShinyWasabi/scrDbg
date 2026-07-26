#pragma once

namespace scrDbg
{
    struct NativeDB
    {
    public:
        enum class Types : uint8_t
        {
            NONE = 0,
            INT,
            BOOL,
            FLOAT,
            STRING,
            REFERENCE
        };

        static bool Load(HMODULE module, int id);

        static std::string_view GetNameByHash(uint64_t hash)
        {
            auto it = Names.find(hash);
            return it != Names.end() ? std::string_view(it->second) : std::string_view{};
        }

        static const std::vector<Types>* GetArgsByHash(uint64_t hash)
        {
            auto it = Args.find(hash);
            return it != Args.end() ? &it->second : nullptr;
        }

        static const std::vector<Types>* GetRetsByHash(uint64_t hash)
        {
            auto it = Rets.find(hash);
            return it != Rets.end() ? &it->second : nullptr;
        }

        static const std::unordered_map<uint64_t, std::string>& GetNames()
        {
            return Names;
        }

    private:
        static inline std::unordered_map<uint64_t, std::string> Names;
        static inline std::unordered_map<uint64_t, std::vector<Types>> Args;
        static inline std::unordered_map<uint64_t, std::vector<Types>> Rets;
    };
}