#pragma once

namespace gta
{
    class Natives
    {
    public:
    	static bool Init()
    	{
    	    return GetInstance().InitImpl();
    	}
    
    	static uint64_t GetNativeHashByHandler(uint64_t handler)
    	{
    	    return GetInstance().GetNativeHashByHandlerImpl(handler);
    	}
    
    	static std::string_view GetNameByHash(uint64_t hash)
    	{
    	    auto it = GetInstance().m_Map.find(hash);
    	    return it != GetInstance().m_Map.end() ? it->second : std::string_view{};
    	}
    
    private:
    	static Natives& GetInstance()
    	{
    	    static Natives instance;
    	    return instance;
    	}
    
    	bool InitImpl();
    	uint64_t GetNativeHashByHandlerImpl(uint64_t handler);
    
    	std::vector<std::string> m_Names; // owns strings
    	std::unordered_map<uint64_t, std::string_view> m_Map;
    };
}