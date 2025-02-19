#include <array>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <span>
#include <stack>
#include <utility>

#include <instructions.hpp>

/*
 * ldr    <dst>,<src>       load value at reg src into reg dst
 * ldi    <dst>,#imm        load immediate into reg dst
 * ldm    <dst>,<lb>,<hb>   load value from memory at lb<<8|hb into reg dst
 * st     <lb>,<hb>,<src>   store value from reg src into memory at lb<<8|hb
 * push   <src>             push value of reg to top of stack
 * pop    <dst>             pop value from bottom of stack to reg
 * add    <dst>,<lhs>,<rhs> put result of lhs+rhs into reg dst
 * sub    <dst>,<lhs>,<rhs> put result of lhs-rhs into reg dst
 * mul    <dst>,<lhs>,<rhs> put result of lhs*rhs into reg dst
 * div    <dst>,<lhs>,<rhs> put result of lhs/rhs into reg dst
 * jp     <lb>,<hb>         jump to addr at lb<<8|hb unconditionally
 * jz     <lb>,<hb>         jump to addr at lb<<8|hb if zero flag set
 * and    <dst>,<lhs>,<rhs>
 *  or    <dst>,<lhs>,<rhs>
 * xor    <dst>,<lhs>,<rhs>
 * hlt                      set 'halt' flag, stopping execution on the next cycle
 */

// Leak all of those out to whoever includes this file, I don't care anymore
using ProcessorSpec::reg_t;
using ProcessorSpec::imm_t;
using ProcessorSpec::data_t;
using ProcessorSpec::insr_t;
using ProcessorSpec::addr_t;

class Processor {
public:
    enum class Flag : u8 {
        Carry = 0b00001,
        Zero = 0b00010,
        Overflow = 0b00100,
        Negative = 0b01000,
        Halt = 0b10000,
    };

    constexpr auto reset() {
        m_memory.fill(0);
        m_registers = { 0 };
        m_program_counter = ProcessorSpec::reset_pc;
        m_stack_pointer = ProcessorSpec::stack_top_addr;
        m_flags = 0;
    }

    constexpr Processor() {
        reset();
    }

    [[nodiscard]] constexpr auto read_memory(addr_t address) const {
        return m_memory[address];
    }

    constexpr auto write_memory(addr_t address, data_t data) {
        m_memory[address] = data;
    }

    constexpr auto write_instruction(addr_t start_address, insr_t encoded_instruction) {
        write_memory(start_address, static_cast<data_t>((encoded_instruction >> 8) & 0xFF));
        write_memory(start_address + 1, static_cast<data_t>(encoded_instruction & 0xFF));
    }

    [[nodiscard]] constexpr auto registers() const noexcept {
        return std::span { m_registers };
    }

    [[nodiscard]] constexpr auto program_counter() const noexcept {
        return m_program_counter;
    }

    [[nodiscard]] constexpr auto stack_pointer() const noexcept {
        return m_stack_pointer;
    }

    [[nodiscard]] constexpr auto flags() const noexcept {
        return m_flags;
    }

    [[nodiscard]] constexpr auto is_flag_set(Flag flag) const noexcept -> bool {
        return m_flags & std::to_underlying(flag);
    }

    constexpr auto set_flag(Flag flag) -> void {
        m_flags |= std::to_underlying(flag);
    }

    constexpr auto unset_flag(Flag flag) -> void {
        m_flags &= ~std::to_underlying(flag);
    }

    [[nodiscard]] constexpr auto flag_string() const -> std::string {
        auto string = std::string {};
        string.append("...");

        if (is_flag_set(Flag::Negative))
            string.push_back('N');
        else
            string.push_back('.');

        if (is_flag_set(Flag::Overflow))
            string.push_back('O');
        else
            string.push_back('.');

        if (is_flag_set(Flag::Zero))
            string.push_back('Z');
        else
            string.push_back('.');

        if (is_flag_set(Flag::Carry))
            string.push_back('C');
        else
            string.push_back('.');

        if (is_flag_set(Flag::Halt))
            string.push_back('H');
        else
            string.push_back('.');

        return string;
    }

