#include <disassembler.hpp>

#include <array>

// Clang doesn't seem to like constexpr std::array
#ifdef __clang__
#define ARRAY_CONSTEXPR
#else
#define ARRAY_CONSTEXPR constexpr
#endif

static constexpr usize register_mask = 0xF;
static constexpr usize imm_mask = 0xFF;

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
    "hlt",
};

enum class InstructionGroup {
    NoReg,
    SingleReg,
    DoubleReg,
    TripleReg,
    RegImm,
    Unknown
};

constexpr auto get_instruction_group(InstructionType type) -> InstructionGroup {
    switch (type) {
    case InstructionType::Halt:
        return InstructionGroup::NoReg;
    case InstructionType::Push:
    case InstructionType::Pop:
        return InstructionGroup::SingleReg;
    case InstructionType::Jump:
    case InstructionType::JumpIfZero:
    case InstructionType::LoadFromReg:
        return InstructionGroup::DoubleReg;
    case InstructionType::Add:
    case InstructionType::And:
    case InstructionType::Div:
    case InstructionType::LoadFromMem:
    case InstructionType::Mul:
    case InstructionType::Or:
    case InstructionType::Store:
    case InstructionType::Sub:
    case InstructionType::Xor:
        return InstructionGroup::TripleReg;
    case InstructionType::LoadFromImm:
        return InstructionGroup::RegImm;
    default:
        return InstructionGroup::Unknown;
    }
}

bool validate_empty_registers(ProcessorSpec::insr_t instruction, i32 expected_registers) {
    static constexpr usize max_registers = 3;

    if (expected_registers > max_registers)
        return false;

    switch (expected_registers) {
    case 1:
        return (instruction & register_mask) == 0;
    case 2:
        return ((instruction >> 4) & register_mask) == 0 && (instruction & register_mask) == 0;
    case 3:
        return ((instruction >> 8) & register_mask) == 0 && ((instruction >> 4) & register_mask) == 0 && (instruction & register_mask) == 0;
    default:
        return true;
    }
}

auto Disassembler::disassemble(std::span<const ProcessorSpec::insr_t> code) -> std::vector<std::string> {
    std::vector<std::string> result;

    for (const auto instruction : code) {
        const auto mnemonic_index = static_cast<u8>((instruction >> 12) & register_mask);
        const auto& mnemonic = instruction_to_string[mnemonic_index];

        const auto instruction_type = static_cast<InstructionType>(mnemonic_index);
        const auto group = get_instruction_group(instruction_type);

        switch (group) {
        case InstructionGroup::NoReg:
            if (!validate_empty_registers(instruction, 3))
                fmt::println(stderr, "Instruction '{}' expected no registers, potentially corrupt!", mnemonic);
            else
                result.emplace_back(mnemonic);
            break;

        case InstructionGroup::SingleReg: {
            const auto r1 = (instruction >> 8) & register_mask;
            if (r1 >= ProcessorSpec::register_count)
                fmt::println(stderr, "Instruction '{}' addresses an invalid register", mnemonic);
            else if (!validate_empty_registers(instruction, 2))
                fmt::println(stderr, "Instruction '{}' expected 1 register, potentially corrupt!", mnemonic);
            else
                result.emplace_back(fmt::format("{} r{}", mnemonic, r1));
            break;
        }

        case InstructionGroup::DoubleReg: {
            const auto r1 = (instruction >> 8) & register_mask;
            const auto r2 = (instruction >> 4) & register_mask;
            if (r1 >= ProcessorSpec::register_count || r2 >= ProcessorSpec::register_count)
                fmt::println(stderr, "Instruction '{}' addresses an invalid register", mnemonic);
            else if (!validate_empty_registers(instruction, 1))
                fmt::println(stderr, "Instruction '{}' expected 2 registers, potentially corrupt!", mnemonic);
            else
                result.emplace_back(fmt::format("{} r{}, r{}", mnemonic, r1, r2));
            break;
        }

        case InstructionGroup::TripleReg: {
            const auto r1 = (instruction >> 8) & register_mask;
            const auto r2 = (instruction >> 4) & register_mask;
            const auto r3 = instruction & register_mask;
            if (r1 >= ProcessorSpec::register_count || r2 >= ProcessorSpec::register_count || r3 >= ProcessorSpec::register_count)
                fmt::println(stderr, "Instruction '{}' addresses an invalid register", mnemonic);
            else
                result.emplace_back(fmt::format("{} r{}, r{}, r{}", mnemonic, r1, r2, r3));
            break;
        }

        case InstructionGroup::RegImm: {
            const auto r1 = (instruction >> 8) & register_mask;
            const auto imm = instruction & imm_mask;
            if (r1 >= ProcessorSpec::register_count)
                fmt::println(stderr, "Instruction '{}' addresses an invalid register", mnemonic);
            else
                result.emplace_back(fmt::format("{} r{}, #{}", mnemonic, r1, imm));
            break;
        }

        default:
            fmt::println(stderr, "Encountered unknown instruction '{:b}'", instruction);
            break;
        }
    }

    return result;
}
