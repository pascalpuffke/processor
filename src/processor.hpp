#include <array>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <span>
#include <stack>
#include <utility>

#include <types.hpp>

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
 * done                     set 'kill' bit and stop execution
 */

// These two are required to clear up compiler confusions about `encode_instruction` parameters
struct Register {
    using reg_t = u8;
    reg_t reg { 0 };
    constexpr explicit Register(reg_t r)
        : reg(r) { }
    constexpr operator reg_t() const { return reg; }
};

struct Immediate {
    using imm_t = u8;
    imm_t imm { 0 };
    constexpr explicit Immediate(imm_t i)
        : imm(i) { }
    constexpr operator imm_t() const { return imm; }
};

template <>
struct fmt::formatter<Register> : formatter<u8> {
    auto format(Register r, format_context& ctx) const {
        return formatter<u8>::format(r.reg, ctx);
    }
};

template <>
struct fmt::formatter<Immediate> : formatter<u8> {
    auto format(Immediate i, format_context& ctx) const {
        return formatter<u8>::format(i.imm, ctx);
    }
};

class Processor {
public:
    // clang-format off
    enum class InstructionType : u8 {
        LoadFromReg = 0b0000,
        Store       = 0b0001,
        Add         = 0b0010,
        Sub         = 0b0011,
        Mul         = 0b0100,
        Div         = 0b0101,
        Jump        = 0b0110,
        JumpIfZero  = 0b0111,
        And         = 0b1000,
        Or          = 0b1001,
        Xor         = 0b1010,
        Push        = 0b1011,
        Pop         = 0b1100,
        LoadFromImm = 0b1101,
        LoadFromMem = 0b1110,
        Done        = 0b1111,
    };

    enum class Flag : u8 {
        Carry    = 0b0001,
        Zero     = 0b0010,
        Overflow = 0b0100,
        Negative = 0b1000,
    };
    // clang-format on

    using data_t = u8;
    using reg_t = u8;
    using imm_t = u8;
    using addr_t = u16;
    using insr_t = u16;

    static constexpr auto highest_addr = std::numeric_limits<addr_t>::max();
    static constexpr auto register_count = 8;
    // Where to look for the address to start execution from (addr_t reset_pc | reset_pc + 1)
    static constexpr addr_t reset_pc = 0xFF00;

    // type|0000|0000|0000
    static constexpr auto encode_instruction(InstructionType type) -> insr_t {
        return std::to_underlying(type) << 12;
    }

    // type|00R1|0000|0000
    static constexpr auto encode_instruction(InstructionType type, Register r1) -> insr_t {
        return std::to_underlying(type) << 12 | r1 << 8;
    }

    // type|00R1|00R2|0000
    static constexpr auto encode_instruction(InstructionType type, Register r1, Register r2) -> insr_t {
        return std::to_underlying(type) << 12 | r1 << 8 | r2 << 4;
    }

    // type|00R1|00R2|00R3
    static constexpr auto encode_instruction(InstructionType type, Register r1, Register r2, Register r3) -> insr_t {
        return std::to_underlying(type) << 12 | r1 << 8 | r2 << 4 | r3 & 0xF;
    }

    // type|00R1|immm|immm
    static constexpr auto encode_instruction(InstructionType type, Register r1, Immediate imm) -> insr_t {
        return std::to_underlying(type) << 12 | r1 << 8 | imm;
    }