    constexpr auto dump_memory(usize width = 8) const noexcept {
        for (usize x = 0; x < m_memory.size(); x += width) {
            fmt::print("{:4X}: ", x);
            for (usize xx = 0; xx < width; xx++) {
                auto byte = m_memory[x + xx];
                fmt::print("{:2X} ", byte);
            }
            fmt::print("\n");
        }
    }

    constexpr auto dump_state(bool with_memory = false) const noexcept {
        if (with_memory) {
            dump_memory(48);
        } else {
            fmt::println("memory=<size 0x{:X} bytes>", m_memory.size());
        }

        fmt::println(
            "registers={}\npc=0x{:X}\nsp=0x{:X}\nflags={} [0b{:b}]",
            m_registers,
            m_program_counter,
            m_stack_pointer,
            flag_string(),
            m_flags
        );
    }

    struct DecodedInstruction {
        InstructionType type;
        // First register, commonly used as a destination
        Register r1;
        // Second register, commonly used as a source or left-hand side op
        Register r2;
        // Third register, commonly used as right-hand side op
        Register r3;
        // 8-bit data for instructions taking immediate values
        data_t data;
    };

    constexpr auto execute(usize instruction_count = std::numeric_limits<usize>::max()) -> bool {
        for (usize i = 0; i < instruction_count; i++) {
            if (is_flag_set(Flag::Halt)) {
                fmt::println("halt flag set");
                return false;
            }

            if (m_program_counter == std::numeric_limits<decltype(m_program_counter)>::max()) {
                fmt::println("reached end of memory");
                return false;
            }

            const auto instruction = fetch_instruction(m_program_counter);
            const auto decoded = decode_instruction(instruction);
            const auto executed = execute_instruction(decoded);

            if (!executed) {
                fmt::println("instruction @ pc=0x{:X} failed to execute", m_program_counter);
                fmt::println(
                    "type={} r1={} r2={} r3={} data={}",
                    std::to_underlying(decoded.type),
                    decoded.r1,
                    decoded.r2,
                    decoded.r3,
                    decoded.data
                );
                return false;
            }

            m_program_counter += sizeof(ProcessorSpec::insr_t);
        }

        return true;
    }

    constexpr auto write_register(u8 reg, data_t data) {
        if (reg < ProcessorSpec::register_count)
            m_registers[reg] = data;
    }

    [[nodiscard]] constexpr auto fetch_instruction(addr_t addr) const -> insr_t {
        static_assert(sizeof(insr_t) == 2);

        const auto first_byte = read_memory(addr);
        const auto second_byte = read_memory(addr + 1);
        const auto instruction = static_cast<insr_t const>((first_byte << 8) | second_byte);
        return instruction;
    }

    constexpr auto decode_instruction(insr_t instruction) -> DecodedInstruction {
        if (instruction == 0)
            set_flag(Flag::Halt);

        // Always parse out all fields, no matter what the instruction actually requires.
        return DecodedInstruction {
            .type = static_cast<InstructionType>(instruction >> 12),
            .r1 = static_cast<decltype(DecodedInstruction::r1)>((instruction >> 8) & 0xF),
            .r2 = static_cast<decltype(DecodedInstruction::r2)>((instruction >> 4) & 0xF),
            .r3 = static_cast<decltype(DecodedInstruction::r3)>((instruction >> 0) & 0xF),
            .data = static_cast<decltype(DecodedInstruction::data)>(instruction & 0xFF)
        };
    }

