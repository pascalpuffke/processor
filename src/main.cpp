#include <assembler.hpp>
#include <disassembler.hpp>
#include <processor.hpp>
#include <ranges>

auto main(int, char**) -> int {
    auto processor = Processor {};

    // this sucks and likely isn't correct
    const auto factorial = std::string { R"(
        ldi r0, #5
        ldi r1, #1
        ldi r2, #1
        ldi r7, #0xFF
        ldi r3, #0x0C
        ldi r5, #0x13
        mul r1, r1, r0
        sub r0, r0, r2
        jz r7, r5
        jp r7, r3
        done
    )" };

    const auto code = Assembler::assemble(factorial);
    const auto disasm = Disassembler::disassemble(std::span { code });
    fmt::println("{}", disasm);

    constexpr auto start = ProcessorSpec::reset_pc;
    for (const auto [i, instruction] : std::views::enumerate(code)) {
        const auto location = static_cast<addr_t>(start + (i * 2));
        processor.write_instruction(location, instruction);
    }

    while (true) {
        if (!processor.execute(1))
            break;

        fmt::println("{}", processor.registers());
    }

    processor.dump_state(true);

    return 0;
}
