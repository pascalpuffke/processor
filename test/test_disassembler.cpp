#include <disassembler.hpp>
#include <gtest/gtest.h>

TEST(Disassembler, DiassembleValidProgram) {
    const auto code = std::vector<insr_t> {
        (std::to_underlying(InstructionType::Add) << 12) | 0 << 8 | 1 << 4 | 2,
        (std::to_underlying(InstructionType::And) << 12) | 1 << 8 | 2 << 4 | 3,
        (std::to_underlying(InstructionType::Div) << 12) | 2 << 8 | 3 << 4 | 4,
        (std::to_underlying(InstructionType::Done) << 12),
        (std::to_underlying(InstructionType::Jump) << 12) | 5 << 8 | 6 << 4,
        (std::to_underlying(InstructionType::JumpIfZero) << 12) | 6 << 8 | 7 << 4,
        (std::to_underlying(InstructionType::LoadFromImm) << 12) | 0 << 8 | 1,
        (std::to_underlying(InstructionType::LoadFromImm) << 12) | 0 << 8 | 0xFF,
        (std::to_underlying(InstructionType::LoadFromMem) << 12) | 0 << 8 | 6 << 4 | 7,
        (std::to_underlying(InstructionType::LoadFromReg) << 12) | 2 << 8 | 3 << 4,
        (std::to_underlying(InstructionType::Mul) << 12) | 2 << 8 | 4 << 4 | 6,
        (std::to_underlying(InstructionType::Or) << 12) | 2 << 8 | 4 << 4 | 6,
        (std::to_underlying(InstructionType::Pop) << 12),
        (std::to_underlying(InstructionType::Push) << 12),
        (std::to_underlying(InstructionType::Store) << 12) | 2 << 8 | 4 << 4 | 6,
        (std::to_underlying(InstructionType::Sub) << 12) | 2 << 8 | 4 << 4 | 6,
        (std::to_underlying(InstructionType::Xor) << 12 | 2 << 8 | 4 << 4 | 6)
    };

    const auto expected_result = std::vector<std::string> {
        "add r0, r1, r2",
        "and r1, r2, r3",
        "div r2, r3, r4",
        "done",
        "jp r5, r6",
        "jz r6, r7",
        "ldi r0, #1",
        "ldi r0, #255",
        "ldm r0, r6, r7",
        "ldr r2, r3",
        "mul r2, r4, r6",
        "or r2, r4, r6",
        "pop r0",
        "push r0",
        "st r2, r4, r6",
        "sub r2, r4, r6",
        "xor r2, r4, r6",
    };
    const auto result = Disassembler::disassemble(std::span { code });

    EXPECT_TRUE(!result.empty());
    EXPECT_EQ(expected_result, result);
}