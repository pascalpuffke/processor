#include <assembler.hpp>
#include <cassert>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <optional>
#include <sstream>
#include <unordered_map>

auto split(const std::string& str, char delimiter) -> std::vector<std::string> {
    std::vector<std::string> tokens;
    std::istringstream stream(str.data());
    std::string token;
    while (std::getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

enum class InstructionArguments {
    None,
    SingleReg,
    DoubleReg,
    TripleReg,
    RegImm,
};

struct InstructionDefinition {
    InstructionType type;
    InstructionArguments args;
};

static std::unordered_map<std::string, InstructionDefinition> instruction_map = {
    { "add", { .type = InstructionType::Add, .args = InstructionArguments::TripleReg } },
    { "and", { .type = InstructionType::And, .args = InstructionArguments::TripleReg } },
    { "div", { .type = InstructionType::Div, .args = InstructionArguments::TripleReg } },
    { "done", { .type = InstructionType::Done, .args = InstructionArguments::None } },
    { "jp", { .type = InstructionType::Jump, .args = InstructionArguments::DoubleReg } },
    { "jz", { .type = InstructionType::JumpIfZero, .args = InstructionArguments::DoubleReg } },
    { "ldi", { .type = InstructionType::LoadFromImm, .args = InstructionArguments::RegImm } },
    { "ldm", { .type = InstructionType::LoadFromMem, .args = InstructionArguments::TripleReg } },
    { "ldr", { .type = InstructionType::LoadFromReg, .args = InstructionArguments::DoubleReg } },
    { "mul", { .type = InstructionType::Mul, .args = InstructionArguments::TripleReg } },
    { "or", { .type = InstructionType::Or, .args = InstructionArguments::TripleReg } },
    { "pop", { .type = InstructionType::Pop, .args = InstructionArguments::SingleReg } },
    { "push", { .type = InstructionType::Push, .args = InstructionArguments::SingleReg } },
    { "st", { .type = InstructionType::Store, .args = InstructionArguments::TripleReg } },
    { "sub", { .type = InstructionType::Sub, .args = InstructionArguments::TripleReg } },
    { "xor", { .type = InstructionType::Xor, .args = InstructionArguments::TripleReg } },
};

// 'r1' and 'r1,' -> 1
auto parse_register(std::string_view token) -> std::optional<Register> {
    if (!token.starts_with('r'))
        return std::nullopt;
    // 2 for a format like 'r1'. 3 when it ends with a comma.
    if (token.length() != 2 && token.length() != 3)
        return std::nullopt;

    const auto register_char = token[1];
    if (!std::isdigit(register_char))
        return std::nullopt;

    const auto register_index = register_char - '0';
    if (register_index < 0 || register_index > 9)
        return std::nullopt;

    return Register { static_cast<Register::reg_t>(register_index) };
}

// '#123' '#0x7B' -> 123
auto parse_immediate(std::string_view token) -> std::optional<Immediate> {
    if (!token.starts_with('#')) {
        fmt::println("immediate needs to start with a '#' character");
        return std::nullopt;
    }

    const auto is_hex = token.starts_with("#0x");
    // Subtract 1 from the substr length for the leading # and another one if
    // the token ends with a comma
    auto subtract = 1 + (token.ends_with(',') ? 1 : 0);

    auto immediate = 0;
    if (is_hex) {
        // get rid of the 0x prefix
        subtract += 2;

        const auto substr = token.substr(3, token.length() - subtract);
        immediate = std::stoi(substr.data(), nullptr, 16);
    } else {
        const auto substr = token.substr(1, token.length() - subtract);
        immediate = std::stoi(substr.data(), nullptr, 10);
    }

    static constexpr auto min = std::numeric_limits<Immediate::imm_t>::min();
    static constexpr auto max = std::numeric_limits<Immediate::imm_t>::max();
    if (immediate < min) {
        fmt::println("immediate is too small (min {})", min);
        return std::nullopt;
    }
    if (immediate > max) {
        fmt::println("immediate is too big (max {})", max);
        return std::nullopt;
    }

    return Immediate { static_cast<Immediate::imm_t>(immediate) };
}

auto strip(const std::string& str) -> std::string {
    const auto first = str.find_first_not_of(" \t");
    const auto last = str.find_last_not_of(" \t");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return str.substr(first, last - first + 1);
}

auto Assembler::assemble(const std::string& source) -> std::vector<insr_t> {
    if (source.empty())
        return {};

    auto lines = split(source, '\n');

    auto instructions = std::vector<insr_t> {};
    for (auto& line : lines) {
        line = strip(line);

        auto tokens = split(line, ' ');
        if (tokens.empty())
            continue;

        auto mnemonic_it = instruction_map.find(tokens[0]);
        if (mnemonic_it == instruction_map.end()) {
            fmt::println("illegal instruction (unknown mnemonic)");
            return {};
        }

        // extremely unhelpful, please improve error handling sometime
#define MUST_PARSE(fn) ({                                    \
    auto __result = fn;                                      \
    if (!__result.has_value()) {                             \
        fmt::println("error parsing register or immediate"); \
        return {};                                           \
    }                                                        \
    __result.value();                                        \
})

        const auto type = mnemonic_it->second.type;
        switch (tokens.size()) {
        case 1: {
            instructions.emplace_back(encode_instruction(type));
            break;
        }
        case 2: {
            const auto reg = MUST_PARSE(parse_register(tokens[1]));
            instructions.emplace_back(encode_instruction(type, reg));
            break;
        }
        case 3: {
            if (mnemonic_it->second.args == InstructionArguments::RegImm) {
                const auto reg = MUST_PARSE(parse_register(tokens[1]));
                const auto imm = MUST_PARSE(parse_immediate(tokens[2]));
                instructions.emplace_back(encode_instruction(type, reg, imm));
            } else {
                const auto r1 = MUST_PARSE(parse_register(tokens[1]));
                const auto r2 = MUST_PARSE(parse_register(tokens[2]));
                instructions.emplace_back(encode_instruction(type, r1, r2));
            }
            break;
        }
        case 4: {
            const auto r1 = MUST_PARSE(parse_register(tokens[1]));
            const auto r2 = MUST_PARSE(parse_register(tokens[2]));
            const auto r3 = MUST_PARSE(parse_register(tokens[3]));
            instructions.emplace_back(encode_instruction(type, r1, r2, r3));
            break;
        }
        default:
            fmt::println("illegal instruction (too many arguments)");
            return {};
        }
    }
#undef MUST_PARSE

    return instructions;
}
