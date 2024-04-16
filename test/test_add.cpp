#include <gtest/gtest.h>
#include <processor.hpp>

TEST(Processor, Add) {
    auto processor = Processor {};
    processor.write_register(2, 1);
    processor.write_register(3, 2);

    auto ins = encode_instruction(
        InstructionType::Add,
        Register { 1 },
        Register { 2 },
        Register { 3 }
    );
    processor.write_instruction(processor.reset_pc, ins);

    EXPECT_TRUE(processor.execute(1));
    EXPECT_EQ(processor.registers()[1], 3);
}

TEST(Processor, AddFlags) {
    auto processor = Processor {};

    auto ins1 = encode_instruction(
        InstructionType::Add,
        Register { 1 },
        Register { 2 },
        Register { 3 }
    );
    processor.write_instruction(processor.reset_pc, ins1);

    EXPECT_TRUE(processor.execute(1));
    EXPECT_EQ(processor.registers()[1], 0);
    EXPECT_TRUE(processor.is_flag_set(Processor::Flag::Zero));

    processor.write_register(2, 128);
    processor.write_register(3, 129);

    auto ins2 = encode_instruction(
        InstructionType::Add,
        Register { 1 },
        Register { 2 },
        Register { 3 }
    );
    processor.write_instruction(processor.reset_pc + 2, ins1);

    EXPECT_TRUE(processor.execute(1));
    EXPECT_EQ(processor.registers()[1], 1);
    EXPECT_FALSE(processor.is_flag_set(Processor::Flag::Zero));
    EXPECT_TRUE(processor.is_flag_set(Processor::Flag::Overflow));

    processor.write_register(2, 128);
    processor.write_register(3, 2);

    auto ins3 = encode_instruction(
        InstructionType::Add,
        Register { 1 },
        Register { 2 },
        Register { 3 }
    );
    processor.write_instruction(processor.reset_pc + 4, ins1);

    EXPECT_TRUE(processor.execute(1));
    EXPECT_EQ(processor.registers()[1], 130);
    EXPECT_FALSE(processor.is_flag_set(Processor::Flag::Zero));
    EXPECT_FALSE(processor.is_flag_set(Processor::Flag::Overflow));
    EXPECT_TRUE(processor.is_flag_set(Processor::Flag::Negative));
}