    constexpr auto reset(addr_t new_pc = reset_pc) {
        m_memory.fill(0);
        m_registers = { 0 };
        m_program_counter = new_pc;
        m_stack_pointer = 0;
        m_stack_size = 0xFF;
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
        write_memory(start_address, (encoded_instruction >> 8) & 0xFF);
        write_memory(start_address + 1, encoded_instruction & 0xFF);
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

    [[nodiscard]] constexpr auto stack_size() const noexcept {
        return m_stack_size;
    }

    [[nodiscard]] constexpr auto flags() const noexcept {
        return m_flags;
    }

    constexpr auto is_flag_set(Flag flag) const noexcept -> bool {
        return m_flags & std::to_underlying(flag);
    }

    constexpr auto set_flag(Flag flag) -> void {
        m_flags |= std::to_underlying(flag);
    }

    constexpr auto unset_flag(Flag flag) -> void {
        m_flags &= ~std::to_underlying(flag);
    }

    constexpr auto flag_string() const -> std::string {
        auto string = std::string {};
        string.append("....");

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

        return string;
    }

    constexpr auto dump_memory(int width = 8) const noexcept {
        for (auto x = 0; x < m_memory.size(); x += width) {
            fmt::print("0x{:X}: ", x);
            for (auto xx = 0; xx < width; xx++) {
                auto byte = m_memory[x + xx];
                fmt::print("{:X} ", byte);
            }
            fmt::print("\n");
        }
    }

    constexpr auto dump_state(bool with_memory = false) const noexcept {
        if (with_memory) {
            fmt::println("memory={}", m_memory);
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
            if (m_should_kill_itself)
                return false;

            if (m_program_counter == std::numeric_limits<decltype(m_program_counter)>::max()) {
                fmt::println("reached end of memory");
                return false;
            }

            const auto instruction = read_instruction(m_program_counter);
            const auto decoded_instruction = decode_instruction(instruction);
            const auto executed = execute_instruction(decoded_instruction);

            if (!executed) {
                fmt::println("instruction @ pc=0x{:X} failed to execute", m_program_counter);
                fmt::println(
                    "type={} r1={} r2={} r3={} data={}",
                    std::to_underlying(decoded_instruction.type),
                    decoded_instruction.r1,
                    decoded_instruction.r2,
                    decoded_instruction.r3,
                    decoded_instruction.data
                );
                return false;
            }

            m_program_counter += sizeof(insr_t);
        }

        return true;
    }

    constexpr auto write_register(u8 reg, data_t data) {
        if (reg < register_count)
            m_registers[reg] = data;
    }

    constexpr auto read_instruction(addr_t addr) -> insr_t {
        static_assert(sizeof(insr_t) == 2);

        const auto first_byte = read_memory(addr);
        const auto second_byte = read_memory(addr + 1);
        const insr_t instruction = (first_byte << 8) | second_byte;
        return instruction;
    }

    constexpr auto decode_instruction(insr_t instruction) -> DecodedInstruction {
        if (instruction == 0)
            m_should_kill_itself = true;

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

            if (r1 >= register_count)
                return false;
            if (r2 >= register_count)
                return false;
            auto& dst = m_registers[r1];
            auto& src = m_registers[r2];

            dst = src;

            set_zn_flags(dst);
        } break;
        case InstructionType::LoadFromImm: {
            fmt::println("ldi r{}, #{}", r1, instruction.data);

            if (r1 >= register_count)
                return false;
            auto& dst = m_registers[r1];

            dst = instruction.data;

            set_zn_flags(dst);
        } break;
        case InstructionType::LoadFromMem: {
            fmt::println("ldm r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= register_count)
                return false;
            if (r2 >= register_count)
                return false;
            if (r3 >= register_count)
                return false;

            auto& dst = m_registers[r1];
            auto& low_reg = m_registers[r2];
            auto& high_reg = m_registers[r3];
            addr_t address = (low_reg << 8) | high_reg;

            dst = read_memory(address);

            set_zn_flags(dst);
        } break;
        case InstructionType::Store: {
            fmt::println("st r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= register_count)
                return false;
            if (r2 >= register_count)
                return false;
            if (r3 >= register_count)
                return false;


            auto& low_reg = m_registers[r1];
            auto& high_reg = m_registers[r2];
            auto& src = m_registers[r3];
            addr_t address = (low_reg << 8) | high_reg;

            write_memory(address, src);

            set_zn_flags(src);
        } break;
        case InstructionType::Add: {
            fmt::println("add r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= register_count)
                return false;
            if (r2 >= register_count)
                return false;
            if (r3 >= register_count)
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

            if (r1 >= register_count)
                return false;
            if (r2 >= register_count)
                return false;
            if (r3 >= register_count)
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

            if (r1 >= register_count)
                return false;
            if (r2 >= register_count)
                return false;
            if (r3 >= register_count)
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

            if (r1 >= register_count)
                return false;
            if (r2 >= register_count)
                return false;
            if (r3 >= register_count)
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

            if (r1 >= register_count)
                return false;
            if (r2 >= register_count)
                return false;

            auto& low_reg = m_registers[r1];
            auto& high_reg = m_registers[r2];
            addr_t address = (low_reg << 8) | high_reg;

            // I don't like the -2 here.
            m_program_counter = address - 2;
        } break;
        case InstructionType::JumpIfZero: {
            fmt::println("jz r{}, r{}", r1, r2);

            if (r1 >= register_count)
                return false;
            if (r2 >= register_count)
                return false;

            auto& low_reg = m_registers[r1];
            auto& high_reg = m_registers[r2];
            addr_t address = (low_reg << 8) | high_reg;

            // Neither do I like it here.
            if (is_flag_set(Flag::Zero))
                m_program_counter = address - 2;
        } break;
        case InstructionType::And: {
            fmt::println("and r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= register_count)
                return false;
            if (r2 >= register_count)
                return false;
            if (r3 >= register_count)
                return false;
            auto& dst = m_registers[r1];
            auto& lhs = m_registers[r2];
            auto& rhs = m_registers[r3];

            dst = lhs & rhs;

            set_zn_flags(dst);
        } break;
        case InstructionType::Or: {
            fmt::println("or r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= register_count)
                return false;
            if (r2 >= register_count)
                return false;
            if (r3 >= register_count)
                return false;
            auto& dst = m_registers[r1];
            auto& lhs = m_registers[r2];
            auto& rhs = m_registers[r3];

            dst = lhs | rhs;

            set_zn_flags(dst);
        } break;
        case InstructionType::Xor: {
            fmt::println("xor r{}, r{}, r{}", r1, r2, r3);

            if (r1 >= register_count)
                return false;
            if (r2 >= register_count)
                return false;
            if (r3 >= register_count)
                return false;
            auto& dst = m_registers[r1];
            auto& lhs = m_registers[r2];
            auto& rhs = m_registers[r3];

            dst = lhs ^ rhs;

            set_zn_flags(dst);
        } break;
        case InstructionType::Push: {
            fmt::println("push r{}", r1);

            if (m_stack_pointer >= m_stack_size) {
                fmt::println("stack overflow");
                return false;
            }

            if (r1 >= register_count)
                return false;
            auto& src = m_registers[r1];

            m_memory[m_stack_pointer] = src;
            m_stack_pointer += sizeof(data_t);
        } break;
        case InstructionType::Pop: {
            fmt::println("pop r{}", r1);

            if (m_stack_pointer == 0) {
                fmt::println("stack underflow (tried popping empty stack)");
                return false;
            }

            if (r1 >= register_count)
                return false;
            auto& dst = m_registers[r1];

            dst = m_memory[m_stack_pointer - 1];
            m_stack_pointer -= sizeof(data_t);
        } break;
        case InstructionType::Done: {
            m_should_kill_itself = true;
            return true;
        } break;
        default:
            return false;
        }
        return true;
    }

private:
    std::array<data_t, highest_addr> m_memory { 0 };
    std::array<reg_t, register_count> m_registers { 0 };

    addr_t m_program_counter { 0 };
    addr_t m_stack_pointer { 0 };
    addr_t m_stack_size { 0xFF };
    u8 m_flags { 0 };

    bool m_should_kill_itself { false };
};
