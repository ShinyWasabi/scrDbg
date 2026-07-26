#pragma once

#undef Yield

namespace scrDbgLib
{
    class ScriptFiber
    {
    public:
        static void Init(int numJobs = 10)
        {
            GetInstance().InitImpl(numJobs);
        }

        static void Destroy()
        {
            GetInstance().DestroyImpl();
        }

        static void Tick(uint32_t currentScriptHash)
        {
            GetInstance().TickImpl(currentScriptHash);
        }

        static void Yield(std::optional<std::chrono::high_resolution_clock::duration> time = std::nullopt)
        {
            GetInstance().YieldImpl(time);
        }

        static void Add(std::function<void()> callback)
        {
            GetInstance().AddImpl(callback);
        }

        static void PushJob(std::function<void()> callback)
        {
            GetInstance().PushJobImpl(std::move(callback), std::nullopt);
        }

        static void PushJob(std::function<void()> callback, uint32_t targetScriptHash)
        {
            GetInstance().PushJobImpl(std::move(callback), targetScriptHash);
        }

    private:
        struct Job
        {
            std::function<void()> Callback;
            std::optional<uint32_t> TargetScriptHash;
        };

        class Fiber
        {
        public:
            explicit Fiber(std::function<void()> callback);
            ~Fiber();

            void Tick();
            void Yield(std::optional<std::chrono::high_resolution_clock::duration> time = std::nullopt);

        private:
            std::function<void()> m_Callback;
            bool m_Done;
            HANDLE m_ChildFiber;
            HANDLE m_MainFiber;
            std::optional<std::chrono::high_resolution_clock::time_point> m_WakeTime;
        };

        static ScriptFiber& GetInstance()
        {
            static ScriptFiber instance;
            return instance;
        }

        static void JobsScript();

        void InitImpl(int numJobs);
        void DestroyImpl();
        void TickImpl(uint32_t currentScriptHash);
        void TickJobs();
        void YieldImpl(std::optional<std::chrono::high_resolution_clock::duration> time = std::nullopt);
        void AddImpl(std::function<void()> callback);
        void PushJobImpl(std::function<void()> callback, std::optional<uint32_t> targetScriptHash);

        std::mutex m_Mutex;
        std::mutex m_JobsMutex;
        std::vector<std::unique_ptr<Fiber>> m_Fibers;
        std::deque<Job> m_Jobs;
        uint32_t m_CurrentScriptHash = 0;
    };
}