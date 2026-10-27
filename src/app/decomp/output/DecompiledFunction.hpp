#pragma once

namespace scrDbgApp::Decomp
{
    // Character range on one line produced by an AST node built from the instruction at Address
    struct DecompiledSpan
    {
        int Line;
        int Start;
        int End;
        uint32_t Address;
    };

    // Character range naming a function (call target, indirect-call target, function pointer) that can be navigated to.
    struct DecompiledLink
    {
        int Line;
        int Start;
        int End;
        uint32_t Target;
    };

    // Pseudocode for one function plus the maps between its text and its bytecode
    class DecompiledFunction
    {
    public:
        std::string Name;
        uint32_t Start = 0;
        uint32_t End = 0;

        std::vector<std::string> Lines;
        std::vector<std::vector<uint32_t>> LineAddresses; // sorted, unique
        std::vector<DecompiledSpan> Spans;
        std::vector<DecompiledLink> Links;
        std::unordered_map<uint32_t, int> AddressToLine; // Every instruction address of the function and the line it belongs to

        std::string ToText() const;

        int FindLine(uint32_t address) const;

        std::vector<DecompiledSpan> FindSpans(uint32_t address) const;

        // narrowest span containing the column
        std::optional<DecompiledSpan> FindSpanAt(int line, int column) const;

        std::optional<uint32_t> FindLinkAt(int line, int column) const;

        // Instruction for a cursor position.
        std::optional<uint32_t> GetAddressAt(int line, int column) const;

        // The address a line stands for
        std::optional<uint32_t> GetLineAddress(int line) const;

        // Gives every instruction that no line claimed (NOPs, ENTER, collapsed short-circuit DUP/INOT/JZ, etc.) the line of the next claimed one, or of the previous one at the end of the function.
        void PlaceUnclaimedInstructions(std::vector<uint32_t> instructionAddresses, int fallbackLine);
    };
}