#pragma once

namespace scrDbg
{
    class PipeClient
    {
    public:
        static bool Init(const std::string& name)
        {
            return GetInstance().InitImpl(name);
        }

        static void Destroy()
        {
            GetInstance().DestroyImpl();
        }

        static bool Send(const void* data, size_t size)
        {
            return GetInstance().SendImpl(data, size);
        }

        static bool Receive(void* data, size_t size)
        {
            return GetInstance().ReceiveImpl(data, size);
        }

    private:
        static PipeClient& GetInstance()
        {
            static PipeClient instance;
            return instance;
        }

        bool InitImpl(const std::string& name);
        void DestroyImpl();
        bool SendImpl(const void* data, size_t size);
        bool ReceiveImpl(void* data, size_t size);

        HANDLE m_PipeHandle = INVALID_HANDLE_VALUE;
    };
}