#pragma once

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

private:
    static PipeServer& GetInstance()
    {
        static PipeServer instance;
        return instance;
    }

    bool InitImpl(const std::string& name);
    void DestroyImpl();
    void RunImpl();

    bool Wait();
    bool Send(const void* data, size_t size);
    bool Receive(void* data, size_t size);

    HANDLE m_PipeHandle = INVALID_HANDLE_VALUE;
};