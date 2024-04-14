#include <gtest/gtest.h>
#include <processor.hpp>

TEST(Processor, Stack) {
    auto processor = Processor {};

    EXPECT_EQ(processor.stack_pointer(), 0);

    processor.write_register(0, 100);

    auto ins1 = Processor::encode_instruction(Processor::InstructionType::Push, Register { 0 });
    processor.write_instruction(processor.reset_pc, ins1);

    EXPECT_TRUE(processor.execute(1));
    EXPECT_EQ(processor.stack_pointer(), 1);
    EXPECT_EQ(processor.read_memory(0), 100);

    auto ins2 = Processor::encode_instruction(Processor::InstructionType::Pop, Register { 1 });
    processor.write_instruction(processor.reset_pc + 2, ins2);

    EXPECT_TRUE(processor.execute(1));
    EXPECT_EQ(processor.registers()[1], processor.registers()[0]);
    EXPECT_EQ(processor.stack_pointer(), 0);
}

TEST(Processor, StackUnderflow) {
    auto processor = Processor {};

    EXPECT_EQ(processor.stack_pointer(), 0);

    auto ins = Processor::encode_instruction(Processor::InstructionType::Pop, Register { 0 });
    processor.write_instruction(processor.reset_pc, ins);

    EXPECT_FALSE(processor.execute(1));
    EXPECT_EQ(processor.stack_pointer(), 0);
}

TEST(Processor, StackOverflow) {
    auto processor = Processor {};

    EXPECT_EQ(processor.stack_pointer(), 0);

    // fill memory with push instructions and let it burn down
    auto ins = Processor::encode_instruction(Processor::InstructionType::Push, Register { 0 });
    for (int i = 0; i < ((0xFFFF - processor.stack_size()) / 2); i++) {
        processor.write_instruction(processor.reset_pc + i, ins);
    }

    EXPECT_FALSE(processor.execute());
}