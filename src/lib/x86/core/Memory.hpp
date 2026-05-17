#pragma once

class Memory
{
public:
    Memory(void* ptr = nullptr)
        : m_Ptr(ptr)
    {
    }

    explicit Memory(uintptr_t ptr)
        : m_Ptr(reinterpret_cast<void*>(ptr))
    {
    }

    template <typename T>
    std::enable_if_t<std::is_pointer_v<T>, T> As() const
    {
        return reinterpret_cast<T>(m_Ptr);
    }

    template <typename T>
    std::enable_if_t<std::is_lvalue_reference_v<T>, T> As() const
    {
        return *reinterpret_cast<std::add_pointer_t<std::remove_reference_t<T>>>(m_Ptr);
    }

    template <typename T>
    std::enable_if_t<std::is_same_v<T, uintptr_t>, T> As() const
    {
        return reinterpret_cast<uintptr_t>(m_Ptr);
    }

    template <typename T>
    Memory Add(T offset) const
    {
        return Memory(As<uintptr_t>() + offset);
    }

    template <typename T>
    Memory Sub(T offset) const
    {
        return Memory(As<uintptr_t>() - offset);
    }

    Memory Rip() const
    {
        return Add(As<int32_t&>()).Add(4);
    }

    static std::vector<std::optional<uint8_t>> ParsePattern(std::string_view pattern);
    static std::optional<Memory> ScanPattern(const char* pattern);

private:
    void* m_Ptr;
};