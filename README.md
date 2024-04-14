# processor

A made up CPU architecture and emulator

## Features

- Load and store from and to other registers or memory addresses

- Stack pushing and popping

- Arithmetic operations

- Jumping, both conditional and unconditional

- Enough to count down from 10 to 0

- That's basically it.

## Details

- 16 different instructions, with the type fitting into 4 bits
- 8-bit data and 16-bit addressing width
- 64KiB memory
- A stack
- 8x 8-bit registers
- 16-bit program counter register
- 16-bit stack pointer register
- 8-bit flag register

Implemented in modern C++23. (Could be compatible with C++20 too, other than `std::to_underlying`.)

Even has some tests.

Full ISA:

```c
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
```

Instruction encoding:

```c
// 16-bit ins
// type' reg' reg' reg'
// tttt'..r1'..r2'..r3
// or
// type' reg'8bit data
// tttt'..r1'dddd'dddd
// reg fields need to be 2-bits
```

## Does this have any practical use?

No.

## Why did you build this, then?

For fun and as a learning experience. In fact, I've already learned quite a lot! I can see dozens of problems immediately, and if I were to start again tomorrow, a fictional processor like this would certainly look different!

I hacked it together in a day (about 5 hours total) with minimal prior knowledge - built a NES/6502 emulator 3 years ago as a C++ learning exercise.

For that reason, there may be similarities to the 6502, but it's still a completely different architecture and not compatible in any way.

## Issues

- 8 registers are too many for such an ancient design.
  The design started out with just 4, but limits were reached far too
  quickly and programming turned out impossible.
- No banking, you're stuck with whatever fits into 64K.
- Limiting to 4-bit instruction types, the current instruction
  encoding could easily support 5 or 6-bit types, with the potential
  to make the assembly more bearable.
- This is because the instruction set was clearly made by someone
  with only the tiniest amount of experience. Instructions are extremely
  heavy on registers, making it hard to keep track of where data
  is located.
- Specifying memory addresses. They are 16 bits wide, but since data
  registers only fit 8 bits and instructions rely on those, all
  addresses need to be split up into their lower and upper bytes,
  loaded into two separate registers and can only then be specified
  for instruction usage.
  In other words, there is no `store $BEEF, #42` syntax, instead it
  would look like this:

  `ldi r1, #190`

  `ldi r2, #239`

  `st r1, r2, #42`

- There is no assembler; programs are hand-assembled, further adding
  to the register allocation mess. (This is why it expanded to 8.)
- There is no disassembler or other debugging tools.
- Testing is minimal and obvious bugs remain.