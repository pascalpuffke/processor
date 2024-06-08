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
    auto processor = Processor {};

    EXPECT_EQ(processor.stack_pointer(), ProcessorSpec::stack_top_addr);

    // fill memory with push instructions and let it burn down
    auto ins = encode_instruction(InstructionType::Push, Register { 0 });
    for (int i = 0; i < 0xFFFF; i += 2) {
        processor.write_instruction(i, ins);
    }

    EXPECT_FALSE(processor.execute());
}