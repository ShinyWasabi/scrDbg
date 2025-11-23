#include "ScriptBreakpoint.hpp"
#include "core/Memory.hpp"
#include "rage/enhanced/scrThread.hpp"
#include "rage/legacy/scrThread.hpp"

bool ScriptBreakpoint::Init()
{
    if (g_IsEnhanced)
    {
        if (auto addr = Memory::ScanPattern("56 48 83 EC 20 80 3D ? ? ? ? ? 75 ? 48 8D 0D"))
            m_PauseGameNow = addr->As<decltype(m_PauseGameNow)>();

        if (auto addr = Memory::ScanPattern("56 57 53 48 83 EC 20 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D"))
            m_UnpauseGameNow = addr->As<decltype(m_UnpauseGameNow)>();

        if (auto addr = Memory::ScanPattern("80 3D ? ? ? ? ? 74 ? 48 83 3D ? ? ? ? ? 74 ? 89 D0"))
            m_TogglePausedRenderPhases = addr->As<decltype(m_TogglePausedRenderPhases)>();

        if (auto addr = Memory::ScanPattern("80 88 ? ? ? ? ? EB ? E8"))
            m_PauseGameNowPatch = addr->Sub(0x1E).As<std::uint8_t*>();
    }
    else
    {
        if (auto addr = Memory::ScanPattern("C6 05 ? ? ? ? ? 75 ? E8 ? ? ? ? C6 05"))
            m_PauseGameNow = addr->Sub(0x43).As<decltype(m_PauseGameNow)>();

        if (auto addr = Memory::ScanPattern("E8 ? ? ? ? C6 05 ? ? ? ? ? 41 B0 01"))
            m_UnpauseGameNow = addr->Add(1).Rip().As<decltype(m_UnpauseGameNow)>();

        if (auto addr = Memory::ScanPattern("48 83 EC 28 45 33 D2 44 8A C1"))
            m_TogglePausedRenderPhases = addr->As<decltype(m_TogglePausedRenderPhases)>();

        if (auto addr = Memory::ScanPattern("75 ? E8 ? ? ? ? C6 05 ? ? ? ? ? EB ? 48 8B 05"))
            m_PauseGameNowPatch = addr->As<std::uint8_t*>();
    }

    return m_PauseGameNow && m_UnpauseGameNow && m_TogglePausedRenderPhases && m_PauseGameNowPatch;
}

void ScriptBreakpoint::PauseGame(bool pause)
{
    if (pause)
    {
        g_IsEnhanced ? * m_PauseGameNowPatch = 0xEB : *reinterpret_cast<std::uint16_t*>(m_PauseGameNowPatch) = 0x9090;
        m_PauseGameNow();
    }
    else
    {
        m_UnpauseGameNow();
        m_TogglePausedRenderPhases(true, 0);
        g_IsEnhanced ? * m_PauseGameNowPatch = 0x74 : *reinterpret_cast<std::uint16_t*>(m_PauseGameNowPatch) = 0x0E75;
    }
}

bool ScriptBreakpoint::Add(std::uint32_t script, std::uint32_t pc)
{
    if (Exists(script, pc))
        return false;

    m_Breakpoints.push_back({script, pc});
    return true;
}

bool ScriptBreakpoint::Remove(std::uint32_t script, std::uint32_t pc)
{
    auto it = std::find_if(m_Breakpoints.begin(), m_Breakpoints.end(), [script, pc](const std::pair<std::uint32_t, std::uint32_t>& bp) { return bp.first == script && bp.second == pc; });

    if (it == m_Breakpoints.end())
        return false;

    if (m_ActiveBreakpoint.has_value() && m_ActiveBreakpoint->first == script && m_ActiveBreakpoint->second == pc)
        Resume();

    m_Breakpoints.erase(it);
    return true;
}

bool ScriptBreakpoint::Exists(std::uint32_t script, std::uint32_t pc)
{
    return std::any_of(m_Breakpoints.begin(), m_Breakpoints.end(), [script, pc](const std::pair<std::uint32_t, std::uint32_t>& bp) { return bp.first == script && bp.second == pc; });
}

bool ScriptBreakpoint::Resume()
{
    return g_IsEnhanced ? rage::enhanced::scrThread::ResumeActiveBreakpoint() : rage::legacy::scrThread::ResumeActiveBreakpoint();
}

void ScriptBreakpoint::SetPauseGame(bool pause)
{
    m_BreakpointsPauseGame = pause;
}

std::optional<std::pair<std::uint32_t, std::uint32_t>> ScriptBreakpoint::GetActive()
{
    return m_ActiveBreakpoint;
}

std::vector<std::pair<std::uint32_t, std::uint32_t>> ScriptBreakpoint::GetAll()
{
    return m_Breakpoints;
}

void ScriptBreakpoint::RemoveAll()
{
    Resume();
    m_Breakpoints.clear();
}