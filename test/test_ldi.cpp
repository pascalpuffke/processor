#include <gtest/gtest.h>
#include <processor.hpp>

TEST(Processor, LoadImmediate) {
    auto run = [](u8 reg, u8 num) {
        auto processor = Processor {};

        auto ins = encode_instruction(
            InstructionType::LoadFromImm,
            Register { reg },
            Immediate { num }
        );
        processor.write_instruction(ProcessorSpec::reset_pc, ins);

        EXPECT_TRUE(processor.execute(1));

        return processor.registers()[reg];
    };

    EXPECT_EQ(run(0, 69), 69);
    EXPECT_EQ(run(1, 42), 42);
    EXPECT_EQ(run(2, 13), 13);
    EXPECT_EQ(run(3, 37), 37);
}

TEST(Processor, LoadImmediateFlags) {
    auto processor = Processor {};

    auto ins1 = encode_instruction(
        InstructionType::LoadFromImm,
        Register { 1 },
        Immediate { 0 }
    );
    processor.write_instruction(ProcessorSpec::reset_pc, ins1);

    EXPECT_TRUE(processor.execute(1));
    EXPECT_TRUE(processor.is_flag_set(Processor::Flag::Zero));
    EXPECT_FALSE(processor.is_flag_set(Processor::Flag::Negative));

    auto ins2 = encode_instruction(
        InstructionType::LoadFromImm,
        Register { 1 },
        Immediate { 0b1000'0000 }
    );
    processor.write_instruction(ProcessorSpec::reset_pc + 2, ins2);

    EXPECT_TRUE(processor.execute(1));
    EXPECT_FALSE(processor.is_flag_set(Processor::Flag::Zero));
    EXPECT_TRUE(processor.is_flag_set(Processor::Flag::Negative));
}

TEST(Processor, LoadIntoInvalidRegister) {
    auto processor = Processor {};

    auto ins = encode_instruction(
        InstructionType::LoadFromImm,
        Register { 10 },
        Immediate { 1 }
    );
    processor.write_instruction(ProcessorSpec::reset_pc, ins);

    EXPECT_FALSE(processor.execute(1));
}