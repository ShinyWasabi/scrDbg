#pragma once

namespace scrDbgLib
{
    class PipeServer
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

        static void Run()
        {
            GetInstance().RunImpl();
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
        static PipeServer& GetInstance()
        {
            static PipeServer instance;
            return instance;
        }

        bool InitImpl(const std::string& name);
        void DestroyImpl();
        void RunImpl();
        bool SendImpl(const void* data, size_t size);
        bool ReceiveImpl(void* data, size_t size);
        bool Wait();

        HANDLE m_PipeHandle = INVALID_HANDLE_VALUE;
    };
}