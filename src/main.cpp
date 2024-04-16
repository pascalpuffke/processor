#include <assembler.hpp>
#include <processor.hpp>

auto main(int, char**) -> int {
    auto processor = Processor {};

    const auto source = std::string { R"(
        ldi r0 #10
        ldi r1, #1
        ldi r7, #255
        ldi r3, #16
        ldi r5, #10
        sub r0, r0, r1
        jz r7, r3
        jp r7, r5
        done
    )" };
    const auto code = Assembler::assemble(source);

    constexpr auto start = Processor::reset_pc;
    for (auto i = 0; i < code.size(); i++) {
        const auto instruction = code[i];
        const auto location = start + (i * 2);
        processor.write_instruction(location, instruction);
    }

    for (auto i = 0; i < 100; i++) {
        if (!processor.execute(1))
            break;

        fmt::println("{}", processor.registers());
    }

    return 0;
}
