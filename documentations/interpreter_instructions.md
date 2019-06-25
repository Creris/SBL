
# Interpreter Instructions Definition

This file contains definitions of behaviour of instructions defined by given mnemonics for the interpreter as well as the amount and type of parameters.

## Table of contents

- [Interpreter Instructions Definition](#interpreter-instructions-definition)
  - [Table of contents](#table-of-contents)
  - [Generic information](#generic-information)
  - [Instructions with no operands](#instructions-with-no-operands)
    - [Nop](#nop)
    - [Halt, End](#halt-end)
    - [Ret](#ret)
    - [Loop](#loop)
    - [Endloop](#endloop)
    - [IRet](#iret)
    - [Push_All](#pushall)
    - [Pop_All](#popall)
    - [Clear_All](#clearall)
    - [DisableAllInts](#disableallints)
    - [RestoreInts](#restoreints)
    - [EnableAllInts](#enableallints)
    - [ClrCb](#clrcb)
  - [Instructions with one parameter](#instructions-with-one-parameter)
    - [Push](#push)
    - [Pop](#pop)
    - [Inc](#inc)
    - [Dec](#dec)
    - [Call, RCall](#call-rcall)
    - [NtvCall](#ntvcall)
    - [Read](#read)
    - [Readstr](#readstr)
    - [Print, PrintS](#print-prints)
    - [Printstr](#printstr)
    - [Jmp, RJmp](#jmp-rjmp)
    - [Jx, RJx](#jx-rjx)
    - [Not](#not)
    - [Time](#time)
    - [Time64](#time64)
    - [ICount](#icount)
    - [ICount64](#icount64)
    - [Dealloc](#dealloc)
    - [Raise](#raise)
    - [DisableInt](#disableint)
    - [EnableInt](#enableint)
    - [ICountInt, RICountInt](#icountint-ricountint)
    - [ICountInt64, RICountInt64](#icountint64-ricountint64)
  - [Instructions with two parameters](#instructions-with-two-parameters)
    - [Mov](#mov)
    - [Movx](#movx)
    - [Laddr](#laddr)
    - [Add](#add)
    - [Sub](#sub)
    - [Mul](#mul)
    - [Div](#div)
    - [Mod](#mod)
    - [Test](#test)
    - [Lsh](#lsh)
    - [Rlsh](#rlsh)
    - [Rsh](#rsh)
    - [Rrsh](#rrsh)
    - [And](#and)
    - [Or](#or)
    - [Xor](#xor)
    - [Eq](#eq)
    - [Neq](#neq)
    - [Lt](#lt)
    - [Bt](#bt)
    - [Loadload](#loadload)
    - [Alloc](#alloc)
    - [VCall](#vcall)
    - [RegInt, RRegInt](#regint-rregint)
    - [GetNtvId](#getntvid)
    - [Xchg](#xchg)

## Generic information

Every instruction that is executed, without looking at what it does, will increment internal counter of executed instructions. This is also true for instructions executed while inside interrupt handler.

Every instruction is exactly 12 bytes in width and is split in the following way:

 * First four bytes(Bytes 1 - 4) define the mnemonic of the instruction. This mnemonic tells the interpreter what the instruction actually does.
 * Second four bytes(Bytes 5 - 8) define the value of the first operand for the instruction.
 * Last four bytes(Bytes 9 - 12) define the value of the second operand for the instruction.

If a instruction is defined to have no operands, or is defined as single operand instruction, the value of the remaining operand(s) is ignored, but the bytes are still required to be present.

All instructions that start with capital R followed by another capital letter are a 'relative' counterparts to their instructions without R.

A relative instruction works with the value relative to the current value.

Example of this is a `Call` and `RCall`. Call will jump to the value provided to it as a parameter, while `RCall` will jump from the address forward relative to current instruction pointer by the amount provided as a parameter(basically incrementing the instruction pointer).

## Instructions with no operands

### Nop

Alternative name: NaN

| Encoding | |
| -------- | :-----: |
| Decimal  | 0       |
|   Hex    | 0x00    |

This instruction tells the interpreter to do nothing and continue to next instruction.

### Halt, End

Alternative names: NaN

| Encoding |  Halt   | End   |
| -------- | :-----: | :---: |
| Decimal  | 1       | 2     |
|   Hex    | 0x01    | 0x02  |

These two instructions will tell the interpreter to stop and finalize all execution.

### Ret

Alternative names: Return from function

| Encoding | |
| -------- | :-----: |
| Decimal  | 3       |
|   Hex    | 0x03    |

This instruction returns the control flow of the interpreter to the last call position.

This is achieved by taking the value stored by the `Call` instruction in stack, by popping the stack, and setting the instruction pointer to this value.

If the value at the top of the stack from the caller's position has been overwritten, or the stack pointer has not been restored to the same position as at the call to the function, the result of this instruction will be change of control flow to undefined position.

### Loop

Alternative names: NaN

| Encoding | |
| -------- | :-----: |
| Decimal  | 4       |
|   Hex    | 0x04    |

This instruction stores the current loop pointer at the top of the stack and sets the loop pointer to the address of the next instruction.

This value is then used by the following `Endloop` instruction.

### Endloop

Alternative names: NaN

| Encoding | |
| -------- | :-----: |
| Decimal  | 5       |
|   Hex    | 0x05    |

This instruction reads a value from register r10 and checks if it is 0. If the register is zero, the interpreter will pop the stack into loop pointer, restoring it to its previous value(allowing nested loops) and continue execution.

If the value of register r10 is not equal to 0, the register is decremented, and the interpreter assigns the loop pointer to instruction pointer, effectively jumping to the first instruction after the most recent `Loop` instruction.

### IRet

Alternative names: Return from instruction handler.

| Encoding | |
| -------- | :-----: |
| Decimal  | 476     |
|   Hex    | 0x1DC   |

Returns the control flow from an interrupt handler to the next instruction to be executed(that was interrupted).

For more information check documentation on `Interrupts`.

### Push_All

Alternative names: Push all registers to stack.

| Encoding | |
| -------- | :-----: |
| Decimal  | 479     |
|   Hex    | 0x1DF   |

Pushes all registers to the stack starting with the lowest indexed one.

### Pop_All

Alternative names: Pop all registers from stack.

| Encoding | |
| -------- | :-----: |
| Decimal  | 480     |
|   Hex    | 0x1E0   |

Pops all registers from the stack, starting with the highest indexed register.

Also overwrites the control registers indexed 60 and higher.

### Clear_All

Alternative names: Clear all data registers

| Encoding | |
| -------- | :-----: |
| Decimal  | 481     |
|   Hex    | 0x1E1   |

Set all data registers(indexes 0 - 59) to 0.

### DisableAllInts

Alternative names: Disable all interrupts.

| Encoding | |
| -------- | :-----: |
| Decimal  | 497     |
|   Hex    | 0x1F1   |

Disables all interrupts, storing the status of all interrupts in a backup array.

### RestoreInts

Alternative names: Restore interrupts from backup.

| Encoding | |
| -------- | :-----: |
| Decimal  | 498     |
|   Hex    | 0x1F2   |

Restores all statuses of interrupts from the backup array.

### EnableAllInts

Alternative names: Enable all interrupts.

| Encoding | |
| -------- | :-----: |
| Decimal  | 499     |
|   Hex    | 0x1F3   |

Enables all interrupts so that every interrupt is active and ready to be handled.

### ClrCb

Alternative names: Clear control byte.

| Encoding | |
| -------- | :-----: |
| Decimal  | 500     |
|   Hex    | 0x1F4   |

Sets the control byte that is used in conditional moves and jumps, and is set by Test to 0.

## Instructions with one parameter

Every instruction with one parameter can have its parameter as one of the following type:

* Register, marked as R
* Address, marked as A
* Indirect, marked as I
* Value, marked as V

A register parameter will interact with a register with given index.

An address parameter will interact with given address.

An indirect parameter will interact with an address pointed to by the given register.

A value parameter will make the instruction work with the value of the parameter itself.

Every instruction address follows the following scheme:

* First encoding of the instruction will use a register parameter.
* Second encoding will use the address as a parameter, equal to First + 1.
* Third encoding will use the indirect parameter, equal to Second + 1.
* Fourth encoding will use the value as a parameter, equal to Third + 1.

If any of these are missing, it is skipped and the next encoding will be the next valid one.

### Push

Alternative names: Push to stack

| Encoding | |
| -------- | :-----: |
| Decimal  | 6       |
|   Hex    | 0x06    |

Available parameter types:

* Register
* Address
* Indirect
* Value

This instruction pushed a value to the address of stack pointer - 1, and then decrements the stack pointer.

If the stack pointer is 0, this instruction will raise a `StackUnderflow` error and stop execution.

### Pop

Alternative names: Pop from stack

| Encoding | |
| -------- | :-----: |
| Decimal  | 10      |
|   Hex    | 0x0A    |

Available parameter types:

* Register
* Address
* Indirect

Takes the value stored in the program memory at where stack pointer points to into the destination and increments the stack pointer.

### Inc

Alternative names: Increment

| Encoding | |
| -------- | :-----: |
| Decimal  | 13      |
|   Hex    | 0x0D    |

Available parameter types:

* Register
* Address
* Indirect

Increments the value stored in the destination by one.

### Dec

Alternative names: Decrement

| Encoding | |
| -------- | :-----: |
| Decimal  | 16      |
|   Hex    | 0x10    |

Available parameter types:

* Register
* Address
* Indirect
  
Decrements the value stored in the destination by one.

### Call, RCall

Alternative names: Call a function, Call a function relative to current instruction pointer.

| Encoding |  Call   | RCall   |
| -------- | :-----: | :-----: |
| Decimal  | 19      | 22      |
|   Hex    | 0x13    | 0x16    |

Available parameter types:

* Register
* Address
* Indirect

Stores the address of the instruction immediately following this `Call` instruction into the top of the stack and writes the the value stored in target into instruction pointer.

Note that for this instruction, if the parameter is of type address, the value is taken as is, instead of loaded from that memory unit.

### NtvCall

Alternative name: Call a native function.

| Encoding | |
| -------- | :-----:  |
| Decimal  | 472      |
|   Hex    | 0x1D8    |

Available parameter types:

* Register
* Address
* Indirect
* Value

Tells the interpreter to attempt to call a native function. For more information please refer to documentation on `Native functions`.

### Read

Alternative name: Read an integer from standard input.

| Encoding | |
| -------- | :-----: |
| Decimal  | 25      |
|   Hex    | 0x19    |

Available parameter types:

* Register
* Address
* Indirect

Instructs the interpreter to read an integer into the destination.

### Readstr

Alternative name: Read a string from standard input.

| Encoding | |
| -------- | :-----: |
| Decimal  | 28      |
|   Hex    | 0x1C    |

Available parameter types:

* Address
* Indirect

Instructs the interpreter to read a string into the destination.

### Print, PrintS

Alternative names: Print an integer to standard output, Print a signed integer into standard output

| Encoding | Print   | PrintS |
| -------- | :-----: | :---:  |
| Decimal  | 30      | 34     |
|   Hex    | 0x1E    | 0x22   |

Available parameter types:

* Register
* Address
* Indirect
* Value

Instructs the interpreter to print a integer, potentially signed, from the destination into the standard output.

### Printstr

Alternative name: Print a string into the standard output.

| Encoding | |
| -------- | :-----: |
| Decimal  | 38      |
|   Hex    | 0x26    |

Available parameter types:

* Address
* Indirect

Instructs the interpreter to print a string from the destination into the standard output.

### Jmp, RJmp

Alternative name: Unconditional jump, Unconditional relative jump

| Encoding | Jmp     | RJmp    |
| -------- | :-----: | :----:  |
| Decimal  | 40      | 79      |
|   Hex    | 0x28    | 0x4F    |

Available parameter types:

* Register
* Address
* Indirect

Changes the instruction pointer to the value stored in the parameter.

Note that this instruction will interpret a parameter of type address as the value itself.

### Jx, RJx

Alternate names: Conditional jumps on control flag X, Conditional relative jumps on control flag X.

| Encoding | Jb    |  Jnb   |  Jbe   |  Jnbe  |  Jl    |  Jnl   |  Jle   |  Jnle  |  Jz    |  Jnz   |  Je    |  Jne   |
| -------- | :---: | :---:  | :---:  | :---:  | :---:  | :---:  | :---:  | :---:  | :---:  | :---:  | :---:  | :---:  |
| Decimal  | 43    | 46     | 49     | 52     | 55     | 58     | 61     | 64     |  67    | 70     | 73     | 76     |
|   Hex    | 0x2B  | 0x2E   | 0x31   | 0x34   | 0x37   | 0x3A   | 0x3D   | 0x40   | 0x43   | 0x46   | 0x49   | 0x4C   |

Relative jumps have encoding equal to the encoding of their non-relative counterpart + 39.

Will perform a jump only if the specified control flag in the control byte of the interpreter is set(this value is set after a call to Test and is not reset by other instructions).

The control flag that is inspected is as following:

| Jump        | Control flag |
| -----       | ------------ |
| Jb, Jnle    | Bigger       |
| Jnb, Jle    | SmallerEqual |
| Jbe, Jnl    | BiggerEqual  |
| Jnbe, Jl    | Smaller      |
| Jz, Je      | Equal        |
| Jnz, Jne    | NotEqual     |

### Not

Alternative name: Bit-invert a value

| Encoding | |
| -------- | :-----: |
| Decimal  | 118     |
|   Hex    | 0x76    |

Available parameter types:

* Register

Inverts every bit in a register.

### Time

Alternative name: Get current time.

| Encoding | Register parameter | Address and Indirect |
| -------- | :-----:            | :--------:           |
| Decimal  | 119                | 462, 463             |
|   Hex    | 0x77               | 0x1CE, 0x1CF         |

Available parameter types:

* Register
* Address
* Indirect

Writes the lower 32 bits of the time since the start of the execution in microseconds into destination.

Overflows every 71.58 minutes.

Even tho the clock overflows, it can be used as a delta for measurement of time between two time events(a time delta) with correct timing.

### Time64

Alternative name: Get current time in 64 bits.

| Encoding | |
| -------- | :-----: |
| Decimal  | 464     |
|   Hex    | 0x1D0   |

Available parameter types:

* Register
* Address
* Indirect

Writes the lower 32 bits of the time obtained, same as with `Time` instruction, into the destination, and then writes the upper 32 bits of running time into the immediate next address or register.

Overflows every 584,942.417 years.

### ICount

Alternative name: Get instruction counter.

| Encoding | Register parameter | Address, Indirect |
| -------- | :-----:            | :--------:        |
| Decimal  | 120                | 467, 468          |
|   Hex    | 0x78               | 0x1D3, 0x1D4      |

Available parameter types:

* Register
* Address
* Indirect

Writes the 32 lowest bits of the counter for instructions executed since the start of execution into destination.

### ICount64

Alternative name: Get 64 bit instruction counter.

| Encoding | |
| -------- | :-----: |
| Decimal  | 469     |
|   Hex    | 0x1D5   |

Available parameter types:

* Register
* Address
* Indirect

Writes the 32 lowest bits of the counter for instructions executed since the start of execution into destination and writes the higher 32 bits into the immediate next address or register.

### Dealloc

Alternative name: Deallocate dynamic memory

| Encoding | |
| -------- | :-----: |
| Decimal  | 121     |
|   Hex    | 0x79    |

Available parameter types:

* Register
* Address
* Indirect

Attempts to deallocate a block of memory that was previously allocated by the `Alloc` instruction pointed to by the parameter.

### Raise

Alternative name: Raise an interrupt.

| Encoding | |
| -------- | :-----: |
| Decimal  | 124     |
|   Hex    | 0x7C    |

Available parameter types:

* Register
* Address
* Indirect
* Value

Raises an interrupt of given number.

For more information on interrupts please refer to the documentation on `Interrupts`.

### DisableInt

Alternative name: Disable an interrupt.

| Encoding | |
| -------- | :-----: |
| Decimal  | 128     |
|   Hex    | 0x80    |

Available parameter types:

* Register
* Value

Disables an interrupt so that it can no longer be raised.

### EnableInt

Alternative name: Enable an interrupt.

| Encoding | |
| -------- | :-----: |
| Decimal  | 130     |
|   Hex    | 0x82    |

Available parameter types:

* Register
* Value

Enables an interrupt so that it can be raised.

### ICountInt, RICountInt

Alternative name: Raise an interrupt on instruction count.

| Encoding | ICountInt | RICountInt |
| -------- | :-----:   | :----: |
| Decimal  | 132       | 477    |
|   Hex    | 0x84      | 0x1DD  |

Available parameter types:

* Register
* Value

Raises an interrupt `InstrCount`(code 251) once executed instruction count is higher than the parameter provided.

If the interrupt is not enabled, does nothing.

If there is another interrupt being handled already when this interrupt is to be raised(such as the instruction count coincides to be hit during handling of another interrupt), the interrupt will be postponed until there is no other interrupt being handled.

### ICountInt64, RICountInt64

Alternative name: Raise an interrupt on instruction count.

| Encoding | ICountInt | RICountInt |
| -------- | :-----:   | :----: |
| Decimal  | 501       | 504    |
|   Hex    | 0x1F5     | 0x1F8  |

Available parameter types:

* Register
* Value

Does the same thing as `ICountInt` and `RICountInt`, but the operand is treated as 64 bit, considering the parameter as the lower 32 bits of the counter, and the immediate next address or register as upper 32 bits.

## Instructions with two parameters

Instructions with two parameters follow the same pattern as instructions with one parameter, alternating the second parameter before the first.

So a instruction with mnemonic `_R_R` will be before `_R_A`, that will be before `_A_R`.

The encoding goes up by 1 for every new mnemonic, unless specified otherwise.

The first parameter is considered to be both source and destination, while the second parameter is considered to be purely source parameter.

### Mov

Alternative name: Move from source to destination.

| Encoding | |
| -------- | :-----: |
| Decimal  | 134     |
|   Hex    | 0x86    |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Moves a value from source to destination.

### Movx

Alternative name: Conditionally move from source to destination.

| Encoding | Move    | Movne   | Movz    | Movnz   |
| -------- | :-----: | :-----: | :-----: | :-----: |
| Decimal  | 146     | 158     | 170     | 182     |
|   Hex    | 0x92    | 0x9E    | 0xAA    | 0xB6    |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Moves a value from source to destination only if a specific control flag in the control byte is set.

Flags inspected:

| Mov          | Control flag |
| -----        | ------------ |
| Move, Movz   | Equal        |
| Movne, Movnz | Unequal      |

### Laddr

Alternative name: Load effective address.

| Encoding | |
| -------- | :-----: |
| Decimal  | 194     |
|   Hex    | 0xC2    |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | No     |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | No     |

Load the address of the source into destination. Effectively moves the value of the parameter instead of the value at that address.

### Add

Alternative name: Add source to destination.

| Encoding | |
| -------- | :-----: |
| Decimal  | 200     |
|   Hex    | 0xC8    |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Adds the value from source to destination.

### Sub

Alternative name: Substract source from destination.

| Encoding | |
| -------- | :-----: |
| Decimal  | 212     |
|   Hex    | 0xD4    |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Substracts the value from source to destination.

### Mul

Alternative name: Multiply source with destination.

| Encoding | |
| -------- | :-----: |
| Decimal  | 224     |
|   Hex    | 0xE0    |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Multiplies the source values and stores the result in destination.

### Div

Alternative name: Divide destination by source.

| Encoding | |
| -------- | :-----: |
| Decimal  | 236     |
|   Hex    | 0xEC    |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Divides the destination by source and stores the result in destination.

### Mod

Alternative name: Modulo destination with source.

| Encoding | |
| -------- | :-----: |
| Decimal  | 248     |
|   Hex    | 0xF8    |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Performs a `destination % source` and stores it in destination.

### Test

Alternative name: Test source and destination.

| Encoding | |
| -------- | :-----: |
| Decimal  | 260     |
|   Hex    | 0x104   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Sets the control flags in the control byte depending on the values of source and destination:

* `destination == source`
  * Sets `Equal, BiggerEqual, SmallerEqual`,
* `destination > source`
  * Sets `Unequal, Bigger`,
* `destination < source`
  * Sets `Unequal, Smaller`.

The values of control byte are not cleared between instructions.

### Lsh

Alternative name: Left shift.

| Encoding | |
| -------- | :-----: |
| Decimal  | 272     |
|   Hex    | 0x110   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Shifts destination by source bits to the left.

### Rlsh

Alternative name: Rotate left shift.

| Encoding | |
| -------- | :-----: |
| Decimal  | 284     |
|   Hex    | 0x11C   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Performs a rotation left shift on `destination` by `source` bits.

### Rsh

Alternative name: Right shift.

| Encoding | |
| -------- | :-----: |
| Decimal  | 296     |
|   Hex    | 0x128   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Performs an arithmetic right shift on `destination` by `source` bits.

### Rrsh

Alternative name: Rotate right shift.

| Encoding | |
| -------- | :-----: |
| Decimal  | 308     |
|   Hex    | 0x134   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Performs a rotate right shift on `destination` by `source` bits.

### And

Alternative name: Logical and.

| Encoding | |
| -------- | :-----: |
| Decimal  | 320     |
|   Hex    | 0x140   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Performs a logical bitwise AND on destination and source and stores the result in destination.

### Or

Alternative name: Logical or.

| Encoding | |
| -------- | :-----: |
| Decimal  | 332     |
|   Hex    | 0x14C   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Performs a logical bitwise OR on destination and source and stores the result in destination.

### Xor

Alternative name: Logical xor.

| Encoding | |
| -------- | :-----: |
| Decimal  | 344     |
|   Hex    | 0x158   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Performs a logical bitwise XOR on destination and source and stores the result in destination.

### Eq

Alternative name: Set equal control flag.

| Encoding | |
| -------- | :-----: |
| Decimal  | 356     |
|   Hex    | 0x164   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Resets control flags in control byte and sets the `Equal`, `BiggerEqual` and `SmallerEqual` flags if and only if `destination == source`.

### Neq

Alternative name: Set unequal control flag.

| Encoding | |
| -------- | :-----: |
| Decimal  | 368     |
|   Hex    | 0x170   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Resets control flags in control byte and sets the `Unequal` flag if and only if `destination != source`.

### Lt

Alternative name: Set unequal and smaller control flags.

| Encoding | |
| -------- | :-----: |
| Decimal  | 380     |
|   Hex    | 0x17C   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Resets control flags in control byte and sets the `Unequal` and `Smaller` flags if and only if `destination < source`.

### Bt

Alternative name: Set unequal and bigger control flags.

| Encoding | |
| -------- | :-----: |
| Decimal  | 392     |
|   Hex    | 0x188   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Resets control flags in control byte and sets the `Unequal` and `Bigger` flags if and only if `destination > source`.

### Loadload

Alternative name: Load from indirect.

| Encoding | |
| -------- | :-----: |
| Decimal  | 404     |
|   Hex    | 0x194   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | No     |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | No     |

Loads the value that the parameter points to into destination.

### Alloc

Alternative name: Load from indirect.

| Encoding | |
| -------- | :-----: |
| Decimal  | 410     |
|   Hex    | 0x19A   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | Yes    |

Allocates a memory block of size equal to second operand and stores the address of the allocated block of memory in the first operand.

### VCall

Alternative name: Load from indirect.

| Encoding | |
| -------- | :-----: |
| Decimal  | 422     |
|   Hex    | 0x1A6   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | Yes    | Yes    |

Performs a virtual call instead of a normal call.

Performs it in following way:

`instruction pointer = operand_1 + operand_2`

First operand represents the virtual table base address and the second operand represents the offset in that virtual table.

### RegInt, RRegInt

Alternative name: Load from indirect.

| Encoding | RegInt  | RRegInt |
| -------- | :-----: | :-----: |
| Decimal  | 438     | 450     |
|   Hex    | 0x1B6   | 0x1C2   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | Yes    | No     |

Loads the value that the parameter points to into destination.

### GetNtvId

Alternative name: Load from indirect.

| Encoding | |
| -------- | :-----: |
| Decimal  | 482     |
|   Hex    | 0x1E2   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | No     |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | No     |

Gets the native function identified by the string stored in the parameter.

If there is no native identified by this name, writes -1 into the destination.

### Xchg

Alternative name: Exchange.

| Encoding | |
| -------- | :-----: |
| Decimal  | 488     |
|   Hex    | 0x1E8   |

Available parameter types:

| Type      | First  | Second |
| -------   | :----: | :----: |
| Register  | Yes    | Yes    |
| Address   | Yes    | Yes    |
| Indirect  | Yes    | Yes    |
| Value     | No     | No     |

Exchanges destination and source.
