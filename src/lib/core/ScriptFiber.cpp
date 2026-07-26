#include "ScriptFiber.hpp"

namespace scrDbgLib
{
    ScriptFiber::Fiber::Fiber(std::function<void()> callback)
        : m_Callback(callback),
          m_Done(false),
          m_ChildFiber(0),
          m_MainFiber(0),
          m_WakeTime(std::nullopt)
    {
        m_ChildFiber = CreateFiber(0, [](void* param) {
            auto thisFiber = static_cast<Fiber*>(param);
            thisFiber->m_Callback();
            thisFiber->m_Done = true;
            SwitchToFiber(thisFiber->m_MainFiber); }, this);
    }

    ScriptFiber::Fiber::~Fiber()
    {
        if (m_ChildFiber)
            DeleteFiber(m_ChildFiber);
    }

    void ScriptFiber::Fiber::Tick()
    {
        m_MainFiber = GetCurrentFiber();
        if ((!m_WakeTime.has_value() || m_WakeTime.value() <= std::chrono::high_resolution_clock::now()) && !m_Done)
            SwitchToFiber(m_ChildFiber);
    }

    void ScriptFiber::Fiber::Yield(std::optional<std::chrono::high_resolution_clock::duration> time)
    {
        if (time.has_value())
            m_WakeTime = std::chrono::high_resolution_clock::now() + time.value();
        else
            m_WakeTime = std::nullopt;

        SwitchToFiber(m_MainFiber);
    }

    void ScriptFiber::JobsScript()
    {
        while (true)
        {
            GetInstance().TickJobs();
            GetInstance().Yield();
        }
    }

    void ScriptFiber::InitImpl(int numJobs)
    {
        for (int i = 0; i < numJobs; i++)
            Add(JobsScript);
    }

    void ScriptFiber::DestroyImpl()
    {
        std::lock_guard jobsLock(m_JobsMutex);
        m_Jobs.clear();

        std::lock_guard fibersLock(m_Mutex);
        m_Fibers.clear();
    }

    void ScriptFiber::TickImpl(uint32_t currentScriptHash)
    {
        std::lock_guard lock(m_Mutex);
        static bool ensureMainFiber = (ConvertThreadToFiber(nullptr), true);

        m_CurrentScriptHash = currentScriptHash;

        for (const auto& script : m_Fibers)
            script->Tick();
    }

    void ScriptFiber::TickJobs()
    {
        std::unique_lock lock(m_JobsMutex);

        for (auto it = m_Jobs.begin(); it != m_Jobs.end(); ++it)
        {
            if (it->TargetScriptHash.has_value() && it->TargetScriptHash.value() != m_CurrentScriptHash)
                continue;

            Job job = std::move(*it);
            m_Jobs.erase(it);
            lock.unlock();

            std::invoke(std::move(job.Callback));
            return;
        }
    }

    void ScriptFiber::YieldImpl(std::optional<std::chrono::high_resolution_clock::duration> time)
    {
        if (auto script = static_cast<Fiber*>(GetFiberData()))
            script->Yield(time);
    }

    void ScriptFiber::AddImpl(std::function<void()> callback)
    {
        std::lock_guard lock(m_Mutex);
        m_Fibers.push_back(std::move(std::make_unique<ScriptFiber::Fiber>(callback)));
    }

    void ScriptFiber::PushJobImpl(std::function<void()> callback, std::optional<uint32_t> targetScriptHash)
    {
        std::lock_guard lock(m_JobsMutex);
        m_Jobs.push_back(Job{std::move(callback), targetScriptHash});
    }
}