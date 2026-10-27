#include "Decompiler.hpp"
#include "builders/FlatBuilder.hpp"
#include "builders/StatementTreeBuilder.hpp"
#include "ir/Function.hpp"
#include "output/CodeWriter.hpp"
#include "pipe/PipeCommands.hpp"

namespace scrDbgApp
{
    using namespace Decomp;

    Decompiler::Decompiler(const Disassembler& disassembler)
        : m_Disassembler(disassembler),
          m_FunctionsBuilt(false)
    {
    }

    Decompiler::~Decompiler() = default;

    void Decompiler::EnsureFunctions()
    {
        if (m_FunctionsBuilt)
            return;

        m_FunctionsBuilt = true;

        int count = m_Disassembler.GetFunctionCount();
        m_Functions.reserve(count);

        for (int i = 0; i < count; i++)
            m_Functions.push_back(std::make_unique<Function>(m_Disassembler.GetFunction(i), GetReservedFrameSlots()));
    }

    int Decompiler::GetFunctionCount()
    {
        EnsureFunctions();
        return static_cast<int>(m_Functions.size());
    }

    std::string Decompiler::GetFunctionName(int index)
    {
        EnsureFunctions();
        if (index < 0 || index >= static_cast<int>(m_Functions.size()))
            return {};

        return m_Functions[index]->GetName();
    }

    int Decompiler::GetFunctionIndexForPc(uint32_t pc)
    {
        EnsureFunctions();

        auto it = std::upper_bound(m_Functions.begin(), m_Functions.end(), pc, [](uint32_t value, const std::unique_ptr<Function>& func) {
            return value < func->GetStart();
        });

        if (it == m_Functions.begin())
            return -1;

        --it;
        if (pc >= (*it)->GetEnd())
            return -1;

        return static_cast<int>(it - m_Functions.begin());
    }

    const Function* Decompiler::FindCallTarget(uint32_t address)
    {
        EnsureFunctions();

        auto it = std::lower_bound(m_Functions.begin(), m_Functions.end(), address, [](const std::unique_ptr<Function>& func, uint32_t value) {
            return func->GetStart() < value;
        });

        if (it == m_Functions.end() || (*it)->GetStart() != address)
            return nullptr;

        return it->get();
    }

    uint64_t Decompiler::GetNativeHash(uint32_t index) const
    {
        uint64_t handler = m_Disassembler.GetProgram()->GetNative(index);
        return handler ? g_Game->GetNativeHashByHandler(handler) : 0;
    }

    std::string Decompiler::GetNativeName(uint32_t index)
    {
        if (auto it = m_NativeNames.find(index); it != m_NativeNames.end())
            return it->second;

        char buf[64];
        std::string name;

        uint64_t hash = GetNativeHash(index);
        if (!hash)
        {
            std::snprintf(buf, sizeof(buf), "UNKNOWN_NATIVE_%u", index);
            name = buf;
        }
        else
        {
            name = NativeDB::GetNameByHash(hash);
            if (name.empty())
            {
                std::snprintf(buf, sizeof(buf), "unk_0x%016llX", static_cast<unsigned long long>(hash));
                name = buf;
            }
        }

        return m_NativeNames.emplace(index, std::move(name)).first->second;
    }

    std::string Decompiler::GetString(uint32_t index)
    {
        if (auto it = m_Strings.find(index); it != m_Strings.end())
            return it->second;

        return m_Strings.emplace(index, m_Disassembler.GetProgram()->GetString(index)).first->second;
    }

    std::string Decompiler::GetStringComment(const std::string& value)
    {
        if (value.empty())
            return {};

        if (auto it = m_StringComments.find(value); it != m_StringComments.end())
            return it->second;

        std::string label = g_Game->GetTextLabel(JOAAT(value));
        std::string comment = label.empty() ? std::string() : "GXT: " + label;
        return m_StringComments.emplace(value, std::move(comment)).first->second;
    }

    std::unordered_map<uint32_t, std::vector<uint32_t>> Decompiler::FetchIndirectCalls() const
    {
        return PipeCommands::GetIndirectCalls(m_Disassembler.GetHash());
    }

