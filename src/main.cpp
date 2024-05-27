#include <assembler.hpp>
#include <disassembler.hpp>
#include <processor.hpp>

auto main(int, char**) -> int {
    auto processor = Processor {};

    const auto source = std::string { R"(
        ldi r0, #10
        ldi r1, #1
        ldi r7, #0xFF
        ldi r3, #0x10
        ldi r5, #0x0A
        sub r0, r0, r1
        jz r7, r3
        jp r7, r5
        done
    )" };
    const auto code = Assembler::assemble(source);
    const auto disasm = Disassembler::disassemble(std::span { code });
    fmt::println("{}", disasm);

    constexpr auto start = ProcessorSpec::reset_pc;
    for (usize i = 0; i < code.size(); i++) {
        const insr_t instruction = code[i];
        const addr_t location = static_cast<addr_t>(start + (i * 2));
        processor.write_instruction(location, instruction);
    }

    for (auto i = 0; i < 100; i++) {
        if (!processor.execute(1))
            break;

        fmt::println("{}", processor.registers());
    }

    return 0;
}
