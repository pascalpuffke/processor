#include <gtest/gtest.h>
#include <processor.hpp>

TEST(Processor, Store) {
    auto processor = Processor {};

    u16 target_addr = 0x1234;
    u8 target_data = 42;
    u8 low_byte_reg = 0;
    u8 high_byte_reg = 1;
    u8 data_reg = 2;

    processor.write_register(low_byte_reg, (target_addr >> 8));
    processor.write_register(high_byte_reg, target_addr);
    processor.write_register(data_reg, target_data);

    auto ins = Processor::encode_instruction(
        Processor::InstructionType::Store,
        Register { low_byte_reg },
        Register { high_byte_reg },
        Register { data_reg }
    );
    processor.write_instruction(processor.reset_pc, ins);

    EXPECT_TRUE(processor.execute(1));

    u8 actual_data = processor.read_memory(target_addr);
    EXPECT_EQ(actual_data, target_data);
}