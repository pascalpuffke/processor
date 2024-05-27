#include <disassembler.hpp>

#include <array>

// Clang doesnt seem to like constexpr std::array
#ifdef __clang__
#define ARRAY_CONSTEXPR
#else
#define ARRAY_CONSTEXPR constexpr
#endif

static ARRAY_CONSTEXPR std::array instruction_to_string = {
    "ldr",
    "st",
    "add",
    "sub",
    "mul",
    "div",
    "jp",
    "jz",
    "and",
    "or",
    "xor",
    "push",
    "pop",
    "ldi",
    "ldm",
    "done",
};

static ARRAY_CONSTEXPR std::array no_reg_instructions = {
    InstructionType::Done
};

static ARRAY_CONSTEXPR std::array single_reg_instructions = {
    InstructionType::Push,
    InstructionType::Pop
};

static ARRAY_CONSTEXPR std::array double_reg_instructions = {
    InstructionType::Jump,
    InstructionType::JumpIfZero,
    InstructionType::LoadFromReg
};

static ARRAY_CONSTEXPR std::array triple_reg_instructions = {
    InstructionType::Add,
    InstructionType::And,
    InstructionType::Div,
    InstructionType::LoadFromMem,
    InstructionType::Mul,
    InstructionType::Or,
    InstructionType::Store,
    InstructionType::Sub,
    InstructionType::Xor,
};

static ARRAY_CONSTEXPR std::array regimm_instructions = {
    InstructionType::LoadFromImm
};

template<typename T, std::size_t N>
bool array_contains(const std::array<T, N>& arr, const T& value) {
    for (const auto& it : arr) {
        if (it == value)
            return true;
    }
    return false;
}

auto Disassembler::disassemble(std::span<const ProcessorSpec::insr_t> code) -> std::vector<std::string> {
    if (code.empty())
        return {};

    auto result = std::vector<std::string> {};
    for (const auto instruction : code) {
        const u8 mnemonic = static_cast<u8>((instruction >> 12) & 0xF);
        const auto& mnemonic_string = instruction_to_string[mnemonic];

        if (array_contains(no_reg_instructions, static_cast<InstructionType>(mnemonic))) {
            result.push_back(mnemonic_string);
            continue;
        }

        if (array_contains(single_reg_instructions, static_cast<InstructionType>(mnemonic))) {
            const auto r1 = (instruction >> 8) & 0xF;
            result.emplace_back(fmt::format("{} r{}", mnemonic_string, r1));
            continue;
        }

        if (array_contains(double_reg_instructions, static_cast<InstructionType>(mnemonic))) {
            const auto r1 = (instruction >> 8) & 0xF;
            const auto r2 = (instruction >> 4) & 0xF;
            result.emplace_back(fmt::format("{} r{}, r{}", mnemonic_string, r1, r2));
            continue;
        }

        if (array_contains(triple_reg_instructions, static_cast<InstructionType>(mnemonic))) {
            const auto r1 = (instruction >> 8) & 0xF;
            const auto r2 = (instruction >> 4) & 0xF;
            const auto r3 = (instruction >> 0) & 0xF;
            result.emplace_back(fmt::format("{} r{}, r{}, r{}", mnemonic_string, r1, r2, r3));
            continue;
        }

        if (array_contains(regimm_instructions, static_cast<InstructionType>(mnemonic))) {
            const auto r1 = (instruction >> 8) & 0xF;
            const auto imm = instruction & 0xFF;
            result.emplace_back(fmt::format("{} r{}, #{}", mnemonic_string, r1, imm));
            continue;
        }
    }

    return result;
}
