#include <gtest/gtest.h>
#include <processor.hpp>

TEST(Processor, Stack) {
    auto processor = Processor {};

    EXPECT_EQ(processor.stack_pointer(), ProcessorSpec::stack_top_addr);

    processor.write_register(0, 100);

    auto ins1 = encode_instruction(InstructionType::Push, Register { 0 });
    processor.write_instruction(ProcessorSpec::reset_pc, ins1);

    EXPECT_TRUE(processor.execute(1));
    EXPECT_EQ(processor.stack_pointer(), ProcessorSpec::stack_top_addr - 1);
    EXPECT_EQ(processor.read_memory(ProcessorSpec::stack_top_addr - 1), 100);

    auto ins2 = encode_instruction(InstructionType::Pop, Register { 1 });
    processor.write_instruction(ProcessorSpec::reset_pc + 2, ins2);

    EXPECT_TRUE(processor.execute(1));
    EXPECT_EQ(processor.registers()[1], processor.registers()[0]);
    EXPECT_EQ(processor.stack_pointer(), ProcessorSpec::stack_top_addr);
}

TEST(Processor, StackUnderflow) {
    auto processor = Processor {};

    EXPECT_EQ(processor.stack_pointer(), ProcessorSpec::stack_top_addr);

    auto ins = encode_instruction(InstructionType::Pop, Register { 0 });
    processor.write_instruction(ProcessorSpec::reset_pc, ins);

    EXPECT_FALSE(processor.execute(1));
    EXPECT_EQ(processor.stack_pointer(), ProcessorSpec::stack_top_addr);
}

TEST(Processor, StackOverflow) {
    EXPECT_EQ(ProcessorSpec::reset_pc, 0xFF00);

    auto processor = Processor {};
    EXPECT_EQ(processor.stack_pointer(), ProcessorSpec::stack_top_addr);

    auto load_upper = encode_instruction(InstructionType::LoadFromImm, Register { 0 }, Immediate { 0xFF });
    auto load_lower = encode_instruction(InstructionType::LoadFromImm, Register { 1 }, Immediate { 0x04 });
    auto push = encode_instruction(InstructionType::Push, Register { 0 });
    auto jump = encode_instruction(InstructionType::Jump, Register { 0 }, Register { 1 });

    processor.write_instruction(ProcessorSpec::reset_pc, load_upper);
    processor.write_instruction(ProcessorSpec::reset_pc + 2, load_lower);
    processor.write_instruction(ProcessorSpec::reset_pc + 4, push);
    processor.write_instruction(ProcessorSpec::reset_pc + 6, jump);

    EXPECT_FALSE(processor.execute());
}
