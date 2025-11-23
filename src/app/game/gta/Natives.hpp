#pragma once

namespace gta::Natives
{
    extern std::string_view GetNameByHash(uint64_t hash);
    extern uint64_t GetHashByHandler(uint64_t handler);
    extern std::unordered_map<uint64_t, uint64_t> GetAll();
}