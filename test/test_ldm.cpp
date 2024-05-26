#include <gtest/gtest.h>
#include <processor.hpp>

TEST(Processor, LoadFromMemory) {
    auto processor = Processor {};

    auto target_addr = 0x1234;
    auto target_data = 0x56;

    processor.write_memory(target_addr, target_data);
    EXPECT_EQ(processor.read_memory(target_addr), target_data);

    auto dst = Register { 0 };
    auto low = Register { 1 };
    auto high = Register { 2 };

    processor.write_register(low, target_addr >> 8);
    processor.write_register(high, target_addr);

    auto ins = encode_instruction(
        InstructionType::LoadFromMem,
        dst,
        low,
        high
    );
    processor.write_instruction(ProcessorSpec::reset_pc, ins);

    EXPECT_TRUE(processor.execute(1));

    auto actual_data = processor.registers()[dst];
    EXPECT_EQ(actual_data, target_data);
    EXPECT_FALSE(processor.is_flag_set(Processor::Flag::Zero));
    EXPECT_FALSE(processor.is_flag_set(Processor::Flag::Negative));
}