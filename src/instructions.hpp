#pragma once

#include <utility>

#include <spec.hpp>
#include <register.hpp>
#include <immediate.hpp>

// clang-format off
enum class InstructionType : u8 {
    LoadFromReg = 0b0000,
    Store       = 0b0001,
    Add         = 0b0010,
    Sub         = 0b0011,
    Mul         = 0b0100,
    Div         = 0b0101,
    Jump        = 0b0110,
    JumpIfZero  = 0b0111,
    And         = 0b1000,
    Or          = 0b1001,
    Xor         = 0b1010,
    Push        = 0b1011,
    Pop         = 0b1100,
    LoadFromImm = 0b1101,
    LoadFromMem = 0b1110,
    Done        = 0b1111,
};
// clang-format on

// type|0000|0000|0000
static constexpr auto encode_instruction(InstructionType type) -> ProcessorSpec::insr_t {
    return static_cast<ProcessorSpec::insr_t>(std::to_underlying(type) << 12);
}

// type|00R1|0000|0000
static constexpr auto encode_instruction(InstructionType type, Register r1) -> ProcessorSpec::insr_t {
    return static_cast<ProcessorSpec::insr_t>(std::to_underlying(type) << 12 | r1 << 8);
}

// type|00R1|00R2|0000
static constexpr auto encode_instruction(InstructionType type, Register r1, Register r2) -> ProcessorSpec::insr_t {
    return static_cast<ProcessorSpec::insr_t>(std::to_underlying(type) << 12 | r1 << 8 | r2 << 4);
}

// type|00R1|00R2|00R3
static constexpr auto encode_instruction(InstructionType type, Register r1, Register r2, Register r3) -> ProcessorSpec::insr_t {
    return static_cast<ProcessorSpec::insr_t>(std::to_underlying(type) << 12 | r1 << 8 | r2 << 4 | (r3 & 0xF));
}

// type|00R1|immm|immm
static constexpr auto encode_instruction(InstructionType type, Register r1, Immediate imm) -> ProcessorSpec::insr_t {
    return static_cast<ProcessorSpec::insr_t>(std::to_underlying(type) << 12 | r1 << 8 | imm);
}