    const std::vector<uint32_t>* Decompiler::GetIndirectCallTargets(uint32_t callSite)
    {
        if (!m_IndirectCalls)
            m_IndirectCalls = FetchIndirectCalls();

        auto it = m_IndirectCalls->find(callSite);
        return it != m_IndirectCalls->end() ? &it->second : nullptr;
    }

    std::string Decompiler::GetScriptHeader()
    {
        EnsureFunctions();

        const ScriptProgram* program = m_Disassembler.GetProgram();

        char hash[16];
        std::snprintf(hash, sizeof(hash), "0x%08X", m_Disassembler.GetHash());

        std::ostringstream out;
        out << "/*\n";
        out << " * Script: " << program->GetName() << "\n";
        out << " * Hash: " << hash << "\n";
        out << " * Game build: " << g_Game->GetGameBuild() << "\n";
        out << " * Functions: " << m_Functions.size() << "\n";
        out << " * Statics: " << program->GetStaticCount() << "\n";
        out << " * Globals: " << program->GetGlobalCount() << "\n";
        out << " * Natives: " << program->GetNativeCount() << "\n";
        out << " * Decompiled by scrDbg " << SCRDBG_FILE_VERSION << "\n";
        out << " */\n";
        return out.str();
    }

    uint32_t Decompiler::GetFunctionStart(int index)
    {
        EnsureFunctions();
        if (index < 0 || index >= static_cast<int>(m_Functions.size()))
            return 0;

        return m_Functions[index]->GetStart();
    }

    DecompiledFunction Decompiler::DecompileFunction(int index)
    {
        EnsureFunctions();
        if (index < 0 || index >= static_cast<int>(m_Functions.size()))
            return {};

        Function& func = *m_Functions[index];

        CodeWriter writer;

        char location[64];
        std::snprintf(location, sizeof(location), "0x%X-0x%X", func.GetStart(), func.GetEnd());
        writer.Line(0, "// " + func.GetName() + " @ " + location + " | args: " + std::to_string(func.GetArgCount()) + " | locals: " + std::to_string(func.GetLocalCount()) + " | returns: " + std::to_string(func.GetRetCount()));

        // The signature line stands for the ENTER, so the function's first instruction (and a breakpoint on it) lands there
        std::string signature = (func.GetRetCount() == 0 ? "void " : "var ") + func.GetName() + "(";
        for (uint32_t i = 0; i < func.GetArgCount(); i++)
            signature += (i ? ", var " : "var ") + func.GetFrameVarName(i);
        signature += ")";
        const int signatureLine = writer.GetLineCount();
        writer.Line(0, signature, {func.GetStart()});
        writer.Line(0, "{");

        const uint32_t localBase = func.GetArgCount() + GetReservedFrameSlots();
        for (uint32_t i = 0; i < func.GetLocalCount(); i++)
            writer.Line(1, "var " + func.GetFrameVarName(localBase + i) + ";");
        if (func.GetLocalCount())
            writer.Blank();

        try
        {
            func.SetInstructions(Lift(func));
            auto tree = StatementTreeBuilder::Build(func, *this);

            CodeWriter body;
            tree->EmitBody(body, 1);
            writer.Append(std::move(body));
        }
        catch (const std::exception& structuredError)
        {
            writer.Line(1, std::string("// Structured decompilation failed: ") + structuredError.what());
            writer.Line(1, "// Showing a flat statement list instead.");
            writer.Blank();

            try
            {
                func.SetInstructions(Lift(func));
                auto statements = FlatBuilder::Build(func, *this);

                CodeWriter body;
                for (const auto& statement : statements)
                    body.Line(dynamic_cast<Label*>(statement.get()) ? 0 : 1, statement->Render());
                writer.Append(std::move(body));
            }
            catch (const std::exception& flatError)
            {
                writer.Line(1, std::string("// FAILED TO DECOMPILE: ") + flatError.what());
            }
        }

        writer.Line(0, "}");

        std::vector<uint32_t> addresses;
        addresses.reserve(func.GetInstructions().size());
        for (const auto& ins : func.GetInstructions())
            addresses.push_back(ins.Offset);

        func.SetInstructions({});

        DecompiledFunction result = std::move(writer.GetResult());
        result.Name = func.GetName();
        result.Start = func.GetStart();
        result.End = func.GetEnd();
        result.PlaceUnclaimedInstructions(std::move(addresses), signatureLine);
        return result;
    }
}