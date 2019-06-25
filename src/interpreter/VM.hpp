#pragma once

#ifndef BUFFED_VM_HEADER_H_
#define BUFFED_VM_HEADER_H_

#include <vector>
#include <array>
#include <string_view>
#include <limits>
#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>

#include "../common/Instruction.hpp"
#include "../common/FixedVector.hpp"

#include <stdint.h>   // for uint32_t
#include <limits.h>   // for CHAR_BIT

namespace sbl::vm {

	/*
		For rotate shifting
	*/
#ifdef _WIN32
#	include <intrin.h>
#else
	static inline uint32_t _rotl(uint32_t n, unsigned int c)
	{
		const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);  // assumes width is a power of 2.

		// assert ( (c<=mask) &&"rotate by type width or more");
		c &= mask;
		return (n << c) | (n >> ((-c)&mask));
	}

	static inline uint32_t _rotr(uint32_t n, unsigned int c)
	{
		const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);

		// assert ( (c<=mask) &&"rotate by type width or more");
		c &= mask;
		return (n >> c) | (n << ((-c)&mask));
	}
#endif

	namespace literals {
		constexpr int8_t operator ""_c(uint64_t i) {
			return static_cast<int8_t>(i);
		}

		constexpr uint8_t operator ""_uc(uint64_t i) {
			return static_cast<uint8_t>(i);
		}
	}

	struct CompiledHeader {
		uint32_t signature;
		uint32_t startAddress;
		uint32_t stackSize;
		uint32_t padding[5];

		/*
			Visual Studio 2017 IntelliSense has an issue with this being default,
			so we have to provide dummy body
		 */
		constexpr CompiledHeader() noexcept : signature(0), startAddress(0), stackSize(0), padding() {}

		constexpr CompiledHeader(const CompiledHeader&) noexcept = default;
		constexpr CompiledHeader(CompiledHeader&&) noexcept = default;

		constexpr CompiledHeader& operator=(const CompiledHeader& other) noexcept {
			signature = other.signature;
			startAddress = other.startAddress;
			stackSize = other.stackSize;
			for (int i = 0; i < 5; ++i)  padding[i] = other.padding[i];
			return *this;
		}

		constexpr CompiledHeader& operator=(CompiledHeader&& other) noexcept {
			signature = other.signature;
			startAddress = other.startAddress;
			stackSize = other.stackSize;
			for (int i = 0; i < 5; ++i)  padding[i] = other.padding[i];
			return *this;
		}

		~CompiledHeader() noexcept = default;

		constexpr CompiledHeader(uint32_t _sig,
			uint32_t startAddr,
			uint32_t stack) : signature(_sig), startAddress(startAddr),
			stackSize(stack), padding() {}

		constexpr bool validate() const {
			return ((signature >> 24) & 0xff) == 's' &&
				((signature >> 16) & 0xff) == 'b' &&
				((signature >> 8) & 0xff) == 'l' &&
				((signature) & 0xff) == 'x';
		}

		constexpr void fromString(std::string_view view) {
			if (view.size() < sizeof(*this))
				return;

			auto unsign = [](const char v) -> uint8_t { return static_cast<uint8_t>(v); };
			auto oneByte = [unsign](const auto v, const auto shift) -> uint32_t {
				return (unsign(v) & 0xff) << shift;
			};

			signature = oneByte(view[0], 24) | oneByte(view[1], 16) | oneByte(view[2], 8) | oneByte(view[3], 0);
			view.remove_prefix(4);
			startAddress = oneByte(view[0], 24) | oneByte(view[1], 16) | oneByte(view[2], 8) | oneByte(view[3], 0);
			view.remove_prefix(4);
			stackSize = oneByte(view[0], 24) | oneByte(view[1], 16) | oneByte(view[2], 8) | oneByte(view[3], 0);
		}
	};

	namespace ch = std::chrono;

	enum class InterruptType : uint8_t {
		InstrCount = 251,
		Unused1 = 252,
		Unused2 = 253,
		Unused3 = 254,
		NoInterrupt = 255
	};

	enum class ErrorCode {
		None,
		InvalidInstruction,
		RetInInterrupt,
		UnhandledInterrupt,
		UnhandledTimeInterrupt,
		InvalidInterruptId,
		NestedInterrupt,
		InvalidNativeId,
		StackUnderflow,
	};

	struct Error {
		ErrorCode code;
		uint32_t instrPtr;
	};

	class VM;

	struct State {
		uint64_t& instrCount;
		uint64_t& nextInstrCountInterrupt;
		uint32_t& controlByte;

		std::array<uint32_t, 64>& registers;

		uint32_t*& program;

		uint32_t& stackPtr;
		uint32_t& instrPtr;
		uint32_t& basePtr;
		uint32_t& loopPtr;
		
		InterruptType& handling;

		Error& errorCode;

		bool& running;
		
		explicit State(VM& vm);

		State(const State&) = delete;
		State(State&&) = delete;
		State& operator=(const State&) = delete;
		State& operator=(State&&) = delete;

		uint32_t nextParameter();
		std::vector<uint32_t> nextParameters(size_t count);
		void returnValue(uint32_t value);
	};

	class VM {
	public:
		using NativeFunc = void(*)(vm::State&);
	private:
		friend struct vm::State;

		enum ControlFlags {
			TestSmaller = 1 << 0,
			TestBigger = 1 << 1,
			TestEqual = 1 << 2,
			TestBiggerEqual = 1 << 3,
			TestSmallerEqual = 1 << 4,
			TestUnequal = 1 << 5,
		};

		uint64_t instrCount = 0;
		uint64_t nextInstrCountInterrupt = 0;
		uint32_t controlByte = 0;
		std::array<uint32_t, 64> registers;

		uint32_t* program;

		bool running = true;

		uint32_t& stackPtr = registers[63];
		uint32_t& instrPtr = registers[62];
		uint32_t& basePtr = registers[61];
		uint32_t& loopPtr = registers[60];

		static constexpr int uint8_tmax = std::numeric_limits<uint8_t>::max() + 1;

		cmn::FixedVector<std::pair<uint32_t, uint32_t>, uint8_tmax> interrupts;
		cmn::FixedVector<std::pair<uint32_t, uint32_t>, uint8_tmax> interruptsRestore;

		InterruptType handling = InterruptType::NoInterrupt;

		Error errorCode;
		ch::nanoseconds startExecTime;
		ch::nanoseconds endExecTime;

		std::vector<std::pair<std::string, NativeFunc>> natives;
		bool namesSorted = false;

		State innerState;

		int32_t _binary_search(const std::string& val) {
			int32_t lower = 0;
			int32_t upper = static_cast<int32_t>(natives.size());

			while (lower <= upper) {
				int32_t middle = lower + ((upper - lower) / 2);
				std::string s;
				auto cmp = natives[middle].first.compare(val);
				if (!cmp)	return middle;
				else if (cmp < 0)	lower = middle + 1;
				else				upper = middle - 1;
			}

			return -1;
		}

		int32_t _findNativeByName(const std::string& s) {
			if (!namesSorted) {
				finalizeNatives();
			}

			return _binary_search(s);
		}

		void _onInvalidDecode() {
			errorCode = { ErrorCode::InvalidInstruction, instrPtr };
			running = false;
		}

		auto _pushStack(uint32_t value) {
			if (!stackPtr) {
				errorCode = { ErrorCode::StackUnderflow, instrPtr };
				running = false;
				return;
			}

			program[stackPtr - 1] = value;
			stackPtr--;
		};

		auto _popStack() {
			return program[stackPtr++];
		};

		auto _popStackRef(uint32_t& ref) {
			ref = _popStack();
		};

		auto _write(uint32_t* from, uint32_t* to) {
			while (*from)
				*(to++) = *(from++);
		}

		auto _writeN(uint32_t* from, size_t count, uint32_t* to) {
			while (count--)
				*(to++) = *(from++);
		}

		auto _write8(uint8_t* from, uint8_t* to) {
			while (*from)
				*(to++) = *(from++);
		}

		auto _write8N(uint8_t* from, size_t count, uint8_t* to) {
			while (count--)
				*(to++) = *(from++);
		}

		auto _writeTime(uint64_t time, uint32_t* addr) {
			*addr = time & 0xFFFFFFFF;
			*(addr + 1) = (time >> 32) & 0xFFFFFFFF;
		}

		void _setNextInstrCountInt(uint32_t lower, uint32_t upper) {
			nextInstrCountInterrupt = (upper << 32) | lower;
		}

		void _setNextInstrCountInt(uint64_t last, uint32_t lower, uint32_t upper) {
			nextInstrCountInterrupt = last + ((upper << 32) | lower);
		}

		void setControl(uint32_t left, uint32_t right) {
			controlByte = 0;
			if (left > right)		controlByte |= (TestBigger | TestUnequal | TestBiggerEqual);
			else if (left < right)	controlByte |= (TestSmaller | TestUnequal | TestSmallerEqual);
			else					controlByte |= (TestSmallerEqual | TestEqual | TestBiggerEqual);
		}

		void setControl(bool b, ControlFlags flags) {
			controlByte = 0;
			if (b)	controlByte |= flags;
		}

		auto getTime(ch::nanoseconds start) {
			using clock = ch::high_resolution_clock;
			return ch::duration_cast<ch::microseconds>(clock::now().time_since_epoch() - start).count();
		}

		void _doNativeCall(uint32_t code) {
			if (code > natives.size()) {
				errorCode = { ErrorCode::InvalidNativeId, instrPtr };
				running = false;
				return;
			}

			natives[code].second(innerState);
		}

		bool _validateInterruptCode(uint32_t code) {
			if (code >= 255) {
				errorCode = { ErrorCode::InvalidInterruptId, instrPtr };
				running = false;
				return false;
			}
			return true;
		}

		bool _executeInterrupt() {
			if (!running)	return false;
			++instrCount;

			// Literally the entire decode in 2 lines of code:
			auto nextInstr = Instruction::fromStream(program, instrPtr);
			instrPtr += 3;

			switch (nextInstr->mnemonic) {
				case Mnemonic::Ret:
					errorCode = { ErrorCode::RetInInterrupt, instrPtr };
					running = false;
					return false;
				case Mnemonic::IRet:
					return false;
			}

			return _perform(nextInstr);
		}

		int _runInterruptCode(uint32_t code) {
			if (!_validateInterruptCode(code))	return 0;
			else if (!interrupts[code].first)	return 1;
			auto addr = interrupts[code].second;
			if (!addr)	return 0;

			if (handling != InterruptType::NoInterrupt) {
				errorCode = { ErrorCode::NestedInterrupt, instrPtr };
				running = false;
				return 0;
			}

			handling = static_cast<InterruptType>(code);
			//auto oldInstr = instrPtr;
			auto regCpy = registers;
			instrPtr = addr;

			/*//Store registers on stack
			for (size_t i = 0, j = registers.size(); i < j; ++i) {
				_pushStack(registers[i]);
			}
			*/

			while (_executeInterrupt()) {
			}

			bool wasEnd = Instruction::fromStream(program, instrPtr - 3)->mnemonic == Mnemonic::End;
			//Restore registers
			registers = regCpy;

			if (wasEnd)
				return 2;

			if (!running)	return 0;

			/*
			for (int32_t i = static_cast<int32_t>(registers.size()) - 1, j = 0; i >= j; --i) {
				registers[i] = _popStack();
			}
			*/

			//instrPtr = oldInstr;
			handling = InterruptType::NoInterrupt;
			return 1;
		}

		void _setInterruptEnabled(uint32_t code, bool enabled) {
			if (!_validateInterruptCode(code))	return;
			interrupts[code].first = enabled;
		}

		void _setInterruptHandler(uint32_t code, uint32_t addr) {
			if (!_validateInterruptCode(code))	return;
			interrupts[code].second = addr;
			interrupts[code].first = true;
		}

		bool _execute() {
			if (!running)	return false;

			++instrCount;
			
			if (nextInstrCountInterrupt && instrCount > nextInstrCountInterrupt) {
				if (handling != InterruptType::NoInterrupt) {
					nextInstrCountInterrupt++;
				}
				else {
					int b = _runInterruptCode(static_cast<uint32_t>(InterruptType::InstrCount));
					nextInstrCountInterrupt = 0;
					if (!b) {
						errorCode = { ErrorCode::UnhandledTimeInterrupt, instrPtr };
						running = false;
						return false;
					}
					else if (b == 2)
						return false;
				}
			}

			// Literally the entire decode in 2 lines of code:
			auto nextInstr = Instruction::fromStream(program, instrPtr);
			instrPtr += 3;

			return _perform(nextInstr);
		}

		bool _perform(Instruction* nextInstr) {
			switch (nextInstr->mnemonic) {
				case Mnemonic::Nop:
				case Mnemonic::Dealloc_R:
				case Mnemonic::Dealloc_A:
				case Mnemonic::Dealloc_I:
				case Mnemonic::Alloc_R_R:
				case Mnemonic::Alloc_R_A:
				case Mnemonic::Alloc_R_I:
				case Mnemonic::Alloc_R_V:
				case Mnemonic::Alloc_A_R:
				case Mnemonic::Alloc_A_A:
				case Mnemonic::Alloc_A_I:
				case Mnemonic::Alloc_A_V:
				case Mnemonic::Alloc_I_R:
				case Mnemonic::Alloc_I_A:
				case Mnemonic::Alloc_I_I:
				case Mnemonic::Alloc_I_V:
				case Mnemonic::Vcall_R_R:
				case Mnemonic::Vcall_R_A:
				case Mnemonic::Vcall_R_I:
				case Mnemonic::Vcall_R_V:
				case Mnemonic::Vcall_A_R:
				case Mnemonic::Vcall_A_A:
				case Mnemonic::Vcall_A_I:
				case Mnemonic::Vcall_A_V:
				case Mnemonic::Vcall_I_R:
				case Mnemonic::Vcall_I_A:
				case Mnemonic::Vcall_I_I:
				case Mnemonic::Vcall_I_V:
				case Mnemonic::Vcall_V_R:
				case Mnemonic::Vcall_V_A:
				case Mnemonic::Vcall_V_I:
				case Mnemonic::Vcall_V_V:
				case Mnemonic::IRet:

					break;
				case Mnemonic::Halt:
				case Mnemonic::End:
					return false;
				case Mnemonic::Ret:
					instrPtr = _popStack();
					break;
				case Mnemonic::Loop:
					_pushStack(loopPtr);
					loopPtr = instrPtr;
					break;
				case Mnemonic::Endloop:
					if (--registers[10]) {
						instrPtr = loopPtr;
					}
					else {
						loopPtr = _popStack();
					}
					break;
				case Mnemonic::Push_R:
					_pushStack(registers[nextInstr->arg1]);
					break;
				case Mnemonic::Push_A:
					_pushStack(program[nextInstr->arg1]);
					break;
				case Mnemonic::Push_I:
					_pushStack(program[registers[nextInstr->arg1]]);
					break;
				case Mnemonic::Push_V:
					_pushStack(nextInstr->arg1);
					break;
				case Mnemonic::Pop_R:
					registers[nextInstr->arg1] = _popStack();
					break;
				case Mnemonic::Pop_A:
					program[nextInstr->arg1] = _popStack();
					break;
				case Mnemonic::Pop_I:
					program[registers[nextInstr->arg1]] = _popStack();
					break;
				case Mnemonic::Inc_R:
					registers[nextInstr->arg1]++;
					break;
				case Mnemonic::Inc_A:
					program[nextInstr->arg1]++;
					break;
				case Mnemonic::Inc_I:
					program[registers[nextInstr->arg1]]++;
					break;
				case Mnemonic::Dec_R:
					registers[nextInstr->arg1]--;
					break;
				case Mnemonic::Dec_A:
					program[nextInstr->arg1]--;
					break;
				case Mnemonic::Dec_I:
					program[registers[nextInstr->arg1]]--;
					break;
				case Mnemonic::Call_R:
					_pushStack(instrPtr);
					instrPtr = registers[nextInstr->arg1];
					break;
				case Mnemonic::Call_A:
					_pushStack(instrPtr);
					instrPtr = nextInstr->arg1;
					break;
				case Mnemonic::Call_I:
					_pushStack(instrPtr);
					instrPtr = program[registers[nextInstr->arg1]];
					break;
				case Mnemonic::RCall_R:
					_pushStack(instrPtr);
					instrPtr += registers[nextInstr->arg1];
					break;
				case Mnemonic::RCall_A:
					_pushStack(instrPtr);
					instrPtr += nextInstr->arg1;
					break;
				case Mnemonic::RCall_I:
					_pushStack(instrPtr);
					instrPtr += program[registers[nextInstr->arg1]];
					break;
				case Mnemonic::Read_R:
					std::cin >> registers[nextInstr->arg1];
					break;
				case Mnemonic::Read_A:
					std::cin >> program[nextInstr->arg1];
					break;
				case Mnemonic::Read_I:
					std::cin >> program[registers[nextInstr->arg1]];
					break;
				case Mnemonic::Readstr_A:
					std::cin >> reinterpret_cast<char*>(&program[nextInstr->arg1]);
					break;
				case Mnemonic::Readstr_I:
					std::cin >> reinterpret_cast<char*>(&program[registers[nextInstr->arg1]]);
					break;
				case Mnemonic::Print_R:
					std::cout << registers[nextInstr->arg1];
					break;
				case Mnemonic::Print_A:
					std::cout << program[nextInstr->arg1];
					break;
				case Mnemonic::Print_I:
					std::cout << program[registers[nextInstr->arg1]];
					break;
				case Mnemonic::Print_V:
					std::cout << nextInstr->arg1;
					break;
				case Mnemonic::PrintS_R:
					std::cout << static_cast<int32_t>(registers[nextInstr->arg1]);
					break;
				case Mnemonic::PrintS_A:
					std::cout << static_cast<int32_t>(program[nextInstr->arg1]);
					break;
				case Mnemonic::PrintS_I:
					std::cout << static_cast<int32_t>(program[registers[nextInstr->arg1]]);
					break;
				case Mnemonic::PrintS_V:
					std::cout << static_cast<int32_t>(nextInstr->arg1);
					break;
				case Mnemonic::Printstr_A:
					std::cout << reinterpret_cast<char*>(&program[nextInstr->arg1]);
					break;
				case Mnemonic::Printstr_I:
					std::cout << reinterpret_cast<char*>(&program[registers[nextInstr->arg1]]);
					break;
				case Mnemonic::Jmp_R:
					instrPtr = registers[nextInstr->arg1];
					break;
				case Mnemonic::Jmp_A:
					instrPtr = nextInstr->arg1;
					break;
				case Mnemonic::Jmp_I:
					instrPtr = program[registers[nextInstr->arg1]];
					break;
				case Mnemonic::Jb_R:
				case Mnemonic::Jnle_R:
					if (controlByte & TestBigger) {
						instrPtr = registers[nextInstr->arg1];
					}
					break;
				case Mnemonic::Jb_A:
				case Mnemonic::Jnle_A:
					if (controlByte & TestBigger) {
						instrPtr = nextInstr->arg1;
					}
					break;
				case Mnemonic::Jb_I:
				case Mnemonic::Jnle_I:
					if (controlByte & TestBigger) {
						instrPtr = program[registers[nextInstr->arg1]];
					}
					break;
				case Mnemonic::Jnb_R:
				case Mnemonic::Jle_R:
					if (controlByte & TestSmallerEqual) {
						instrPtr = registers[nextInstr->arg1];
					}
					break;
				case Mnemonic::Jnb_A:
				case Mnemonic::Jle_A:
					if (controlByte & TestSmallerEqual) {
						instrPtr = nextInstr->arg1;
					}
					break;
				case Mnemonic::Jnb_I:
				case Mnemonic::Jle_I:
					if (controlByte & TestSmallerEqual) {
						instrPtr = program[registers[nextInstr->arg1]];
					}
					break;
				case Mnemonic::Jbe_R:
				case Mnemonic::Jnl_R:
					if (controlByte & TestBiggerEqual) {
						instrPtr = registers[nextInstr->arg1];
					}
					break;
				case Mnemonic::Jbe_A:
				case Mnemonic::Jnl_A:
					if (controlByte & TestBiggerEqual) {
						instrPtr = nextInstr->arg1;
					}
					break;
				case Mnemonic::Jbe_I:
				case Mnemonic::Jnl_I:
					if (controlByte & TestBiggerEqual) {
						instrPtr = program[registers[nextInstr->arg1]];
					}
					break;
				case Mnemonic::Jnbe_R:
				case Mnemonic::Jl_R:
					if (controlByte & TestSmaller) {
						instrPtr = registers[nextInstr->arg1];
					}
					break;
				case Mnemonic::Jnbe_A:
				case Mnemonic::Jl_A:
					if (controlByte & TestSmaller) {
						instrPtr = nextInstr->arg1;
					}
					break;
				case Mnemonic::Jnbe_I:
				case Mnemonic::Jl_I:
					if (controlByte & TestSmaller) {
						instrPtr = program[registers[nextInstr->arg1]];
					}
					break;
				case Mnemonic::Jz_R:
				case Mnemonic::Je_R:
					if (controlByte & TestEqual) {
						instrPtr = registers[nextInstr->arg1];
					}
					break;
				case Mnemonic::Jz_A:
				case Mnemonic::Je_A:
					if (controlByte & TestEqual) {
						instrPtr = nextInstr->arg1;
					}
					break;
				case Mnemonic::Jz_I:
				case Mnemonic::Je_I:
					if (controlByte & TestEqual) {
						instrPtr = program[registers[nextInstr->arg1]];
					}
					break;
				case Mnemonic::Jnz_R:
				case Mnemonic::Jne_R:
					if (controlByte & TestUnequal) {
						instrPtr = registers[nextInstr->arg1];
					}
					break;
				case Mnemonic::Jnz_A:
				case Mnemonic::Jne_A:
					if (controlByte & TestUnequal) {
						instrPtr = nextInstr->arg1;
					}
					break;
				case Mnemonic::Jnz_I:
				case Mnemonic::Jne_I:
					if (controlByte & TestUnequal) {
						instrPtr = program[registers[nextInstr->arg1]];
					}
					break;
				case Mnemonic::RJmp_R:
					instrPtr += registers[nextInstr->arg1];
					break;
				case Mnemonic::RJmp_A:
					instrPtr += nextInstr->arg1;
					break;
				case Mnemonic::RJmp_I:
					instrPtr += program[registers[nextInstr->arg1]];
					break;
				case Mnemonic::RJb_R:
				case Mnemonic::RJnle_R:
					if (controlByte & TestBigger) {
						instrPtr += registers[nextInstr->arg1];
					}
					break;
				case Mnemonic::RJb_A:
				case Mnemonic::RJnle_A:
					if (controlByte & TestBigger) {
						instrPtr += nextInstr->arg1;
					}
					break;
				case Mnemonic::RJb_I:
				case Mnemonic::RJnle_I:
					if (controlByte & TestBigger) {
						instrPtr += program[registers[nextInstr->arg1]];
					}
					break;
				case Mnemonic::RJnb_R:
				case Mnemonic::RJle_R:
					if (controlByte & TestSmallerEqual) {
						instrPtr += registers[nextInstr->arg1];
					}
					break;
				case Mnemonic::RJnb_A:
				case Mnemonic::RJle_A:
					if (controlByte & TestSmallerEqual) {
						instrPtr += nextInstr->arg1;
					}
					break;
				case Mnemonic::RJnb_I:
				case Mnemonic::RJle_I:
					if (controlByte & TestSmallerEqual) {
						instrPtr += program[registers[nextInstr->arg1]];
					}
					break;
				case Mnemonic::RJbe_R:
				case Mnemonic::RJnl_R:
					if (controlByte & TestBiggerEqual) {
						instrPtr += registers[nextInstr->arg1];
					}
					break;
				case Mnemonic::RJbe_A:
				case Mnemonic::RJnl_A:
					if (controlByte & TestBiggerEqual) {
						instrPtr += nextInstr->arg1;
					}
					break;
				case Mnemonic::RJbe_I:
				case Mnemonic::RJnl_I:
					if (controlByte & TestBiggerEqual) {
						instrPtr += program[registers[nextInstr->arg1]];
					}
					break;
				case Mnemonic::RJnbe_R:
				case Mnemonic::RJl_R:
					if (controlByte & TestSmaller) {
						instrPtr += registers[nextInstr->arg1];
					}
					break;
				case Mnemonic::RJnbe_A:
				case Mnemonic::RJl_A:
					if (controlByte & TestSmaller) {
						instrPtr += nextInstr->arg1;
					}
					break;
				case Mnemonic::RJnbe_I:
				case Mnemonic::RJl_I:
					if (controlByte & TestSmaller) {
						instrPtr += program[registers[nextInstr->arg1]];
					}
					break;
				case Mnemonic::RJz_R:
				case Mnemonic::RJe_R:
					if (controlByte & TestEqual) {
						instrPtr += registers[nextInstr->arg1];
					}
					break;
				case Mnemonic::RJz_A:
				case Mnemonic::RJe_A:
					if (controlByte & TestEqual) {
						instrPtr += nextInstr->arg1;
					}
					break;
				case Mnemonic::RJz_I:
				case Mnemonic::RJe_I:
					if (controlByte & TestEqual) {
						instrPtr += program[registers[nextInstr->arg1]];
					}
					break;
				case Mnemonic::RJnz_R:
				case Mnemonic::RJne_R:
					if (controlByte & TestUnequal) {
						instrPtr += registers[nextInstr->arg1];
					}
					break;
				case Mnemonic::RJnz_A:
				case Mnemonic::RJne_A:
					if (controlByte & TestUnequal) {
						instrPtr += nextInstr->arg1;
					}
					break;
				case Mnemonic::RJnz_I:
				case Mnemonic::RJne_I:
					if (controlByte & TestUnequal) {
						instrPtr += program[registers[nextInstr->arg1]];
					}
					break;
				case Mnemonic::Not_R:
					registers[nextInstr->arg1] = ~registers[nextInstr->arg1];
					break;
				case Mnemonic::Time_R:
					registers[nextInstr->arg1] = static_cast<uint32_t>(getTime(startExecTime));
					break;
				case Mnemonic::ICount_R:
					registers[nextInstr->arg1] = static_cast<uint32_t>(instrCount);
					break;
				case Mnemonic::Raise_R:
					_runInterruptCode(registers[nextInstr->arg1]);
					break;
				case Mnemonic::Raise_A:
					_runInterruptCode(program[nextInstr->arg1]);
					break;
				case Mnemonic::Raise_I:
					_runInterruptCode(program[registers[nextInstr->arg1]]);
					break;
				case Mnemonic::Raise_V:
					_runInterruptCode(nextInstr->arg1);
					break;
				case Mnemonic::DisableInt_R:
					_setInterruptEnabled(registers[nextInstr->arg1], false);
					break;
				case Mnemonic::DisableInt_V:
					_setInterruptEnabled(nextInstr->arg1, false);
					break;
				case Mnemonic::EnableInt_R:
					_setInterruptEnabled(registers[nextInstr->arg1], true);
					break;
				case Mnemonic::EnableInt_V:
					_setInterruptEnabled(nextInstr->arg1, true);
					break;
				case Mnemonic::ICountInt_R:
					nextInstrCountInterrupt = registers[nextInstr->arg1];
					break;
				case Mnemonic::ICountInt_V:
					nextInstrCountInterrupt = nextInstr->arg1;
					break;
				case Mnemonic::Mov_R_R:
					registers[nextInstr->arg1] = registers[nextInstr->arg2];
					break;
				case Mnemonic::Mov_R_A:
					registers[nextInstr->arg1] = program[nextInstr->arg2];
					break;
				case Mnemonic::Mov_R_I:
					registers[nextInstr->arg1] = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Mov_R_V:
				case Mnemonic::Laddr_R_A:
				case Mnemonic::Laddr_R_I:
					registers[nextInstr->arg1] = nextInstr->arg2;
					break;
				case Mnemonic::Mov_A_R:
					program[nextInstr->arg1] = registers[nextInstr->arg2];
					break;
				case Mnemonic::Mov_A_A:
					program[nextInstr->arg1] = program[nextInstr->arg2];
					break;
				case Mnemonic::Mov_A_I:
					program[nextInstr->arg1] = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Mov_A_V:
				case Mnemonic::Laddr_A_A:
				case Mnemonic::Laddr_A_I:
					program[nextInstr->arg1] = nextInstr->arg2;
					break;
				case Mnemonic::Mov_I_R:
					program[registers[nextInstr->arg1]] = registers[nextInstr->arg2];
					break;
				case Mnemonic::Mov_I_A:
					program[registers[nextInstr->arg1]] = program[nextInstr->arg2];
					break;
				case Mnemonic::Mov_I_I:
					program[registers[nextInstr->arg1]] = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Mov_I_V:
				case Mnemonic::Laddr_I_A:
				case Mnemonic::Laddr_I_I:
					program[registers[nextInstr->arg1]] = nextInstr->arg2;
					break;
				case Mnemonic::Move_R_R:
				case Mnemonic::Movz_R_R:
					if (controlByte & TestEqual) {
						registers[nextInstr->arg1] = registers[nextInstr->arg2];
					}
					break;
				case Mnemonic::Move_R_A:
				case Mnemonic::Movz_R_A:
					if (controlByte & TestEqual) {
						registers[nextInstr->arg1] = program[nextInstr->arg2];
					}
					break;
				case Mnemonic::Move_R_I:
				case Mnemonic::Movz_R_I:
					if (controlByte & TestEqual) {
						registers[nextInstr->arg1] = program[registers[nextInstr->arg2]];
					}
					break;
				case Mnemonic::Move_R_V:
				case Mnemonic::Movz_R_V:
					if (controlByte & TestEqual) {
						registers[nextInstr->arg1] = nextInstr->arg2;
					}
					break;
				case Mnemonic::Move_A_R:
				case Mnemonic::Movz_A_R:
					if (controlByte & TestEqual) {
						program[nextInstr->arg1] = registers[nextInstr->arg2];
					}
					break;
				case Mnemonic::Move_A_A:
				case Mnemonic::Movz_A_A:
					if (controlByte & TestEqual) {
						program[nextInstr->arg1] = program[nextInstr->arg2];
					}
					break;
				case Mnemonic::Move_A_I:
				case Mnemonic::Movz_A_I:
					if (controlByte & TestEqual) {
						program[nextInstr->arg1] = program[registers[nextInstr->arg2]];
					}
					break;
				case Mnemonic::Move_A_V:
				case Mnemonic::Movz_A_V:
					if (controlByte & TestEqual) {
						program[nextInstr->arg1] = nextInstr->arg2;
					}
					break;
				case Mnemonic::Move_I_R:
				case Mnemonic::Movz_I_R:
					if (controlByte & TestEqual) {
						program[registers[nextInstr->arg1]] = registers[nextInstr->arg2];
					}
					break;
				case Mnemonic::Move_I_A:
				case Mnemonic::Movz_I_A:
					if (controlByte & TestEqual) {
						program[registers[nextInstr->arg1]] = program[nextInstr->arg2];
					}
					break;
				case Mnemonic::Move_I_I:
				case Mnemonic::Movz_I_I:
					if (controlByte & TestEqual) {
						program[registers[nextInstr->arg1]] = program[registers[nextInstr->arg2]];
					}
					break;
				case Mnemonic::Move_I_V:
				case Mnemonic::Movz_I_V:
					if (controlByte & TestEqual) {
						program[registers[nextInstr->arg1]] = nextInstr->arg2;
					}
					break;
				case Mnemonic::Movne_R_R:
				case Mnemonic::Movnz_R_R:
					if (controlByte & TestUnequal) {
						registers[nextInstr->arg1] = registers[nextInstr->arg2];
					}
					break;
				case Mnemonic::Movne_R_A:
				case Mnemonic::Movnz_R_A:
					if (controlByte & TestUnequal) {
						registers[nextInstr->arg1] = program[nextInstr->arg2];
					}
					break;
				case Mnemonic::Movne_R_I:
				case Mnemonic::Movnz_R_I:
					if (controlByte & TestUnequal) {
						registers[nextInstr->arg1] = program[registers[nextInstr->arg2]];
					}
					break;
				case Mnemonic::Movne_R_V:
				case Mnemonic::Movnz_R_V:
					if (controlByte & TestUnequal) {
						registers[nextInstr->arg1] = nextInstr->arg2;
					}
					break;
				case Mnemonic::Movne_A_R:
				case Mnemonic::Movnz_A_R:
					if (controlByte & TestUnequal) {
						program[nextInstr->arg1] = registers[nextInstr->arg2];
					}
					break;
				case Mnemonic::Movne_A_A:
				case Mnemonic::Movnz_A_A:
					if (controlByte & TestUnequal) {
						program[nextInstr->arg1] = program[nextInstr->arg2];
					}
					break;
				case Mnemonic::Movne_A_I:
				case Mnemonic::Movnz_A_I:
					if (controlByte & TestUnequal) {
						program[nextInstr->arg1] = program[registers[nextInstr->arg2]];
					}
					break;
				case Mnemonic::Movne_A_V:
				case Mnemonic::Movnz_A_V:
					if (controlByte & TestUnequal) {
						program[nextInstr->arg1] = nextInstr->arg2;
					}
					break;
				case Mnemonic::Movne_I_R:
				case Mnemonic::Movnz_I_R:
					if (controlByte & TestUnequal) {
						program[registers[nextInstr->arg1]] = registers[nextInstr->arg2];
					}
					break;
				case Mnemonic::Movne_I_A:
				case Mnemonic::Movnz_I_A:
					if (controlByte & TestUnequal) {
						program[registers[nextInstr->arg1]] = program[nextInstr->arg2];
					}
					break;
				case Mnemonic::Movne_I_I:
				case Mnemonic::Movnz_I_I:
					if (controlByte & TestUnequal) {
						program[registers[nextInstr->arg1]] = program[registers[nextInstr->arg2]];
					}
					break;
				case Mnemonic::Movne_I_V:
				case Mnemonic::Movnz_I_V:
					if (controlByte & TestUnequal) {
						program[registers[nextInstr->arg1]] = nextInstr->arg2;
					}
					break;
				case Mnemonic::Add_R_R:
					registers[nextInstr->arg1] += registers[nextInstr->arg2];
					break;
				case Mnemonic::Add_R_I:
					registers[nextInstr->arg1] += program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Add_R_A:
					registers[nextInstr->arg1] += program[nextInstr->arg2];
					break;
				case Mnemonic::Add_R_V:
					registers[nextInstr->arg1] += nextInstr->arg2;
					break;
				case Mnemonic::Add_I_R:
					program[registers[nextInstr->arg1]] += registers[nextInstr->arg2];
					break;
				case Mnemonic::Add_I_I:
					program[registers[nextInstr->arg1]] += program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Add_I_A:
					program[registers[nextInstr->arg1]] += program[nextInstr->arg2];
					break;
				case Mnemonic::Add_I_V:
					program[registers[nextInstr->arg1]] += nextInstr->arg2;
					break;
				case Mnemonic::Add_A_R:
					program[nextInstr->arg1] += registers[nextInstr->arg2];
					break;
				case Mnemonic::Add_A_I:
					program[nextInstr->arg1] += program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Add_A_A:
					program[nextInstr->arg1] += program[nextInstr->arg2];
					break;
				case Mnemonic::Add_A_V:
					program[nextInstr->arg1] += nextInstr->arg2;
					break;
				case Mnemonic::Sub_R_R:
					registers[nextInstr->arg1] -= registers[nextInstr->arg2];
					break;
				case Mnemonic::Sub_R_A:
					registers[nextInstr->arg1] -= program[nextInstr->arg2];
					break;
				case Mnemonic::Sub_R_I:
					registers[nextInstr->arg1] -= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Sub_R_V:
					registers[nextInstr->arg1] -= nextInstr->arg2;
					break;
				case Mnemonic::Sub_A_R:
					program[nextInstr->arg1] -= registers[nextInstr->arg2];
					break;
				case Mnemonic::Sub_A_A:
					program[nextInstr->arg1] -= program[nextInstr->arg2];
					break;
				case Mnemonic::Sub_A_I:
					program[nextInstr->arg1] -= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Sub_A_V:
					program[nextInstr->arg1] -= nextInstr->arg2;
					break;
				case Mnemonic::Sub_I_R:
					program[registers[nextInstr->arg1]] -= registers[nextInstr->arg2];
					break;
				case Mnemonic::Sub_I_A:
					program[registers[nextInstr->arg1]] -= program[nextInstr->arg2];
					break;
				case Mnemonic::Sub_I_I:
					program[registers[nextInstr->arg1]] -= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Sub_I_V:
					program[registers[nextInstr->arg1]] -= nextInstr->arg2;
					break;
				case Mnemonic::Mul_R_R:
					registers[nextInstr->arg1] *= registers[nextInstr->arg2];
					break;
				case Mnemonic::Mul_R_A:
					registers[nextInstr->arg1] *= program[nextInstr->arg2];
					break;
				case Mnemonic::Mul_R_I:
					registers[nextInstr->arg1] *= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Mul_R_V:
					registers[nextInstr->arg1] *= nextInstr->arg2;
					break;
				case Mnemonic::Mul_A_R:
					program[nextInstr->arg1] *= registers[nextInstr->arg2];
					break;
				case Mnemonic::Mul_A_A:
					program[nextInstr->arg1] *= program[nextInstr->arg2];
					break;
				case Mnemonic::Mul_A_I:
					program[nextInstr->arg1] *= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Mul_A_V:
					program[nextInstr->arg1] *= nextInstr->arg2;
					break;
				case Mnemonic::Mul_I_R:
					program[registers[nextInstr->arg1]] *= registers[nextInstr->arg2];
					break;
				case Mnemonic::Mul_I_A:
					program[registers[nextInstr->arg1]] *= program[nextInstr->arg2];
					break;
				case Mnemonic::Mul_I_I:
					program[registers[nextInstr->arg1]] *= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Mul_I_V:
					program[registers[nextInstr->arg1]] *= nextInstr->arg2;
					break;
				case Mnemonic::Div_R_R:
					registers[nextInstr->arg1] /= registers[nextInstr->arg2];
					break;
				case Mnemonic::Div_R_A:
					registers[nextInstr->arg1] /= program[nextInstr->arg2];
					break;
				case Mnemonic::Div_R_I:
					registers[nextInstr->arg1] /= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Div_R_V:
					registers[nextInstr->arg1] /= nextInstr->arg2;
					break;
				case Mnemonic::Div_A_R:
					program[nextInstr->arg1] /= registers[nextInstr->arg2];
					break;
				case Mnemonic::Div_A_A:
					program[nextInstr->arg1] /= program[nextInstr->arg2];
					break;
				case Mnemonic::Div_A_I:
					program[nextInstr->arg1] /= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Div_A_V:
					program[nextInstr->arg1] /= nextInstr->arg2;
					break;
				case Mnemonic::Div_I_R:
					program[registers[nextInstr->arg1]] /= registers[nextInstr->arg2];
					break;
				case Mnemonic::Div_I_A:
					program[registers[nextInstr->arg1]] /= program[nextInstr->arg2];
					break;
				case Mnemonic::Div_I_I:
					program[registers[nextInstr->arg1]] /= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Div_I_V:
					program[registers[nextInstr->arg1]] /= nextInstr->arg2;
					break;
				case Mnemonic::Mod_R_R:
					registers[nextInstr->arg1] %= registers[nextInstr->arg2];
					break;
				case Mnemonic::Mod_R_A:
					registers[nextInstr->arg1] %= program[nextInstr->arg2];
					break;
				case Mnemonic::Mod_R_I:
					registers[nextInstr->arg1] %= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Mod_R_V:
					registers[nextInstr->arg1] %= nextInstr->arg2;
					break;
				case Mnemonic::Mod_A_R:
					program[nextInstr->arg1] %= registers[nextInstr->arg2];
					break;
				case Mnemonic::Mod_A_A:
					program[nextInstr->arg1] %= program[nextInstr->arg2];
					break;
				case Mnemonic::Mod_A_I:
					program[nextInstr->arg1] %= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Mod_A_V:
					program[nextInstr->arg1] %= nextInstr->arg2;
					break;
				case Mnemonic::Mod_I_R:
					program[registers[nextInstr->arg1]] %= registers[nextInstr->arg2];
					break;
				case Mnemonic::Mod_I_A:
					program[registers[nextInstr->arg1]] %= program[nextInstr->arg2];
					break;
				case Mnemonic::Mod_I_I:
					program[registers[nextInstr->arg1]] %= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Mod_I_V:
					program[registers[nextInstr->arg1]] %= nextInstr->arg2;
					break;
				case Mnemonic::Test_R_R:
					setControl(registers[nextInstr->arg1], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Test_R_A:
					setControl(registers[nextInstr->arg1], program[nextInstr->arg2]);
					break;
				case Mnemonic::Test_R_I:
					setControl(registers[nextInstr->arg1], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Test_R_V:
					setControl(registers[nextInstr->arg1], nextInstr->arg2);
					break;
				case Mnemonic::Test_A_R:
					setControl(program[nextInstr->arg1], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Test_A_A:
					setControl(program[nextInstr->arg1], program[nextInstr->arg2]);
					break;
				case Mnemonic::Test_A_I:
					setControl(program[nextInstr->arg1], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Test_A_V:
					setControl(program[nextInstr->arg1], nextInstr->arg2);
					break;
				case Mnemonic::Test_I_R:
					setControl(program[registers[nextInstr->arg1]], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Test_I_A:
					setControl(program[registers[nextInstr->arg1]], program[nextInstr->arg2]);
					break;
				case Mnemonic::Test_I_I:
					setControl(program[registers[nextInstr->arg1]], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Test_I_V:
					setControl(program[registers[nextInstr->arg1]], nextInstr->arg2);
					break;
				case Mnemonic::Lsh_R_R:
					registers[nextInstr->arg1] <<= registers[nextInstr->arg2];
					break;
				case Mnemonic::Lsh_R_A:
					registers[nextInstr->arg1] <<= program[nextInstr->arg2];
					break;
				case Mnemonic::Lsh_R_I:
					registers[nextInstr->arg1] <<= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Lsh_R_V:
					registers[nextInstr->arg1] <<= nextInstr->arg2;
					break;
				case Mnemonic::Lsh_A_R:
					program[nextInstr->arg1] <<= registers[nextInstr->arg2];
					break;
				case Mnemonic::Lsh_A_A:
					program[nextInstr->arg1] <<= program[nextInstr->arg2];
					break;
				case Mnemonic::Lsh_A_I:
					program[nextInstr->arg1] <<= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Lsh_A_V:
					program[nextInstr->arg1] <<= nextInstr->arg2;
					break;
				case Mnemonic::Lsh_I_R:
					program[registers[nextInstr->arg1]] <<= registers[nextInstr->arg2];
					break;
				case Mnemonic::Lsh_I_A:
					program[registers[nextInstr->arg1]] <<= program[nextInstr->arg2];
					break;
				case Mnemonic::Lsh_I_I:
					program[registers[nextInstr->arg1]] <<= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Lsh_I_V:
					program[registers[nextInstr->arg1]] <<= nextInstr->arg2;
					break;
				case Mnemonic::Rlsh_R_R:
					registers[nextInstr->arg1] = _rotl(registers[nextInstr->arg1], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Rlsh_R_A:
					registers[nextInstr->arg1] = _rotl(registers[nextInstr->arg1], program[nextInstr->arg2]);
					break;
				case Mnemonic::Rlsh_R_I:
					registers[nextInstr->arg1] = _rotl(registers[nextInstr->arg1], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Rlsh_R_V:
					registers[nextInstr->arg1] = _rotl(registers[nextInstr->arg1], nextInstr->arg2);
					break;
				case Mnemonic::Rlsh_A_R:
					program[nextInstr->arg1] = _rotl(program[nextInstr->arg1], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Rlsh_A_A:
					program[nextInstr->arg1] = _rotl(program[nextInstr->arg1], program[nextInstr->arg2]);
					break;
				case Mnemonic::Rlsh_A_I:
					program[nextInstr->arg1] = _rotl(program[nextInstr->arg1], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Rlsh_A_V:
					program[nextInstr->arg1] = _rotl(program[nextInstr->arg1], nextInstr->arg2);
					break;
				case Mnemonic::Rlsh_I_R:
					program[registers[nextInstr->arg1]] = _rotl(program[registers[nextInstr->arg1]], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Rlsh_I_A:
					program[registers[nextInstr->arg1]] = _rotl(program[registers[nextInstr->arg1]], program[nextInstr->arg2]);
					break;
				case Mnemonic::Rlsh_I_I:
					program[registers[nextInstr->arg1]] = _rotl(program[registers[nextInstr->arg1]], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Rlsh_I_V:
					program[registers[nextInstr->arg1]] = _rotl(program[registers[nextInstr->arg1]], nextInstr->arg2);
					break;
				case Mnemonic::Rsh_R_R:
					registers[nextInstr->arg1] >>= registers[nextInstr->arg2];
					break;
				case Mnemonic::Rsh_R_A:
					registers[nextInstr->arg1] >>= program[nextInstr->arg2];
					break;
				case Mnemonic::Rsh_R_I:
					registers[nextInstr->arg1] >>= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Rsh_R_V:
					registers[nextInstr->arg1] >>= nextInstr->arg2;
					break;
				case Mnemonic::Rsh_A_R:
					program[nextInstr->arg1] >>= registers[nextInstr->arg2];
					break;
				case Mnemonic::Rsh_A_A:
					program[nextInstr->arg1] >>= program[nextInstr->arg2];
					break;
				case Mnemonic::Rsh_A_I:
					program[nextInstr->arg1] >>= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Rsh_A_V:
					program[nextInstr->arg1] >>= nextInstr->arg2;
					break;
				case Mnemonic::Rsh_I_R:
					program[registers[nextInstr->arg1]] >>= registers[nextInstr->arg2];
					break;
				case Mnemonic::Rsh_I_A:
					program[registers[nextInstr->arg1]] >>= program[nextInstr->arg2];
					break;
				case Mnemonic::Rsh_I_I:
					program[registers[nextInstr->arg1]] >>= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Rsh_I_V:
					program[registers[nextInstr->arg1]] >>= nextInstr->arg2;
					break;
				case Mnemonic::Rrsh_R_R:
					registers[nextInstr->arg1] = _rotr(registers[nextInstr->arg1], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Rrsh_R_A:
					registers[nextInstr->arg1] = _rotr(registers[nextInstr->arg1], program[nextInstr->arg2]);
					break;
				case Mnemonic::Rrsh_R_I:
					registers[nextInstr->arg1] = _rotr(registers[nextInstr->arg1], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Rrsh_R_V:
					registers[nextInstr->arg1] = _rotr(registers[nextInstr->arg1], nextInstr->arg2);
					break;
				case Mnemonic::Rrsh_A_R:
					program[nextInstr->arg1] = _rotr(program[nextInstr->arg1], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Rrsh_A_A:
					program[nextInstr->arg1] = _rotr(program[nextInstr->arg1], program[nextInstr->arg2]);
					break;
				case Mnemonic::Rrsh_A_I:
					program[nextInstr->arg1] = _rotr(program[nextInstr->arg1], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Rrsh_A_V:
					program[nextInstr->arg1] = _rotr(program[nextInstr->arg1], nextInstr->arg2);
					break;
				case Mnemonic::Rrsh_I_R:
					program[registers[nextInstr->arg1]] = _rotr(program[registers[nextInstr->arg1]], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Rrsh_I_A:
					program[registers[nextInstr->arg1]] = _rotr(program[registers[nextInstr->arg1]], program[nextInstr->arg2]);
					break;
				case Mnemonic::Rrsh_I_I:
					program[registers[nextInstr->arg1]] = _rotr(program[registers[nextInstr->arg1]], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Rrsh_I_V:
					program[registers[nextInstr->arg1]] = _rotr(program[registers[nextInstr->arg1]], nextInstr->arg2);
					break;
				case Mnemonic::And_R_R:
					registers[nextInstr->arg1] &= registers[nextInstr->arg2];
					break;
				case Mnemonic::And_R_A:
					registers[nextInstr->arg1] &= program[nextInstr->arg2];
					break;
				case Mnemonic::And_R_I:
					registers[nextInstr->arg1] &= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::And_R_V:
					registers[nextInstr->arg1] &= nextInstr->arg2;
					break;
				case Mnemonic::And_A_R:
					program[nextInstr->arg1] &= registers[nextInstr->arg2];
					break;
				case Mnemonic::And_A_A:
					program[nextInstr->arg1] &= program[nextInstr->arg2];
					break;
				case Mnemonic::And_A_I:
					program[nextInstr->arg1] &= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::And_A_V:
					program[nextInstr->arg1] &= nextInstr->arg2;
					break;
				case Mnemonic::And_I_R:
					program[registers[nextInstr->arg1]] &= registers[nextInstr->arg2];
					break;
				case Mnemonic::And_I_A:
					program[registers[nextInstr->arg1]] &= program[nextInstr->arg2];
					break;
				case Mnemonic::And_I_I:
					program[registers[nextInstr->arg1]] &= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::And_I_V:
					program[registers[nextInstr->arg1]] &= nextInstr->arg2;
					break;
				case Mnemonic::Or_R_R:
					registers[nextInstr->arg1] |= registers[nextInstr->arg2];
					break;
				case Mnemonic::Or_R_A:
					registers[nextInstr->arg1] |= program[nextInstr->arg2];
					break;
				case Mnemonic::Or_R_I:
					registers[nextInstr->arg1] |= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Or_R_V:
					registers[nextInstr->arg1] |= nextInstr->arg2;
					break;
				case Mnemonic::Or_A_R:
					program[nextInstr->arg1] |= registers[nextInstr->arg2];
					break;
				case Mnemonic::Or_A_A:
					program[nextInstr->arg1] |= program[nextInstr->arg2];
					break;
				case Mnemonic::Or_A_I:
					program[nextInstr->arg1] |= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Or_A_V:
					program[nextInstr->arg1] |= nextInstr->arg2;
					break;
				case Mnemonic::Or_I_R:
					program[registers[nextInstr->arg1]] |= registers[nextInstr->arg2];
					break;
				case Mnemonic::Or_I_A:
					program[registers[nextInstr->arg1]] |= program[nextInstr->arg2];
					break;
				case Mnemonic::Or_I_I:
					program[registers[nextInstr->arg1]] |= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Or_I_V:
					program[registers[nextInstr->arg1]] |= nextInstr->arg2;
					break;
				case Mnemonic::Xor_R_R:
					registers[nextInstr->arg1] ^= registers[nextInstr->arg2];
					break;
				case Mnemonic::Xor_R_A:
					registers[nextInstr->arg1] ^= program[nextInstr->arg2];
					break;
				case Mnemonic::Xor_R_I:
					registers[nextInstr->arg1] ^= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Xor_R_V:
					registers[nextInstr->arg1] ^= nextInstr->arg2;
					break;
				case Mnemonic::Xor_A_R:
					program[nextInstr->arg1] ^= registers[nextInstr->arg2];
					break;
				case Mnemonic::Xor_A_A:
					program[nextInstr->arg1] ^= program[nextInstr->arg2];
					break;
				case Mnemonic::Xor_A_I:
					program[nextInstr->arg1] ^= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Xor_A_V:
					program[nextInstr->arg1] ^= nextInstr->arg2;
					break;
				case Mnemonic::Xor_I_R:
					program[registers[nextInstr->arg1]] ^= registers[nextInstr->arg2];
					break;
				case Mnemonic::Xor_I_A:
					program[registers[nextInstr->arg1]] ^= program[nextInstr->arg2];
					break;
				case Mnemonic::Xor_I_I:
					program[registers[nextInstr->arg1]] ^= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Xor_I_V:
					program[registers[nextInstr->arg1]] ^= nextInstr->arg2;
					break;
				case Mnemonic::Eq_R_R:
					setControl(registers[nextInstr->arg1] == registers[nextInstr->arg2], TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_R_A:
					setControl(registers[nextInstr->arg1] == program[nextInstr->arg2], TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_R_I:
					setControl(registers[nextInstr->arg1] == program[registers[nextInstr->arg2]], TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_R_V:
					setControl(registers[nextInstr->arg1] == nextInstr->arg2, TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_A_R:
					setControl(program[nextInstr->arg1] == registers[nextInstr->arg2], TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_A_A:
					setControl(program[nextInstr->arg1] == program[nextInstr->arg2], TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_A_I:
					setControl(program[nextInstr->arg1] == program[registers[nextInstr->arg2]], TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_A_V:
					setControl(program[nextInstr->arg1] == nextInstr->arg2, TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_I_R:
					setControl(program[registers[nextInstr->arg1]] == registers[nextInstr->arg2], TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_I_A:
					setControl(program[registers[nextInstr->arg1]] == program[nextInstr->arg2], TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_I_I:
					setControl(program[registers[nextInstr->arg1]] == program[registers[nextInstr->arg2]], TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_I_V:
					setControl(program[registers[nextInstr->arg1]] == nextInstr->arg2, TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Neq_R_R:
					setControl(registers[nextInstr->arg1] != registers[nextInstr->arg2], TestUnequal);
					break;
				case Mnemonic::Neq_R_A:
					setControl(registers[nextInstr->arg1] != program[nextInstr->arg2], TestUnequal);
					break;
				case Mnemonic::Neq_R_I:
					setControl(registers[nextInstr->arg1] != program[registers[nextInstr->arg2]], TestUnequal);
					break;
				case Mnemonic::Neq_R_V:
					setControl(registers[nextInstr->arg1] != nextInstr->arg2, TestUnequal);
					break;
				case Mnemonic::Neq_A_R:
					setControl(program[nextInstr->arg1] != registers[nextInstr->arg2], TestUnequal);
					break;
				case Mnemonic::Neq_A_A:
					setControl(program[nextInstr->arg1] != program[nextInstr->arg2], TestUnequal);
					break;
				case Mnemonic::Neq_A_I:
					setControl(program[nextInstr->arg1] != program[registers[nextInstr->arg2]], TestUnequal);
					break;
				case Mnemonic::Neq_A_V:
					setControl(program[nextInstr->arg1] != nextInstr->arg2, TestUnequal);
					break;
				case Mnemonic::Neq_I_R:
					setControl(program[registers[nextInstr->arg1]] != registers[nextInstr->arg2], TestUnequal);
					break;
				case Mnemonic::Neq_I_A:
					setControl(program[registers[nextInstr->arg1]] != program[nextInstr->arg2], TestUnequal);
					break;
				case Mnemonic::Neq_I_I:
					setControl(program[registers[nextInstr->arg1]] != program[registers[nextInstr->arg2]], TestUnequal);
					break;
				case Mnemonic::Neq_I_V:
					setControl(program[registers[nextInstr->arg1]] != nextInstr->arg2, TestUnequal);
					break;
				case Mnemonic::Lt_R_R:
					setControl(registers[nextInstr->arg1] < registers[nextInstr->arg2], TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_R_A:
					setControl(registers[nextInstr->arg1] < program[nextInstr->arg2], TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_R_I:
					setControl(registers[nextInstr->arg1] < program[registers[nextInstr->arg2]], TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_R_V:
					setControl(registers[nextInstr->arg1] < nextInstr->arg2, TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_A_R:
					setControl(program[nextInstr->arg1] < registers[nextInstr->arg2], TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_A_A:
					setControl(program[nextInstr->arg1] < program[nextInstr->arg2], TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_A_I:
					setControl(program[nextInstr->arg1] < program[registers[nextInstr->arg2]], TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_A_V:
					setControl(program[nextInstr->arg1] < nextInstr->arg2, TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_I_R:
					setControl(program[registers[nextInstr->arg1]] < registers[nextInstr->arg2], TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_I_A:
					setControl(program[registers[nextInstr->arg1]] < program[nextInstr->arg2], TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_I_I:
					setControl(program[registers[nextInstr->arg1]] < program[registers[nextInstr->arg2]], TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_I_V:
					setControl(program[registers[nextInstr->arg1]] < nextInstr->arg2, TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Bt_R_R:
					setControl(registers[nextInstr->arg1] > registers[nextInstr->arg2], TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_R_A:
					setControl(registers[nextInstr->arg1] > program[nextInstr->arg2], TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_R_I:
					setControl(registers[nextInstr->arg1] > program[registers[nextInstr->arg2]], TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_R_V:
					setControl(registers[nextInstr->arg1] > nextInstr->arg2, TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_A_R:
					setControl(program[nextInstr->arg1] > registers[nextInstr->arg2], TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_A_A:
					setControl(program[nextInstr->arg1] > program[nextInstr->arg2], TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_A_I:
					setControl(program[nextInstr->arg1] > program[registers[nextInstr->arg2]], TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_A_V:
					setControl(program[nextInstr->arg1] > nextInstr->arg2, TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_I_R:
					setControl(program[registers[nextInstr->arg1]] > registers[nextInstr->arg2], TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_I_A:
					setControl(program[registers[nextInstr->arg1]] > program[nextInstr->arg2], TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_I_I:
					setControl(program[registers[nextInstr->arg1]] > program[registers[nextInstr->arg2]], TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_I_V:
					setControl(program[registers[nextInstr->arg1]] > nextInstr->arg2, TestBigger | TestUnequal | TestBiggerEqual);
					break;

				//UNHANDLED = INVALID INSTRUCTION
				default:
					return _perform2(nextInstr);
			}

			return true;
		}

		bool _perform2(Instruction* nextInstr) {
			switch (nextInstr->mnemonic) {
				case Mnemonic::Loadload_R_A:
					registers[nextInstr->arg1] = program[program[nextInstr->arg2]];
					break;
				case Mnemonic::Loadload_R_I:
					registers[nextInstr->arg1] = program[program[registers[nextInstr->arg2]]];
					break;
				case Mnemonic::Loadload_A_A:
					program[nextInstr->arg1] = program[program[nextInstr->arg2]];
					break;
				case Mnemonic::Loadload_A_I:
					program[nextInstr->arg1] = program[program[registers[nextInstr->arg2]]];
					break;
				case Mnemonic::Loadload_I_A:
					program[registers[nextInstr->arg1]] = program[program[nextInstr->arg2]];
					break;
				case Mnemonic::Loadload_I_I:
					program[registers[nextInstr->arg1]] = program[program[registers[nextInstr->arg2]]];
					break;
				case Mnemonic::RegInt_R_R:
					_setInterruptHandler(registers[nextInstr->arg1], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Regint_R_I:
					_setInterruptHandler(registers[nextInstr->arg1], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Regint_R_A:
					_setInterruptHandler(registers[nextInstr->arg1], nextInstr->arg2);
					break;
				case Mnemonic::RegInt_A_R:
					_setInterruptHandler(program[nextInstr->arg1], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Regint_A_I:
					_setInterruptHandler(program[nextInstr->arg1], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Regint_A_A:
					_setInterruptHandler(program[nextInstr->arg1], nextInstr->arg2);
					break;
				case Mnemonic::RegInt_I_R:
					_setInterruptHandler(program[registers[nextInstr->arg1]], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Regint_I_I:
					_setInterruptHandler(program[registers[nextInstr->arg1]], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Regint_I_A:
					_setInterruptHandler(program[registers[nextInstr->arg1]], nextInstr->arg2);
					break;
				case Mnemonic::RegInt_V_R:
					_setInterruptHandler(nextInstr->arg1, registers[nextInstr->arg2]);
					break;
				case Mnemonic::Regint_V_I:
					_setInterruptHandler(nextInstr->arg1, program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Regint_V_A:
					_setInterruptHandler(nextInstr->arg1, nextInstr->arg2);
					break;
				case Mnemonic::RRegInt_R_R:
					_setInterruptHandler(registers[nextInstr->arg1], instrPtr + registers[nextInstr->arg2]);
					break;
				case Mnemonic::RRegInt_R_I:
					_setInterruptHandler(registers[nextInstr->arg1], instrPtr + program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::RRegInt_R_A:
					_setInterruptHandler(registers[nextInstr->arg1], instrPtr + nextInstr->arg2);
					break;
				case Mnemonic::RRegInt_A_R:
					_setInterruptHandler(program[nextInstr->arg1], instrPtr + registers[nextInstr->arg2]);
					break;
				case Mnemonic::RRegInt_A_I:
					_setInterruptHandler(program[nextInstr->arg1], instrPtr + program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::RRegInt_A_A:
					_setInterruptHandler(program[nextInstr->arg1], instrPtr + nextInstr->arg2);
					break;
				case Mnemonic::RRegInt_I_R:
					_setInterruptHandler(program[registers[nextInstr->arg1]], instrPtr + registers[nextInstr->arg2]);
					break;
				case Mnemonic::RRegInt_I_I:
					_setInterruptHandler(program[registers[nextInstr->arg1]], instrPtr + program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::RRegInt_I_A:
					_setInterruptHandler(program[registers[nextInstr->arg1]], instrPtr + nextInstr->arg2);
					break;
				case Mnemonic::RRegInt_V_R:
					_setInterruptHandler(nextInstr->arg1, instrPtr + registers[nextInstr->arg2]);
					break;
				case Mnemonic::RRegInt_V_I:
					_setInterruptHandler(nextInstr->arg1, instrPtr + program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::RRegInt_V_A:
					_setInterruptHandler(nextInstr->arg1, instrPtr + nextInstr->arg2);
					break;
				case Mnemonic::Time_A:
					program[nextInstr->arg1] = static_cast<uint32_t>(getTime(startExecTime));
					break;
				case Mnemonic::Time_I:
					program[registers[nextInstr->arg1]] = static_cast<uint32_t>(getTime(startExecTime));
					break;
				case Mnemonic::Time64_R:
					_writeTime(getTime(startExecTime), &registers[nextInstr->arg1]);
					break;
				case Mnemonic::Time64_A:
					_writeTime(getTime(startExecTime), &program[nextInstr->arg1]);
					break;
				case Mnemonic::Time64_I:
					_writeTime(getTime(startExecTime), &program[registers[nextInstr->arg1]]);
					break;
				case Mnemonic::ICount_A:
					program[nextInstr->arg1] = static_cast<uint32_t>(instrCount);
					break;
				case Mnemonic::ICount_I:
					program[registers[nextInstr->arg1]] = static_cast<uint32_t>(instrCount);
					break;
				case Mnemonic::ICount64_R:
					_writeTime(instrCount, &registers[nextInstr->arg1]);
					break;
				case Mnemonic::ICount64_A:
					_writeTime(instrCount, &program[nextInstr->arg1]);
					break;
				case Mnemonic::ICount64_I:
					_writeTime(instrCount, &program[registers[nextInstr->arg1]]);
					break;
				case Mnemonic::NtvCall_R:
					_doNativeCall(registers[nextInstr->arg1]);
					break;
				case Mnemonic::NtvCall_A:
					_doNativeCall(program[nextInstr->arg1]);
					break;
				case Mnemonic::NtvCall_I:
					_doNativeCall(program[registers[nextInstr->arg1]]);
					break;
				case Mnemonic::NtvCall_V:
					_doNativeCall(nextInstr->arg1);
					break;
				case Mnemonic::RICountInt_R:
					nextInstrCountInterrupt = instrCount + registers[nextInstr->arg1];
					break;
				case Mnemonic::RICountInt_V:
					nextInstrCountInterrupt = instrCount + nextInstr->arg1;
					break;
				case Mnemonic::Push_All:
					for (size_t i = 0, j = registers.size(); i < j; ++i) {
						_pushStack(registers[i]);
					}
					break;
				case Mnemonic::Pop_All:
					for (size_t i = registers.size() - 1, j = 0; i >= j; --i) {
						registers[i] = _popStack();
					}
					break;
				case Mnemonic::Clear_All:
					for (size_t i = 0, j = registers.size() - 4; i < j; ++i) {
						registers[i] = 0;
					}
					break;
				case Mnemonic::GetNtvId_R_A:
					registers[nextInstr->arg1] = _findNativeByName(reinterpret_cast<char*>(&program[nextInstr->arg2]));
					break;
				case Mnemonic::GetNtvId_R_I:
					registers[nextInstr->arg1] = _findNativeByName(reinterpret_cast<char*>(&program[registers[nextInstr->arg2]]));
					break;
				case Mnemonic::GetNtvId_A_A:
					program[nextInstr->arg1] = _findNativeByName(reinterpret_cast<char*>(&program[nextInstr->arg2]));
					break;
				case Mnemonic::GetNtvId_A_I:
					program[nextInstr->arg1] = _findNativeByName(reinterpret_cast<char*>(&program[registers[nextInstr->arg2]]));
					break;
				case Mnemonic::GetNtvId_I_A:
					program[registers[nextInstr->arg1]] = _findNativeByName(reinterpret_cast<char*>(&program[nextInstr->arg2]));
					break;
				case Mnemonic::GetNtvId_I_I:
					program[registers[nextInstr->arg1]] = _findNativeByName(reinterpret_cast<char*>(&program[registers[nextInstr->arg2]]));
					break;
				case Mnemonic::Xchg_R_R:
					std::swap(registers[nextInstr->arg1], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Xchg_R_A:
					std::swap(registers[nextInstr->arg1], program[nextInstr->arg2]);
					break;
				case Mnemonic::Xchg_R_I:
					std::swap(registers[nextInstr->arg1], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Xchg_A_R:
					std::swap(program[nextInstr->arg1], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Xchg_A_A:
					std::swap(program[nextInstr->arg1], program[nextInstr->arg2]);
					break;
				case Mnemonic::Xchg_A_I:
					std::swap(program[nextInstr->arg1], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Xchg_I_R:
					std::swap(program[registers[nextInstr->arg1]], registers[nextInstr->arg2]);
					break;
				case Mnemonic::Xchg_I_A:
					std::swap(program[registers[nextInstr->arg1]], program[nextInstr->arg2]);
					break;
				case Mnemonic::Xchg_I_I:
					std::swap(program[registers[nextInstr->arg1]], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::DisableAllInts:
					interruptsRestore = interrupts;
					for (auto& x : interrupts)  x.first = false;
					break;
				case Mnemonic::RestoreInts:
					interrupts = interruptsRestore;
					break;
				case Mnemonic::EnableAllInts:
					for (auto& x : interrupts)  x.first = true;
					break;
				case Mnemonic::ClrCb:
					controlByte = 0;
					break;
				case Mnemonic::ICountInt64_R:
					_setNextInstrCountInt(registers[nextInstr->arg1], *(&registers[nextInstr->arg1] + 1));
					break;
				case Mnemonic::ICountInt64_A:
					_setNextInstrCountInt(program[nextInstr->arg1], *(&program[nextInstr->arg1] + 1));
					break;
				case Mnemonic::ICountInt64_I:
					_setNextInstrCountInt(program[registers[nextInstr->arg1]], *(&program[registers[nextInstr->arg1]] + 1));
					break;
				case Mnemonic::RICountInt64_R:
					_setNextInstrCountInt(nextInstrCountInterrupt, registers[nextInstr->arg1], *(&registers[nextInstr->arg1] + 1));
					break;
				case Mnemonic::RICountInt64_A:
					_setNextInstrCountInt(nextInstrCountInterrupt, program[nextInstr->arg1], *(&program[nextInstr->arg1] + 1));
					break;
				case Mnemonic::RICountInt64_I:
					_setNextInstrCountInt(nextInstrCountInterrupt, program[registers[nextInstr->arg1]], *(&program[registers[nextInstr->arg1]] + 1));
					break;

				//UNHANDLED = INVALID INSTRUCTION
				default:
					_onInvalidDecode();
					break;
			}
			
			return true;
		}

		void _printError() {
			std::cout << formatErrorCode() << "\n";
			std::cout << "While running instruction on address 0x" << std::hex << errorCode.instrPtr << ".\n";
		}
	public:
		VM() : innerState(*this) {}

		template <class T>
		bool run(uint32_t memoffset, T& bytecode) {
			program = &bytecode[0];
			registers.fill(0);
			errorCode = { ErrorCode::None, 0 };
			stackPtr = 1000;
			running = true;
			instrCount = 0;
			controlByte = 0;
			instrPtr = memoffset;
			handling = InterruptType::NoInterrupt;
			nextInstrCountInterrupt = 0;
			interrupts.fill({ 0, 0 });
			
			auto oldSync = std::ios::sync_with_stdio(false);

			startExecTime = ch::high_resolution_clock::now().time_since_epoch();

			while (_execute()) {
			}

			endExecTime = ch::high_resolution_clock::now().time_since_epoch();

			std::ios::sync_with_stdio(oldSync);

			if (!running && errorCode.code != ErrorCode::None) {
				_printError();
			}

			return running;
		}

		uint64_t totalExecuted() const {
			return instrCount;
		}

		auto getStartingTime() const {
			return startExecTime;
		}

		auto getEndingTime() const {
			return endExecTime;
		}

		size_t addNativeFunction(const std::string& identifier, NativeFunc native, bool immediateSort = true) {
			size_t id = natives.size();
			natives.push_back(std::make_pair(identifier, native));
			if (immediateSort)	finalizeNatives();
			namesSorted = !immediateSort;
			return id;
		}

		void removeNativeFunction(const std::string& identifier) {
			removeNativeFunction(_findNativeByName(identifier));
		}

		void removeNativeFunction(size_t index) {
			if (index > natives.size())
				return;
			natives.erase(natives.begin() + index);
		}

		void finalizeNatives() {
			std::sort(natives.begin(), natives.end(), [](const auto& left, const auto& right) {
				return left.first < right.first;
			});

			namesSorted = true;
		}

		Error getError() const {
			return errorCode;
		}

		std::string formatErrorCode() const {
			std::string errorMsg = "Error: ";
			switch (errorCode.code) {
				case ErrorCode::InvalidInstruction:
					errorMsg += "Attempting to execute invalid instruction";
					break;
				case ErrorCode::InvalidInterruptId:
					errorMsg += "Attempting to raise interrupt with id outside of valid range [0, 255]";
					break;
				case ErrorCode::NestedInterrupt:
					errorMsg += "Attempting to raise interrupt inside interrupt handler";
					break;
				case ErrorCode::RetInInterrupt:
					errorMsg += "Ret instruction used inside Interrupt handler instead of IRet";
					break;
				case ErrorCode::UnhandledInterrupt:
					errorMsg += "Interrupt raised that is enabled and has no handler";
					break;
				case ErrorCode::UnhandledTimeInterrupt:
					errorMsg += "Failed to handle interrupt registered with ICountInt";
					break;
				case ErrorCode::InvalidNativeId:
					errorMsg += "Attempt to execute nonexistent Native function";
					break;
				case ErrorCode::StackUnderflow:
					errorMsg += "Ran out of stack storage";
					break;
			}
			errorMsg += '.';
			return errorMsg;
		}
	};

	inline vm::State::State(VM& vm) : instrCount(vm.instrCount), controlByte(vm.controlByte),
		nextInstrCountInterrupt(vm.nextInstrCountInterrupt), registers(vm.registers),
		program(vm.program), stackPtr(vm.stackPtr), instrPtr(vm.instrPtr), basePtr(vm.basePtr),
		loopPtr(vm.loopPtr), handling(vm.handling), errorCode(vm.errorCode), running(vm.running)
	{
	}

	inline uint32_t vm::State::nextParameter() {
		return program[stackPtr++];
	}

	inline std::vector<uint32_t> vm::State::nextParameters(size_t count) {
		std::vector<uint32_t> v;
		while (count--) {
			v.push_back(nextParameter());
		}
		return v;
	}

	inline void vm::State::returnValue(uint32_t value) {
		if (!stackPtr) {
			errorCode = { ErrorCode::StackUnderflow, instrPtr };
			running = false;
		}
		program[--stackPtr] = value;
	}

}

#endif	//BUFFED_VM_HEADER_H_