    constexpr auto set_zn_flags(data_t value) -> void {
        if (value == 0)
            set_flag(Flag::Zero);
        else
            unset_flag(Flag::Zero);
        if (value & 0b1000'0000)
            set_flag(Flag::Negative);
        else
            unset_flag(Flag::Negative);
    }

    constexpr auto execute_instruction(const DecodedInstruction& instruction) -> bool {
        const auto r1 = instruction.r1.reg;
        const auto r2 = instruction.r2.reg;
        const auto r3 = instruction.r3.reg;

        fmt::print("0x{:X} ", m_program_counter);
        switch (instruction.type) {
        case InstructionType::LoadFromReg: {
            fmt::println("ldr r{}, r{}", r1, r2);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            if (r2 >= ProcessorSpec::register_count)
                return false;
            auto& dst = m_registers[r1];
            auto& src = m_registers[r2];

            dst = src;

            set_zn_flags(dst);
        } break;
        case InstructionType::LoadFromImm: {
            fmt::println("ldi r{}, #{}", r1, instruction.data);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            auto& dst = m_registers[r1];

            dst = instruction.data;

            set_zn_flags(dst);
        } break;
        case InstructionType::LoadFromMem: {
            fmt::println("ldm r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            if (r2 >= ProcessorSpec::register_count)
                return false;
            if (r3 >= ProcessorSpec::register_count)
                return false;

            auto& dst = m_registers[r1];
            auto& high_reg = m_registers[r2];
            auto& low_reg = m_registers[r3];
            auto address = static_cast<addr_t>((high_reg << 8) | low_reg);

            dst = read_memory(address);

            set_zn_flags(dst);
        } break;
        case InstructionType::Store: {
            fmt::println("st r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            if (r2 >= ProcessorSpec::register_count)
                return false;
            if (r3 >= ProcessorSpec::register_count)
                return false;

            auto& high_reg = m_registers[r1];
            auto& low_reg = m_registers[r2];
            auto& src = m_registers[r3];
            auto address = static_cast<addr_t>((high_reg << 8) | low_reg);

            write_memory(address, src);

            set_zn_flags(src);
        } break;
        case InstructionType::Add: {
            fmt::println("add r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            if (r2 >= ProcessorSpec::register_count)
                return false;
            if (r3 >= ProcessorSpec::register_count)
                return false;
            auto& dst = m_registers[r1];
            auto& lhs = m_registers[r2];
            auto& rhs = m_registers[r3];

            dst = lhs + rhs;

            if ((static_cast<usize>(lhs) + rhs) > std::numeric_limits<data_t>::max())
                set_flag(Flag::Overflow);
            else
                unset_flag(Flag::Overflow);

            set_zn_flags(dst);
        } break;
        case InstructionType::Sub: {
            fmt::println("sub r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            if (r2 >= ProcessorSpec::register_count)
                return false;
            if (r3 >= ProcessorSpec::register_count)
                return false;
            auto& dst = m_registers[r1];
            auto& lhs = m_registers[r2];
            auto& rhs = m_registers[r3];

            dst = lhs - rhs;

            if (rhs > lhs)
                set_flag(Flag::Carry);
            else
                unset_flag(Flag::Carry);

            set_zn_flags(dst);
        } break;
        case InstructionType::Mul: {
            fmt::println("mul r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            if (r2 >= ProcessorSpec::register_count)
                return false;
            if (r3 >= ProcessorSpec::register_count)
                return false;
            auto& dst = m_registers[r1];
            auto& lhs = m_registers[r2];
            auto& rhs = m_registers[r3];

            dst = lhs * rhs;

            if ((static_cast<usize>(lhs) * rhs) > std::numeric_limits<data_t>::max())
                set_flag(Flag::Overflow);
            else
                unset_flag(Flag::Overflow);

            set_zn_flags(dst);
        } break;
        case InstructionType::Div: {
            fmt::println("div r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            if (r2 >= ProcessorSpec::register_count)
                return false;
            if (r3 >= ProcessorSpec::register_count)
                return false;
            auto& dst = m_registers[r1];
            auto& lhs = m_registers[r2];
            auto& rhs = m_registers[r3];

            if (rhs == 0) {
                fmt::println("division by zero at instruction $0x{:X}", m_program_counter);
                return false;
            } else {
                dst = lhs / rhs;
            }

            set_zn_flags(dst);
        } break;
        case InstructionType::Jump: {
            fmt::println("jp r{}, r{}", r1, r2);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            if (r2 >= ProcessorSpec::register_count)
                return false;

            auto& high_reg = m_registers[r1];
            auto& low_reg = m_registers[r2];
            auto address = static_cast<addr_t>((high_reg << 8) | low_reg);

	    // Point to the previous instruction, so execution begins at the
	    // correct address
            m_program_counter = address - sizeof(insr_t);
        } break;
        case InstructionType::JumpIfZero: {
            fmt::println("jz r{}, r{}", r1, r2);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            if (r2 >= ProcessorSpec::register_count)
                return false;

            auto& high_reg = m_registers[r1];
            auto& low_reg = m_registers[r2];
            auto address = static_cast<addr_t>((high_reg << 8) | low_reg);

            if (is_flag_set(Flag::Zero))
                m_program_counter = address - sizeof(insr_t);
        } break;
        case InstructionType::And: {
            fmt::println("and r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            if (r2 >= ProcessorSpec::register_count)
                return false;
            if (r3 >= ProcessorSpec::register_count)
                return false;
            auto& dst = m_registers[r1];
            auto& lhs = m_registers[r2];
            auto& rhs = m_registers[r3];

            dst = lhs & rhs;

            set_zn_flags(dst);
        } break;
        case InstructionType::Or: {
            fmt::println("or r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            if (r2 >= ProcessorSpec::register_count)
                return false;
            if (r3 >= ProcessorSpec::register_count)
                return false;
            auto& dst = m_registers[r1];
            auto& lhs = m_registers[r2];
            auto& rhs = m_registers[r3];

            dst = lhs | rhs;

            set_zn_flags(dst);
        } break;
        case InstructionType::Xor: {
            fmt::println("xor r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= ProcessorSpec::register_count)
                return false;
            if (r2 >= ProcessorSpec::register_count)
                return false;
            if (r3 >= ProcessorSpec::register_count)
                return false;
            auto& dst = m_registers[r1];
            auto& lhs = m_registers[r2];
            auto& rhs = m_registers[r3];

            dst = lhs ^ rhs;

            set_zn_flags(dst);
        } break;
        case InstructionType::Push: {
            fmt::println("push r{}", r1);

            if (m_stack_pointer <= ProcessorSpec::stack_top_addr - ProcessorSpec::stack_size) {
                fmt::println("stack overflow");
                return false;
            }

            if (r1 >= ProcessorSpec::register_count)
                return false;
            auto& src = m_registers[r1];

            m_stack_pointer -= static_cast<addr_t>(sizeof(data_t));
            m_memory[m_stack_pointer] = src;
        } break;
        case InstructionType::Pop: {
            fmt::println("pop r{}", r1);

            if (m_stack_pointer >= ProcessorSpec::stack_top_addr) {
                fmt::println("stack underflow (tried popping empty stack)");
                return false;
            }

            if (r1 >= ProcessorSpec::register_count)
                return false;
            auto& dst = m_registers[r1];

            dst = m_memory[m_stack_pointer];
            m_stack_pointer += sizeof(data_t);
        } break;
        case InstructionType::Halt: {
            set_flag(Flag::Halt);
            return true;
        } break;
        default:
            return false;
        }
        return true;
    }

private:
    std::array<u8, ProcessorSpec::highest_addr> m_memory { 0 };
    std::array<reg_t, ProcessorSpec::register_count> m_registers { 0 };

    addr_t m_program_counter { 0 };
    addr_t m_stack_pointer { ProcessorSpec::stack_top_addr };
    u8 m_flags { 0 };
};
