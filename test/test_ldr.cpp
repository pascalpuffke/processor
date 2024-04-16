#include <gtest/gtest.h>
#include <processor.hpp>

TEST(Processor, LoadRegister) {
    auto processor = Processor {};
    u8 expected = 0xA0;
    processor.write_register(0, 0);
    processor.write_register(7, expected);

    auto ins = encode_instruction(
        InstructionType::LoadFromReg,
        Register { 0 },
        Register { 7 }
    );
    processor.write_instruction(processor.reset_pc, ins);

    EXPECT_TRUE(processor.execute(1));
    u8 actual = processor.registers()[0];
    EXPECT_EQ(expected, actual);
}

TEST(Processor, LoadRegisterFlags) {
    {
        auto processor = Processor {};
        u8 expected = 0xFF;
        processor.write_register(0, 0);
        processor.write_register(7, expected);

        auto ins = encode_instruction(
            InstructionType::LoadFromReg,
            Register { 0 },
            Register { 7 }
        );
        processor.write_instruction(processor.reset_pc, ins);

        EXPECT_TRUE(processor.execute(1));
        u8 actual = processor.registers()[0];
        EXPECT_EQ(expected, actual);
        EXPECT_FALSE(processor.is_flag_set(Processor::Flag::Zero));
        EXPECT_TRUE(processor.is_flag_set(Processor::Flag::Negative));
    }

    {
        auto processor = Processor {};
        u8 expected = 0;
        processor.write_register(0, 0);
        processor.write_register(7, 0);

        auto ins = encode_instruction(
            InstructionType::LoadFromReg,
            Register { 0 },
            Register { 7 }
        );
        processor.write_instruction(processor.reset_pc, ins);

        EXPECT_TRUE(processor.execute(1));
        u8 actual = processor.registers()[0];
        EXPECT_EQ(expected, actual);
        EXPECT_TRUE(processor.is_flag_set(Processor::Flag::Zero));
        EXPECT_FALSE(processor.is_flag_set(Processor::Flag::Negative));
    }
}