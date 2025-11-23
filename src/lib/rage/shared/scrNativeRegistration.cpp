#include "scrNativeRegistration.hpp"
#include "core/Memory.hpp"

namespace rage::shared
{
    bool scrNativeRegistration::Init()
    {
        if (m_NativeRegistrationTable)
            return true;

        if (scrDbgLib::g_IsEnhanced)
        {
            if (auto addr = scrDbgLib::Memory::ScanPattern("4C 8D 0D ? ? ? ? 4C 8D 15 ? ? ? ? 45 31 F6"))
                m_NativeRegistrationTable = addr->Add(3).Rip().As<scrNativeRegistration*>();
        }
        else
        {
            if (auto addr = scrDbgLib::Memory::ScanPattern("48 8D 0D ? ? ? ? 48 8B 14 FA E8 ? ? ? ? 48 85 C0 75 0A"))
                m_NativeRegistrationTable = addr->Add(3).Rip().As<scrNativeRegistration*>();
        }

        return m_NativeRegistrationTable != nullptr;
    }
}