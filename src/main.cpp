#include <processor.hpp>

auto main(int, char**) -> int {
    auto processor = Processor {};

    // Count down from 10 to 0.
    // FF00 ldi r0, #10
    // FF02 ldi r1, #1
    // FF04 ldi r7, #255
    // FF06 ldi r3, #16
    // FF08 ldi r5, #10
    // FF0A sub r0, r0, r1
    // FF0C jz r7, r3
    // FF0E jp r7, r5
    // FF10 done

    // FF00
    auto a = Processor::encode_instruction(Processor::InstructionType::LoadFromImm, Register { 0 }, Immediate { 10 });
    // FF02
    auto b = Processor::encode_instruction(Processor::InstructionType::LoadFromImm, Register { 1 }, Immediate { 1 });
    // FF04
    auto c = Processor::encode_instruction(Processor::InstructionType::LoadFromImm, Register { 7 }, Immediate { 0xFF });
    // FF06
    auto d = Processor::encode_instruction(
        Processor::InstructionType::LoadFromImm,
        Register { 3 },
        Immediate { 0x10 }
    );
    // FF08
    auto e = Processor::encode_instruction(
        Processor::InstructionType::LoadFromImm,
        Register { 5 },
        Immediate { 0x0A }
    );
    // FF0A
    auto f = Processor::encode_instruction(Processor::InstructionType::Sub, Register { 0 }, Register { 0 }, Register { 1 });
    // FF0C
    auto g = Processor::encode_instruction(Processor::InstructionType::JumpIfZero, Register { 7 }, Register { 3 });
    // FF0E
    auto h = Processor::encode_instruction(Processor::InstructionType::Jump, Register { 7 }, Register { 5 });
    // FF10
    auto end = Processor::encode_instruction(Processor::InstructionType::Done);

    const auto start = processor.reset_pc;
    processor.write_instruction(start + 0, a);
    processor.write_instruction(start + 2, b);
    processor.write_instruction(start + 4, c);
    processor.write_instruction(start + 6, d);
    processor.write_instruction(start + 8, e);
    processor.write_instruction(start + 10, f);
    processor.write_instruction(start + 12, g);
    processor.write_instruction(start + 14, h);
    processor.write_instruction(start + 16, end);

    for (auto i = 0; i < 100; i++) {
        auto result = processor.execute(1);
        if (!result)
            break;

        fmt::println("{}", processor.registers());
    }

    return 0;
}