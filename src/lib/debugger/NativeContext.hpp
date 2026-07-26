#pragma once
#include "rage/payne/scrCommand.hpp"
#include "rage/shared/scrNativeContext.hpp"

namespace scrDbgLib
{
    class NativeContext
    {
    public:
        virtual ~NativeContext() = default;

        virtual void Reset() = 0;
        virtual void PushArg(rage::scrValue value) = 0;
        virtual void Invoke(void* handler) = 0;
        virtual const rage::scrValue* GetRets() const = 0;

    protected:
        rage::scrValue m_ArgStack[20]{};
        rage::scrValue m_RetStack[10]{};
    };

    template <typename Context, typename Handler, auto CopyVectorsOutFn>
    class NativeContextImpl : public NativeContext
    {
    public:
        NativeContextImpl()
        {
            m_Context.m_Args = m_ArgStack;
            m_Context.m_Rets = m_RetStack;
        }

        void Reset() override
        {
            m_Context.m_ArgCount = 0;
            m_Context.m_VectorRefCount = 0;
        }

        void PushArg(rage::scrValue value) override
        {
            m_Context.m_Args[m_Context.m_ArgCount++] = value;
        }

        void Invoke(void* handler) override
        {
            reinterpret_cast<Handler>(handler)(&m_Context);
            (m_Context.*CopyVectorsOutFn)();
        }

        const rage::scrValue* GetRets() const override
        {
            return m_Context.m_Rets;
        }

    protected:
        Context m_Context;
    };

    using NativeContextDefault = NativeContextImpl<rage::scrNativeContext, rage::scrNativeContext::Handler, &rage::scrNativeContext::CopyVectorsOut>;
#if defined(_M_X64)
    using NativeContextRDR2 = NativeContextImpl<rage::scrNativeContext, rage::scrNativeContext::Handler, &rage::scrNativeContext::CopyVectorsOut2>;
#endif
#if defined(_M_IX86)
    using NativeContextPayne = NativeContextImpl<rage::payne::scrCommand::Context, rage::payne::scrCommand::Context::Handler, &rage::payne::scrCommand::Context::CopyVectorsOut>;
#endif
}