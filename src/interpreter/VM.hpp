#pragma once

#ifndef BUFFED_VM_HEADER_H_
#define BUFFED_VM_HEADER_H_

#include <vector>
#include <array>
#include <cstdint>
#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>

#include "../common/Instruction.hpp"
#include "../common/FixedVector.hpp"

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

	namespace ch = std::chrono;

	enum class InterruptType : uint8_t {
		InsufficientPrivilege = 250,
		InstrCount = 251,
		UnprivilegedExec = 252,
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
		StackOverrflow,
		UnpirivlegedInstrExec,
		UnprivilegedIntRaise,
		InvalidRegisterId,

		DisabledExtensionUse,

		UnknownError,
	};

	struct Error {
		ErrorCode code;
		uint32_t instrPtr;
	};

	class VM;

	class State {
		VM* vm;
	public:
		explicit State(VM& vm);

		State(const State&) = delete;
		State(State&&) = delete;
		State& operator=(const State&) = delete;
		State& operator=(State&&) = delete;

		uint32_t nextParameter();
		std::vector<uint32_t> nextParameters(size_t count);
		void returnValue(uint32_t value);

		uint32_t instrPtr() const;
		Error currentError() const;
		InterruptType currentInterrupt() const;
		bool isIntEnabled(uint8_t intCode) const;

		uint32_t currentPrivilege() const;
		uint32_t instrPrivilegeRequired(Mnemonic m) const;
		uint32_t interruptPrivilege(uint8_t intCode) const;

		void addArgument(uint32_t arg);
		void runFunction(uint32_t address);
		void runFunction(uint32_t address, uint8_t privilege);
	};

	struct InterruptData {
		uint8_t enabled = 0;
		uint8_t privilege = 255;
		uint8_t privilegeRequired = 0;
		uint32_t addr = 0;
	};

	struct ExtensionData {
		uint8_t privilege = 0;
		uint8_t enabled = true;
	};

	class VM {
	public:
		using NativeFunc = void(*)(vm::State&);
	private:
		friend vm::State;

		enum ControlFlags {
			TestSmaller = 1 << 0,
			TestBigger = 1 << 1,
			TestEqual = 1 << 2,
			TestBiggerEqual = 1 << 3,
			TestSmallerEqual = 1 << 4,
			TestUnequal = 1 << 5,
			TestFloatPositive = 1 << 6,
			TestFloatNegative = 1 << 7,
			TestFloatNan = 1 << 8,
			TestFloatInf = 1 << 9,
		};

		uint64_t instrCount = 0;
		uint64_t nextInstrCountInterrupt = 0;
		uint32_t controlByte = 0;
		std::array<uint32_t, 64> registers;
		std::array<float, 16> fpregisters;

		uint32_t* program;
		uint32_t callDepth;

		bool running = true;
		uint8_t privilegeLevel;

		uint32_t& stackPtr = registers[63];
		uint32_t& instrPtr = registers[62];
		uint32_t& basePtr = registers[61];
		uint32_t& loopPtr = registers[60];

		enum Extensions : uint8_t {
			FloatOperations = 0,

			TotalCount,
		};

		std::array<uint8_t, static_cast<uint32_t>(Mnemonic::TotalCount)> instrPrivileges;
		std::array<ExtensionData, static_cast<uint8_t>(Extensions::TotalCount)> extensionData;

		static constexpr int uint8_tmax = std::numeric_limits<uint8_t>::max() + 1;

		cmn::FixedVector<InterruptData, uint8_tmax> interrupts;
		decltype(interrupts) interruptsRestore;

		InterruptType handling = InterruptType::NoInterrupt;

		Error error;
		ch::nanoseconds startExecTime;
		ch::nanoseconds endExecTime;

		std::vector<std::pair<std::string, NativeFunc>> natives;
		bool namesSorted = false;

		State innerState;

		__forceinline uint32_t& _accessRegister(uint32_t index) {
			if (index >= registers.size()) {
				error = Error{ ErrorCode::InvalidRegisterId, instrPtr };
				running = false;
				throw ErrorCode::InvalidRegisterId;
			}
			return registers[index];
		}

		__forceinline float& _accessFpRegister(uint32_t index) {
			if (index >= fpregisters.size()) {
				error = Error{ ErrorCode::InvalidRegisterId, instrPtr };
				running = false;
				throw ErrorCode::InvalidRegisterId;
			}
			return fpregisters[index];
		}

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

		__forceinline int32_t _findNativeByName(const std::string& s) {
			if (!namesSorted) {
				finalizeNatives();
			}

			return _binary_search(s);
		}

		void _onInvalidDecode() {
			error = { ErrorCode::InvalidInstruction, instrPtr };
			running = false;
		}

		__forceinline auto _pushStack(uint32_t value) {
			if (!stackPtr) {
				error = { ErrorCode::StackOverrflow, instrPtr };
				running = false;
				return;
			}

			program[stackPtr - 1] = value;
			stackPtr--;
		};

		__forceinline auto _popStack() {
			return program[stackPtr++];
		};

		__forceinline auto _popStackRef(uint32_t& ref) {
			ref = _popStack();
		};

		__forceinline auto _write(uint32_t* from, uint32_t* to) {
			while (*from)
				*(to++) = *(from++);
		}

		__forceinline auto _writeN(uint32_t* from, size_t count, uint32_t* to) {
			while (count--)
				*(to++) = *(from++);
		}

		__forceinline auto _write8(uint8_t* from, uint8_t* to) {
			while (*from)
				*(to++) = *(from++);
		}

		__forceinline auto _write8N(uint8_t* from, size_t count, uint8_t* to) {
			while (count--)
				*(to++) = *(from++);
		}

		__forceinline auto _writeTime(uint64_t time, uint32_t* addr) {
			*addr = time & 0xFFFFFFFF;
			*(addr + 1) = (time >> 32) & 0xFFFFFFFF;
		}

		__forceinline void _setNextInstrCountInt(uint32_t lower, uint32_t upper) {
			nextInstrCountInterrupt = (static_cast<uint64_t>(upper) << 32) | lower;
		}

		__forceinline void _setNextInstrCountInt(uint64_t last, uint32_t lower, uint32_t upper) {
			nextInstrCountInterrupt = last + ((static_cast<uint64_t>(upper) << 32) | lower);
		}

		__forceinline void setControl(uint32_t left, uint32_t right) {
			controlByte = 0;
			if (left > right)		controlByte |= (TestBigger | TestUnequal | TestBiggerEqual);
			else if (left < right)	controlByte |= (TestSmaller | TestUnequal | TestSmallerEqual);
			else					controlByte |= (TestSmallerEqual | TestEqual | TestBiggerEqual);
		}

		__forceinline void setFloatControl(float left, float right) {
			controlByte = 0;
			if (left > right)		controlByte |= (TestBigger | TestUnequal | TestBiggerEqual);
			else if (left < right)	controlByte |= (TestSmaller | TestUnequal | TestSmallerEqual);
			else					controlByte |= (TestSmallerEqual | TestEqual | TestBiggerEqual);
		}

		__forceinline void setControl(bool b, ControlFlags flags) {
			controlByte = 0;
			if (b)	controlByte |= flags;
		}

		__forceinline auto getTime(ch::nanoseconds start) {
			using clock = ch::high_resolution_clock;
			return ch::duration_cast<ch::microseconds>(clock::now().time_since_epoch() - start).count();
		}

		__forceinline void _doNativeCall(uint32_t code) {
			if (code > natives.size()) {
				error = { ErrorCode::InvalidNativeId, instrPtr };
				running = false;
				return;
			}

			natives[code].second(innerState);
		}

		__forceinline bool _validateInterruptCode(uint32_t code) {
			if (code >= 255) {
				error = { ErrorCode::InvalidInterruptId, instrPtr };
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
					error = { ErrorCode::RetInInterrupt, instrPtr };
					running = false;
					return false;
				case Mnemonic::IRet:
					return false;
			}

			return _perform(nextInstr);
		}

		int _runInterruptCode(uint32_t code) {
			if (!_validateInterruptCode(code))	return 0;
			else if (!interrupts[code].enabled)	return 1;

			auto addr = interrupts[code].addr;
			if (!addr)	return 0;

			if (handling != InterruptType::NoInterrupt) {
				error = { ErrorCode::NestedInterrupt, instrPtr };
				running = false;
				return 0;
			}

			handling = static_cast<InterruptType>(code);
			//Store registers
			auto regCpy = registers;
			instrPtr = addr;

			//store old privilege
			auto oldPriv = privilegeLevel;
			//set privilege level to the one designated for the interrupt
			privilegeLevel = interrupts[code].privilege;

			while (_executeInterrupt()) {
			}

			bool wasEnd = Instruction::fromStream(program, instrPtr - 3)->mnemonic == Mnemonic::End;

			//Restore registers
			registers = regCpy;
			//Restore privilege
			privilegeLevel = oldPriv;

			if (wasEnd)
				return 2;

			if (!running)	return 0;

			handling = InterruptType::NoInterrupt;
			return 1;
		}

		__forceinline void _setInterruptEnabled(uint32_t code, bool enabled) {
			if (!_validateInterruptCode(code))	return;
			interrupts[code].enabled = enabled;
		}

		__forceinline void _setInterruptHandler(uint32_t code, uint32_t addr) {
			if (!_validateInterruptCode(code))	return;
			interrupts[code].addr = addr;
			interrupts[code].enabled = true;
		}

		__forceinline bool _testPrivilege(uint8_t totest, ErrorCode ec = ErrorCode::UnpirivlegedInstrExec) {
			if (totest > privilegeLevel) {
				auto oldR59 = registers[59];
				registers[59] = totest;
				bool b = _runInterruptCode(static_cast<uint32_t>(InterruptType::InsufficientPrivilege));
				registers[59] = oldR59;
				bool enabled = interrupts[static_cast<uint8_t>(InterruptType::InsufficientPrivilege)].enabled;
				if (!b || !enabled) {
					error = { ec, instrPtr };
					running = false;
					return false;
				}
				return true;
			}
			return true;
		}

		bool _executeFunc(uint32_t startingAddr) {
			if (!running)	return false;

			int nestedDepth = callDepth - 1;
			auto oldInstr = instrPtr;
			instrPtr = startingAddr;
			
			//Need to push because the escape from this is through
			//Ret, which will pop the stack.
			_pushStack(instrPtr);

			while (nestedDepth != callDepth) {
				++instrCount;

				if (nextInstrCountInterrupt && instrCount > nextInstrCountInterrupt) {
					int b = _runInterruptCode(static_cast<uint32_t>(InterruptType::InstrCount));
					nextInstrCountInterrupt = 0;
					if (!b) {
						error = { ErrorCode::UnhandledTimeInterrupt, instrPtr };
						running = false;
						return false;
					}
					else if (b == 2)
						return false;
				}

				// Literally the entire decode in 2 lines of code:
				auto nextInstr = Instruction::fromStream(program, instrPtr);
				instrPtr += 3;

				bool b = _perform(nextInstr);
				if (!b) {
					instrPtr = oldInstr;
					return false;
				}
			}

			instrPtr = oldInstr;
			return true;
		}

		__forceinline bool _executeFunc(uint32_t addr, uint8_t withPrivilege) {
			if (!_testPrivilege(withPrivilege))
				return false;
			auto oldPriv = privilegeLevel;
			privilegeLevel = withPrivilege;
			bool b = _executeFunc(addr);
			privilegeLevel = oldPriv;
			return b;
		}

		bool _execute() {
			if (!running)	return false;

			++instrCount;
			
			if (nextInstrCountInterrupt && instrCount > nextInstrCountInterrupt) {
				int b = _runInterruptCode(static_cast<uint32_t>(InterruptType::InstrCount));
				nextInstrCountInterrupt = 0;
				if (!b) {
					error = { ErrorCode::UnhandledTimeInterrupt, instrPtr };
					running = false;
					return false;
				}
				else if (b == 2)
					return false;
			}

			// Literally the entire decode in 2 lines of code:
			auto nextInstr = Instruction::fromStream(program, instrPtr);
			instrPtr += 3;

			return _perform(nextInstr);
		}

		bool _perform(Instruction* nextInstr) {
			auto mnem_v = static_cast<uint32_t>(nextInstr->mnemonic);
			if (mnem_v >= Mnemonic::TotalCount) {
				_onInvalidDecode();
				return false;
			}
			
			if (!_testPrivilege(instrPrivileges[mnem_v])) {
				return false;
			}

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
				case Mnemonic::IRet:

					break;
				case Mnemonic::Halt:
				case Mnemonic::End:
					return false;
					break;
				case Mnemonic::Ret:
					instrPtr = _popStack();
					callDepth--;
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
					_pushStack(_accessRegister(nextInstr->arg1));
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
					_accessRegister(nextInstr->arg1) = _popStack();
					break;
				case Mnemonic::Pop_A:
					program[nextInstr->arg1] = _popStack();
					break;
				case Mnemonic::Pop_I:
					program[registers[nextInstr->arg1]] = _popStack();
					break;
				case Mnemonic::Inc_R:
					_accessRegister(nextInstr->arg1)++;
					break;
				case Mnemonic::Inc_A:
					program[nextInstr->arg1]++;
					break;
				case Mnemonic::Inc_I:
					program[registers[nextInstr->arg1]]++;
					break;
				case Mnemonic::Dec_R:
					_accessRegister(nextInstr->arg1)--;
					break;
				case Mnemonic::Dec_A:
					program[nextInstr->arg1]--;
					break;
				case Mnemonic::Dec_I:
					program[registers[nextInstr->arg1]]--;
					break;
				case Mnemonic::Call_R:
					_pushStack(instrPtr);
					instrPtr = _accessRegister(nextInstr->arg1);
					++callDepth;
					break;
				case Mnemonic::Call_A:
					_pushStack(instrPtr);
					instrPtr = nextInstr->arg1;
					++callDepth;
					break;
				case Mnemonic::Call_I:
					_pushStack(instrPtr);
					instrPtr = program[registers[nextInstr->arg1]];
					++callDepth;
					break;
				case Mnemonic::RCall_R:
					_pushStack(instrPtr);
					instrPtr += _accessRegister(nextInstr->arg1);
					++callDepth;
					break;
				case Mnemonic::RCall_A:
					_pushStack(instrPtr);
					instrPtr += nextInstr->arg1;
					++callDepth;
					break;
				case Mnemonic::RCall_I:
					_pushStack(instrPtr);
					instrPtr += program[registers[nextInstr->arg1]];
					++callDepth;
					break;
				case Mnemonic::Read_R:
					std::cin >> _accessRegister(nextInstr->arg1);
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
					std::cout << _accessRegister(nextInstr->arg1);
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
					std::cout << static_cast<int32_t>(_accessRegister(nextInstr->arg1));
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
					instrPtr = _accessRegister(nextInstr->arg1);
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
						instrPtr = _accessRegister(nextInstr->arg1);
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
						instrPtr = _accessRegister(nextInstr->arg1);
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
						instrPtr = _accessRegister(nextInstr->arg1);
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
						instrPtr = _accessRegister(nextInstr->arg1);
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
						instrPtr = _accessRegister(nextInstr->arg1);
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
						instrPtr = _accessRegister(nextInstr->arg1);
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
					instrPtr += _accessRegister(nextInstr->arg1);
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
						instrPtr += _accessRegister(nextInstr->arg1);
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
						instrPtr += _accessRegister(nextInstr->arg1);
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
						instrPtr += _accessRegister(nextInstr->arg1);
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
						instrPtr += _accessRegister(nextInstr->arg1);
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
						instrPtr += _accessRegister(nextInstr->arg1);
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
						instrPtr += _accessRegister(nextInstr->arg1);
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
					_accessRegister(nextInstr->arg1) = ~_accessRegister(nextInstr->arg1);
					break;
				case Mnemonic::Time_R:
					_accessRegister(nextInstr->arg1) = static_cast<uint32_t>(getTime(startExecTime));
					break;
				case Mnemonic::ICount_R:
					_accessRegister(nextInstr->arg1) = static_cast<uint32_t>(instrCount);
					break;
				case Mnemonic::Raise_R:
					if (!_testPrivilege(interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired))
						return false;
					_runInterruptCode(_accessRegister(nextInstr->arg1));
					break;
				case Mnemonic::Raise_A:
					if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired))
						return false;
					_runInterruptCode(program[nextInstr->arg1]);
					break;
				case Mnemonic::Raise_I:
					if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilegeRequired))
						return false;
					_runInterruptCode(program[registers[nextInstr->arg1]]);
					break;
				case Mnemonic::Raise_V:
					if (!_testPrivilege(interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired))
						return false;
					_runInterruptCode(nextInstr->arg1);
					break;
				case Mnemonic::DisableInt_R:
					_setInterruptEnabled(_accessRegister(nextInstr->arg1), false);
					break;
				case Mnemonic::DisableInt_V:
					_setInterruptEnabled(nextInstr->arg1, false);
					break;
				case Mnemonic::EnableInt_R:
					_setInterruptEnabled(_accessRegister(nextInstr->arg1), true);
					break;
				case Mnemonic::EnableInt_V:
					_setInterruptEnabled(nextInstr->arg1, true);
					break;
				case Mnemonic::ICountInt_R:
					nextInstrCountInterrupt = _accessRegister(nextInstr->arg1);
					break;
				case Mnemonic::ICountInt_V:
					nextInstrCountInterrupt = nextInstr->arg1;
					break;
				case Mnemonic::RICountInt_R:
					nextInstrCountInterrupt = static_cast<uint32_t>(instrCount) + _accessRegister(nextInstr->arg1);
					break;
				case Mnemonic::RICountInt_V:
					nextInstrCountInterrupt = static_cast<uint32_t>(instrCount) + nextInstr->arg1;
					break;
				case Mnemonic::Mov_R_R:
					_accessRegister(nextInstr->arg1) = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::Mov_R_A:
					_accessRegister(nextInstr->arg1) = program[nextInstr->arg2];
					break;
				case Mnemonic::Mov_R_I:
					_accessRegister(nextInstr->arg1) = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Mov_R_V:
				case Mnemonic::Laddr_R_A:
				case Mnemonic::Laddr_R_I:
					_accessRegister(nextInstr->arg1) = nextInstr->arg2;
					break;
				case Mnemonic::Mov_A_R:
					program[nextInstr->arg1] = _accessRegister(nextInstr->arg2);
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
					program[registers[nextInstr->arg1]] = _accessRegister(nextInstr->arg2);
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
						_accessRegister(nextInstr->arg1) = _accessRegister(nextInstr->arg2);
					}
					break;
				case Mnemonic::Move_R_A:
				case Mnemonic::Movz_R_A:
					if (controlByte & TestEqual) {
						_accessRegister(nextInstr->arg1) = program[nextInstr->arg2];
					}
					break;
				case Mnemonic::Move_R_I:
				case Mnemonic::Movz_R_I:
					if (controlByte & TestEqual) {
						_accessRegister(nextInstr->arg1) = program[registers[nextInstr->arg2]];
					}
					break;
				case Mnemonic::Move_R_V:
				case Mnemonic::Movz_R_V:
					if (controlByte & TestEqual) {
						_accessRegister(nextInstr->arg1) = nextInstr->arg2;
					}
					break;
				case Mnemonic::Move_A_R:
				case Mnemonic::Movz_A_R:
					if (controlByte & TestEqual) {
						program[nextInstr->arg1] = _accessRegister(nextInstr->arg2);
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
						program[registers[nextInstr->arg1]] = _accessRegister(nextInstr->arg2);
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
						_accessRegister(nextInstr->arg1) = _accessRegister(nextInstr->arg2);
					}
					break;
				case Mnemonic::Movne_R_A:
				case Mnemonic::Movnz_R_A:
					if (controlByte & TestUnequal) {
						_accessRegister(nextInstr->arg1) = program[nextInstr->arg2];
					}
					break;
				case Mnemonic::Movne_R_I:
				case Mnemonic::Movnz_R_I:
					if (controlByte & TestUnequal) {
						_accessRegister(nextInstr->arg1) = program[registers[nextInstr->arg2]];
					}
					break;
				case Mnemonic::Movne_R_V:
				case Mnemonic::Movnz_R_V:
					if (controlByte & TestUnequal) {
						_accessRegister(nextInstr->arg1) = nextInstr->arg2;
					}
					break;
				case Mnemonic::Movne_A_R:
				case Mnemonic::Movnz_A_R:
					if (controlByte & TestUnequal) {
						program[nextInstr->arg1] = _accessRegister(nextInstr->arg2);
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
						program[registers[nextInstr->arg1]] = _accessRegister(nextInstr->arg2);
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
					_accessRegister(nextInstr->arg1) += _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::Add_R_I:
					_accessRegister(nextInstr->arg1) += program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Add_R_A:
					_accessRegister(nextInstr->arg1) += program[nextInstr->arg2];
					break;
				case Mnemonic::Add_R_V:
					_accessRegister(nextInstr->arg1) += nextInstr->arg2;
					break;
				case Mnemonic::Add_I_R:
					program[registers[nextInstr->arg1]] += _accessRegister(nextInstr->arg2);
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
					program[nextInstr->arg1] += _accessRegister(nextInstr->arg2);
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
					_accessRegister(nextInstr->arg1) -= _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::Sub_R_A:
					_accessRegister(nextInstr->arg1) -= program[nextInstr->arg2];
					break;
				case Mnemonic::Sub_R_I:
					_accessRegister(nextInstr->arg1) -= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Sub_R_V:
					_accessRegister(nextInstr->arg1) -= nextInstr->arg2;
					break;
				case Mnemonic::Sub_A_R:
					program[nextInstr->arg1] -= _accessRegister(nextInstr->arg2);
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
					program[registers[nextInstr->arg1]] -= _accessRegister(nextInstr->arg2);
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
					_accessRegister(nextInstr->arg1) *= _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::Mul_R_A:
					_accessRegister(nextInstr->arg1) *= program[nextInstr->arg2];
					break;
				case Mnemonic::Mul_R_I:
					_accessRegister(nextInstr->arg1) *= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Mul_R_V:
					_accessRegister(nextInstr->arg1) *= nextInstr->arg2;
					break;
				case Mnemonic::Mul_A_R:
					program[nextInstr->arg1] *= _accessRegister(nextInstr->arg2);
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
					program[registers[nextInstr->arg1]] *= _accessRegister(nextInstr->arg2);
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
					_accessRegister(nextInstr->arg1) /= _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::Div_R_A:
					_accessRegister(nextInstr->arg1) /= program[nextInstr->arg2];
					break;
				case Mnemonic::Div_R_I:
					_accessRegister(nextInstr->arg1) /= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Div_R_V:
					_accessRegister(nextInstr->arg1) /= nextInstr->arg2;
					break;
				case Mnemonic::Div_A_R:
					program[nextInstr->arg1] /= _accessRegister(nextInstr->arg2);
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
					program[registers[nextInstr->arg1]] /= _accessRegister(nextInstr->arg2);
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
					_accessRegister(nextInstr->arg1) %= _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::Mod_R_A:
					_accessRegister(nextInstr->arg1) %= program[nextInstr->arg2];
					break;
				case Mnemonic::Mod_R_I:
					_accessRegister(nextInstr->arg1) %= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Mod_R_V:
					_accessRegister(nextInstr->arg1) %= nextInstr->arg2;
					break;
				case Mnemonic::Mod_A_R:
					program[nextInstr->arg1] %= _accessRegister(nextInstr->arg2);
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
					program[registers[nextInstr->arg1]] %= _accessRegister(nextInstr->arg2);
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
					setControl(_accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::Test_R_A:
					setControl(_accessRegister(nextInstr->arg1), program[nextInstr->arg2]);
					break;
				case Mnemonic::Test_R_I:
					setControl(_accessRegister(nextInstr->arg1), program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Test_R_V:
					setControl(_accessRegister(nextInstr->arg1), nextInstr->arg2);
					break;
				case Mnemonic::Test_A_R:
					setControl(program[nextInstr->arg1], _accessRegister(nextInstr->arg2));
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
					setControl(program[registers[nextInstr->arg1]], _accessRegister(nextInstr->arg2));
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
					_accessRegister(nextInstr->arg1) <<= _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::Lsh_R_A:
					_accessRegister(nextInstr->arg1) <<= program[nextInstr->arg2];
					break;
				case Mnemonic::Lsh_R_I:
					_accessRegister(nextInstr->arg1) <<= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Lsh_R_V:
					_accessRegister(nextInstr->arg1) <<= nextInstr->arg2;
					break;
				case Mnemonic::Lsh_A_R:
					program[nextInstr->arg1] <<= _accessRegister(nextInstr->arg2);
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
					program[registers[nextInstr->arg1]] <<= _accessRegister(nextInstr->arg2);
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
					_accessRegister(nextInstr->arg1) = _rotl(_accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::Rlsh_R_A:
					_accessRegister(nextInstr->arg1) = _rotl(_accessRegister(nextInstr->arg1), program[nextInstr->arg2]);
					break;
				case Mnemonic::Rlsh_R_I:
					_accessRegister(nextInstr->arg1) = _rotl(_accessRegister(nextInstr->arg1), program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Rlsh_R_V:
					_accessRegister(nextInstr->arg1) = _rotl(_accessRegister(nextInstr->arg1), nextInstr->arg2);
					break;
				case Mnemonic::Rlsh_A_R:
					program[nextInstr->arg1] = _rotl(program[nextInstr->arg1], _accessRegister(nextInstr->arg2));
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
					program[registers[nextInstr->arg1]] = _rotl(program[registers[nextInstr->arg1]], _accessRegister(nextInstr->arg2));
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
					_accessRegister(nextInstr->arg1) >>= _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::Rsh_R_A:
					_accessRegister(nextInstr->arg1) >>= program[nextInstr->arg2];
					break;
				case Mnemonic::Rsh_R_I:
					_accessRegister(nextInstr->arg1) >>= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Rsh_R_V:
					_accessRegister(nextInstr->arg1) >>= nextInstr->arg2;
					break;
				case Mnemonic::Rsh_A_R:
					program[nextInstr->arg1] >>= _accessRegister(nextInstr->arg2);
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
					program[registers[nextInstr->arg1]] >>= _accessRegister(nextInstr->arg2);
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
					_accessRegister(nextInstr->arg1) = _rotr(_accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::Rrsh_R_A:
					_accessRegister(nextInstr->arg1) = _rotr(_accessRegister(nextInstr->arg1), program[nextInstr->arg2]);
					break;
				case Mnemonic::Rrsh_R_I:
					_accessRegister(nextInstr->arg1) = _rotr(_accessRegister(nextInstr->arg1), program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Rrsh_R_V:
					_accessRegister(nextInstr->arg1) = _rotr(_accessRegister(nextInstr->arg1), nextInstr->arg2);
					break;
				case Mnemonic::Rrsh_A_R:
					program[nextInstr->arg1] = _rotr(program[nextInstr->arg1], _accessRegister(nextInstr->arg2));
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
					program[registers[nextInstr->arg1]] = _rotr(program[registers[nextInstr->arg1]], _accessRegister(nextInstr->arg2));
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
					_accessRegister(nextInstr->arg1) &= _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::And_R_A:
					_accessRegister(nextInstr->arg1) &= program[nextInstr->arg2];
					break;
				case Mnemonic::And_R_I:
					_accessRegister(nextInstr->arg1) &= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::And_R_V:
					_accessRegister(nextInstr->arg1) &= nextInstr->arg2;
					break;
				case Mnemonic::And_A_R:
					program[nextInstr->arg1] &= _accessRegister(nextInstr->arg2);
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
					program[registers[nextInstr->arg1]] &= _accessRegister(nextInstr->arg2);
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
					_accessRegister(nextInstr->arg1) |= _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::Or_R_A:
					_accessRegister(nextInstr->arg1) |= program[nextInstr->arg2];
					break;
				case Mnemonic::Or_R_I:
					_accessRegister(nextInstr->arg1) |= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Or_R_V:
					_accessRegister(nextInstr->arg1) |= nextInstr->arg2;
					break;
				case Mnemonic::Or_A_R:
					program[nextInstr->arg1] |= _accessRegister(nextInstr->arg2);
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
					program[registers[nextInstr->arg1]] |= _accessRegister(nextInstr->arg2);
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
					_accessRegister(nextInstr->arg1) ^= _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::Xor_R_A:
					_accessRegister(nextInstr->arg1) ^= program[nextInstr->arg2];
					break;
				case Mnemonic::Xor_R_I:
					_accessRegister(nextInstr->arg1) ^= program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::Xor_R_V:
					_accessRegister(nextInstr->arg1) ^= nextInstr->arg2;
					break;
				case Mnemonic::Xor_A_R:
					program[nextInstr->arg1] ^= _accessRegister(nextInstr->arg2);
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
					program[registers[nextInstr->arg1]] ^= _accessRegister(nextInstr->arg2);
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
					setControl(_accessRegister(nextInstr->arg1) == _accessRegister(nextInstr->arg2), TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_R_A:
					setControl(_accessRegister(nextInstr->arg1) == program[nextInstr->arg2], TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_R_I:
					setControl(_accessRegister(nextInstr->arg1) == program[registers[nextInstr->arg2]], TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_R_V:
					setControl(_accessRegister(nextInstr->arg1) == nextInstr->arg2, TestEqual | TestBiggerEqual | TestSmallerEqual);
					break;
				case Mnemonic::Eq_A_R:
					setControl(program[nextInstr->arg1] == _accessRegister(nextInstr->arg2), TestEqual | TestBiggerEqual | TestSmallerEqual);
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
					setControl(program[registers[nextInstr->arg1]] == _accessRegister(nextInstr->arg2), TestEqual | TestBiggerEqual | TestSmallerEqual);
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
					setControl(_accessRegister(nextInstr->arg1) != _accessRegister(nextInstr->arg2), TestUnequal);
					break;
				case Mnemonic::Neq_R_A:
					setControl(_accessRegister(nextInstr->arg1) != program[nextInstr->arg2], TestUnequal);
					break;
				case Mnemonic::Neq_R_I:
					setControl(_accessRegister(nextInstr->arg1) != program[registers[nextInstr->arg2]], TestUnequal);
					break;
				case Mnemonic::Neq_R_V:
					setControl(_accessRegister(nextInstr->arg1) != nextInstr->arg2, TestUnequal);
					break;
				case Mnemonic::Neq_A_R:
					setControl(program[nextInstr->arg1] != _accessRegister(nextInstr->arg2), TestUnequal);
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
					setControl(program[registers[nextInstr->arg1]] != _accessRegister(nextInstr->arg2), TestUnequal);
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
					setControl(_accessRegister(nextInstr->arg1) < _accessRegister(nextInstr->arg2), TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_R_A:
					setControl(_accessRegister(nextInstr->arg1) < program[nextInstr->arg2], TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_R_I:
					setControl(_accessRegister(nextInstr->arg1) < program[registers[nextInstr->arg2]], TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_R_V:
					setControl(_accessRegister(nextInstr->arg1) < nextInstr->arg2, TestSmaller | TestUnequal | TestSmallerEqual);
					break;
				case Mnemonic::Lt_A_R:
					setControl(program[nextInstr->arg1] < _accessRegister(nextInstr->arg2), TestSmaller | TestUnequal | TestSmallerEqual);
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
					setControl(program[registers[nextInstr->arg1]] < _accessRegister(nextInstr->arg2), TestSmaller | TestUnequal | TestSmallerEqual);
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
					setControl(_accessRegister(nextInstr->arg1) > _accessRegister(nextInstr->arg2), TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_R_A:
					setControl(_accessRegister(nextInstr->arg1) > program[nextInstr->arg2], TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_R_I:
					setControl(_accessRegister(nextInstr->arg1) > program[registers[nextInstr->arg2]], TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_R_V:
					setControl(_accessRegister(nextInstr->arg1) > nextInstr->arg2, TestBigger | TestUnequal | TestBiggerEqual);
					break;
				case Mnemonic::Bt_A_R:
					setControl(program[nextInstr->arg1] > _accessRegister(nextInstr->arg2), TestBigger | TestUnequal | TestBiggerEqual);
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
					setControl(program[registers[nextInstr->arg1]] > _accessRegister(nextInstr->arg2), TestBigger | TestUnequal | TestBiggerEqual);
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

				default:
					return _perform2(nextInstr);
			}

			return true;
		}

		bool _perform2(Instruction* nextInstr) {
			switch (nextInstr->mnemonic) {
				case Mnemonic::Loadload_R_A:
					_accessRegister(nextInstr->arg1) = program[program[nextInstr->arg2]];
					break;
				case Mnemonic::Loadload_R_I:
					_accessRegister(nextInstr->arg1) = program[program[registers[nextInstr->arg2]]];
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
				case Mnemonic::Vcall_R_R:
					_pushStack(instrPtr);
					instrPtr = program[_accessRegister(nextInstr->arg1) + _accessRegister(nextInstr->arg2)];
					break;
				case Mnemonic::Vcall_R_A:
					_pushStack(instrPtr);
					instrPtr = program[_accessRegister(nextInstr->arg1) + program[nextInstr->arg2]];
					break;
				case Mnemonic::Vcall_R_I:
					_pushStack(instrPtr);
					instrPtr = program[_accessRegister(nextInstr->arg1) + program[registers[nextInstr->arg2]]];
					break;
				case Mnemonic::Vcall_R_V:
					_pushStack(instrPtr);
					instrPtr = program[_accessRegister(nextInstr->arg1) + nextInstr->arg2];
					break;
				case Mnemonic::Vcall_A_R:
					_pushStack(instrPtr);
					instrPtr = program[program[nextInstr->arg1] + _accessRegister(nextInstr->arg2)];
					break;
				case Mnemonic::Vcall_A_A:
					_pushStack(instrPtr);
					instrPtr = program[program[nextInstr->arg1] + program[nextInstr->arg2]];
					break;
				case Mnemonic::Vcall_A_I:
					_pushStack(instrPtr);
					instrPtr = program[program[nextInstr->arg1] + program[registers[nextInstr->arg2]]];
					break;
				case Mnemonic::Vcall_A_V:
					_pushStack(instrPtr);
					instrPtr = program[program[nextInstr->arg1] + nextInstr->arg2];
					break;
				case Mnemonic::Vcall_I_R:
					_pushStack(instrPtr);
					instrPtr = program[program[registers[nextInstr->arg1]] + _accessRegister(nextInstr->arg2)];
					break;
				case Mnemonic::Vcall_I_A:
					_pushStack(instrPtr);
					instrPtr = program[program[registers[nextInstr->arg1]] + program[nextInstr->arg2]];
					break;
				case Mnemonic::Vcall_I_I:
					_pushStack(instrPtr);
					instrPtr = program[program[registers[nextInstr->arg1]] + program[registers[nextInstr->arg2]]];
					break;
				case Mnemonic::Vcall_I_V:
					_pushStack(instrPtr);
					instrPtr = program[program[registers[nextInstr->arg1]] + nextInstr->arg2];
					break;
				case Mnemonic::Vcall_V_R:
					_pushStack(instrPtr);
					instrPtr = program[nextInstr->arg1 + _accessRegister(nextInstr->arg2)];
					break;
				case Mnemonic::Vcall_V_A:
					_pushStack(instrPtr);
					instrPtr = program[nextInstr->arg1 + program[nextInstr->arg2]];
					break;
				case Mnemonic::Vcall_V_I:
					_pushStack(instrPtr);
					instrPtr = program[nextInstr->arg1 + program[registers[nextInstr->arg2]]];
					break;
				case Mnemonic::Vcall_V_V:
					_pushStack(instrPtr);
					instrPtr = program[nextInstr->arg1 + nextInstr->arg2];
					break;
				case Mnemonic::RegInt_R_R:
					_setInterruptHandler(_accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::Regint_R_I:
					_setInterruptHandler(_accessRegister(nextInstr->arg1), program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Regint_R_A:
					_setInterruptHandler(_accessRegister(nextInstr->arg1), nextInstr->arg2);
					break;
				case Mnemonic::RegInt_A_R:
					_setInterruptHandler(program[nextInstr->arg1], _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::Regint_A_I:
					_setInterruptHandler(program[nextInstr->arg1], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Regint_A_A:
					_setInterruptHandler(program[nextInstr->arg1], nextInstr->arg2);
					break;
				case Mnemonic::RegInt_I_R:
					_setInterruptHandler(program[registers[nextInstr->arg1]], _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::Regint_I_I:
					_setInterruptHandler(program[registers[nextInstr->arg1]], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Regint_I_A:
					_setInterruptHandler(program[registers[nextInstr->arg1]], nextInstr->arg2);
					break;
				case Mnemonic::RegInt_V_R:
					_setInterruptHandler(nextInstr->arg1, _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::Regint_V_I:
					_setInterruptHandler(nextInstr->arg1, program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Regint_V_A:
					_setInterruptHandler(nextInstr->arg1, nextInstr->arg2);
					break;
				case Mnemonic::RRegInt_R_R:
					_setInterruptHandler(_accessRegister(nextInstr->arg1), instrPtr + _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::RRegInt_R_I:
					_setInterruptHandler(_accessRegister(nextInstr->arg1), instrPtr + program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::RRegInt_R_A:
					_setInterruptHandler(_accessRegister(nextInstr->arg1), instrPtr + nextInstr->arg2);
					break;
				case Mnemonic::RRegInt_A_R:
					_setInterruptHandler(program[nextInstr->arg1], instrPtr + _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::RRegInt_A_I:
					_setInterruptHandler(program[nextInstr->arg1], instrPtr + program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::RRegInt_A_A:
					_setInterruptHandler(program[nextInstr->arg1], instrPtr + nextInstr->arg2);
					break;
				case Mnemonic::RRegInt_I_R:
					_setInterruptHandler(program[registers[nextInstr->arg1]], instrPtr + _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::RRegInt_I_I:
					_setInterruptHandler(program[registers[nextInstr->arg1]], instrPtr + program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::RRegInt_I_A:
					_setInterruptHandler(program[registers[nextInstr->arg1]], instrPtr + nextInstr->arg2);
					break;
				case Mnemonic::RRegInt_V_R:
					_setInterruptHandler(nextInstr->arg1, instrPtr + _accessRegister(nextInstr->arg2));
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
					_writeTime(getTime(startExecTime), &_accessRegister(nextInstr->arg1));
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
					_writeTime(instrCount, &_accessRegister(nextInstr->arg1));
					break;
				case Mnemonic::ICount64_A:
					_writeTime(instrCount, &program[nextInstr->arg1]);
					break;
				case Mnemonic::ICount64_I:
					_writeTime(instrCount, &program[registers[nextInstr->arg1]]);
					break;
				case Mnemonic::NtvCall_R:
					_doNativeCall(_accessRegister(nextInstr->arg1));
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
					_accessRegister(nextInstr->arg1) = _findNativeByName(reinterpret_cast<char*>(&program[nextInstr->arg2]));
					break;
				case Mnemonic::GetNtvId_R_I:
					_accessRegister(nextInstr->arg1) = _findNativeByName(reinterpret_cast<char*>(&program[registers[nextInstr->arg2]]));
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
					std::swap(_accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::Xchg_R_A:
					std::swap(_accessRegister(nextInstr->arg1), program[nextInstr->arg2]);
					break;
				case Mnemonic::Xchg_R_I:
					std::swap(_accessRegister(nextInstr->arg1), program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Xchg_A_R:
					std::swap(program[nextInstr->arg1], _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::Xchg_A_A:
					std::swap(program[nextInstr->arg1], program[nextInstr->arg2]);
					break;
				case Mnemonic::Xchg_A_I:
					std::swap(program[nextInstr->arg1], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::Xchg_I_R:
					std::swap(program[registers[nextInstr->arg1]], _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::Xchg_I_A:
					std::swap(program[registers[nextInstr->arg1]], program[nextInstr->arg2]);
					break;
				case Mnemonic::Xchg_I_I:
					std::swap(program[registers[nextInstr->arg1]], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::DisableAllInts:
					interruptsRestore = interrupts;
					for (auto& x : interrupts)  x.enabled = false;
					break;
				case Mnemonic::RestoreInts:
					interrupts = interruptsRestore;
					break;
				case Mnemonic::EnableAllInts:
					for (auto& x : interrupts)  x.enabled = true;
					break;
				case Mnemonic::ClrCb:
					controlByte = 0;
					break;
				case Mnemonic::ICountInt64_R:
					_setNextInstrCountInt(_accessRegister(nextInstr->arg1), *(&_accessRegister(nextInstr->arg1) + 1));
					break;
				case Mnemonic::ICountInt64_A:
					_setNextInstrCountInt(program[nextInstr->arg1], *(&program[nextInstr->arg1] + 1));
					break;
				case Mnemonic::ICountInt64_I:
					_setNextInstrCountInt(program[registers[nextInstr->arg1]], *(&program[registers[nextInstr->arg1]] + 1));
					break;
				case Mnemonic::RICountInt64_R:
					_setNextInstrCountInt(nextInstrCountInterrupt, _accessRegister(nextInstr->arg1), *(&_accessRegister(nextInstr->arg1) + 1));
					break;
				case Mnemonic::RICountInt64_A:
					_setNextInstrCountInt(nextInstrCountInterrupt, program[nextInstr->arg1], *(&program[nextInstr->arg1] + 1));
					break;
				case Mnemonic::RICountInt64_I:
					_setNextInstrCountInt(nextInstrCountInterrupt, program[registers[nextInstr->arg1]], *(&program[registers[nextInstr->arg1]] + 1));
					break;
				case Mnemonic::GetPrivlg_R:
					_accessRegister(nextInstr->arg1) = privilegeLevel;
					break;
				case Mnemonic::GetPrivlg_A:
					program[nextInstr->arg1] = privilegeLevel;
					break;
				case Mnemonic::GetPrivlg_I:
					program[registers[nextInstr->arg1]] = privilegeLevel;
					break;
				case Mnemonic::SetInstrPrivlg_R_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					instrPrivileges[_accessRegister(nextInstr->arg1)] = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetInstrPrivlg_R_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					instrPrivileges[_accessRegister(nextInstr->arg1)] = program[nextInstr->arg2];
					break;
				case Mnemonic::SetInstrPrivlg_R_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					instrPrivileges[_accessRegister(nextInstr->arg1)] = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetInstrPrivlg_R_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					instrPrivileges[_accessRegister(nextInstr->arg1)] = nextInstr->arg2;
					break;
				case Mnemonic::SetInstrPrivlg_A_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					instrPrivileges[program[nextInstr->arg1]] = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetInstrPrivlg_A_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					instrPrivileges[program[nextInstr->arg1]] = program[nextInstr->arg2];
					break;
				case Mnemonic::SetInstrPrivlg_A_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					instrPrivileges[program[nextInstr->arg1]] = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetInstrPrivlg_A_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					instrPrivileges[program[nextInstr->arg1]] = nextInstr->arg2;
					break;
				case Mnemonic::SetInstrPrivlg_I_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					instrPrivileges[program[registers[nextInstr->arg1]]] = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetInstrPrivlg_I_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					instrPrivileges[program[registers[nextInstr->arg1]]] = program[nextInstr->arg2];
					break;
				case Mnemonic::SetInstrPrivlg_I_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					instrPrivileges[program[registers[nextInstr->arg1]]] = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetInstrPrivlg_I_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					instrPrivileges[program[registers[nextInstr->arg1]]] = nextInstr->arg2;
					break;
				case Mnemonic::SetInstrPrivlg_V_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					instrPrivileges[nextInstr->arg1] = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetInstrPrivlg_V_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					instrPrivileges[nextInstr->arg1] = program[nextInstr->arg2];
					break;
				case Mnemonic::SetInstrPrivlg_V_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					instrPrivileges[nextInstr->arg1] = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetInstrPrivlg_V_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					instrPrivileges[nextInstr->arg1] = nextInstr->arg2;
					break;
				case Mnemonic::SetPrivlg_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg1))))
						return false;
					privilegeLevel = _accessRegister(nextInstr->arg1);
					break;
				case Mnemonic::SetPrivlg_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg1])))
						return false;
					privilegeLevel = program[nextInstr->arg1];
					break;
				case Mnemonic::SetPrivlg_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg1]])))
						return false;
					privilegeLevel = program[registers[nextInstr->arg1]];
					break;
				case Mnemonic::SetPrivlg_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg1)))
						return false;
					privilegeLevel = nextInstr->arg1;
					break;
				case Mnemonic::GetInstrPrivlg_R_R:
					if (_accessRegister(nextInstr->arg2) >= Mnemonic::TotalCount) {
						error = { ErrorCode::InvalidInstruction, instrPtr };
						running = false;
						return false;
					}
					_accessRegister(nextInstr->arg1) = instrPrivileges[_accessRegister(nextInstr->arg2)];
					break;
				case Mnemonic::GetInstrPrivlg_R_A:
					if (program[nextInstr->arg2] >= Mnemonic::TotalCount) {
						error = { ErrorCode::InvalidInstruction, instrPtr };
						running = false;
						return false;
					}
					_accessRegister(nextInstr->arg1) = instrPrivileges[program[nextInstr->arg2]];
					break;
				case Mnemonic::GetInstrPrivlg_R_I:
					if (program[registers[nextInstr->arg2]] >= Mnemonic::TotalCount) {
						error = { ErrorCode::InvalidInstruction, instrPtr };
						running = false;
						return false;
					}
					_accessRegister(nextInstr->arg1) = instrPrivileges[program[registers[nextInstr->arg2]]];
					break;
				case Mnemonic::GetInstrPrivlg_R_V:
					if (nextInstr->arg2 >= Mnemonic::TotalCount) {
						error = { ErrorCode::InvalidInstruction, instrPtr };
						running = false;
						return false;
					}
					_accessRegister(nextInstr->arg1) = instrPrivileges[nextInstr->arg2];
					break;
				case Mnemonic::GetInstrPrivlg_A_R:
					if (_accessRegister(nextInstr->arg2) >= Mnemonic::TotalCount) {
						error = { ErrorCode::InvalidInstruction, instrPtr };
						running = false;
						return false;
					}
					program[nextInstr->arg1] = instrPrivileges[_accessRegister(nextInstr->arg2)];
					break;
				case Mnemonic::GetInstrPrivlg_A_A:
					if (program[nextInstr->arg2] >= Mnemonic::TotalCount) {
						error = { ErrorCode::InvalidInstruction, instrPtr };
						running = false;
						return false;
					}
					program[nextInstr->arg1] = instrPrivileges[program[nextInstr->arg2]];
					break;
				case Mnemonic::GetInstrPrivlg_A_I:
					if (program[registers[nextInstr->arg2]] >= Mnemonic::TotalCount) {
						error = { ErrorCode::InvalidInstruction, instrPtr };
						running = false;
						return false;
					}
					program[nextInstr->arg1] = instrPrivileges[program[registers[nextInstr->arg2]]];
					break;
				case Mnemonic::GetInstrPrivlg_A_V:
					if (nextInstr->arg2 >= Mnemonic::TotalCount) {
						error = { ErrorCode::InvalidInstruction, instrPtr };
						running = false;
						return false;
					}
					program[nextInstr->arg1] = instrPrivileges[nextInstr->arg2];
					break;
				case Mnemonic::GetInstrPrivlg_I_R:
					if (_accessRegister(nextInstr->arg2) >= Mnemonic::TotalCount) {
						error = { ErrorCode::InvalidInstruction, instrPtr };
						running = false;
						return false;
					}
					program[registers[nextInstr->arg1]] = instrPrivileges[_accessRegister(nextInstr->arg2)];
					break;
				case Mnemonic::GetInstrPrivlg_I_A:
					if (program[nextInstr->arg2] >= Mnemonic::TotalCount) {
						error = { ErrorCode::InvalidInstruction, instrPtr };
						running = false;
						return false;
					}
					program[registers[nextInstr->arg1]] = instrPrivileges[program[nextInstr->arg2]];
					break;
				case Mnemonic::GetInstrPrivlg_I_I:
					if (program[registers[nextInstr->arg2]] >= Mnemonic::TotalCount) {
						error = { ErrorCode::InvalidInstruction, instrPtr };
						running = false;
						return false;
					}
					program[registers[nextInstr->arg1]] = instrPrivileges[program[registers[nextInstr->arg2]]];
					break;
				case Mnemonic::GetInstrPrivlg_I_V:
					if (nextInstr->arg2 >= Mnemonic::TotalCount) {
						error = { ErrorCode::InvalidInstruction, instrPtr };
						running = false;
						return false;
					}
					program[registers[nextInstr->arg1]] = instrPrivileges[nextInstr->arg2];
					break;
				case Mnemonic::SetIntPrivlg_R_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetIntPrivlg_R_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = program[nextInstr->arg2];
					break;
				case Mnemonic::SetIntPrivlg_R_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetIntPrivlg_R_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = nextInstr->arg2;
					break;
				case Mnemonic::SetIntPrivlg_A_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetIntPrivlg_A_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = program[nextInstr->arg2];
					break;
				case Mnemonic::SetIntPrivlg_A_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetIntPrivlg_A_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = nextInstr->arg2;
					break;
				case Mnemonic::SetIntPrivlg_I_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					interrupts[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilege = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetIntPrivlg_I_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					interrupts[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilege = program[nextInstr->arg2];
					break;
				case Mnemonic::SetIntPrivlg_I_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					interrupts[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilege = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetIntPrivlg_I_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					interrupts[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilege = nextInstr->arg2;
					break;
				case Mnemonic::SetIntPrivlg_V_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilege = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetIntPrivlg_V_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilege = program[nextInstr->arg2];
					break;
				case Mnemonic::SetIntPrivlg_V_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilege = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetIntPrivlg_V_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilege = nextInstr->arg2;
					break;
				case Mnemonic::GetIntPrivlg_R_R:
					_accessRegister(nextInstr->arg1) = interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].privilege;
					break;
				case Mnemonic::GetIntPrivlg_R_A:
					_accessRegister(nextInstr->arg1) = interrupts[static_cast<uint8_t>(program[nextInstr->arg2])].privilege;
					break;
				case Mnemonic::GetIntPrivlg_R_I:
					_accessRegister(nextInstr->arg1) = interrupts[static_cast<uint8_t>(program[registers[nextInstr->arg2]])].privilege;
					break;
				case Mnemonic::GetIntPrivlg_R_V:
					_accessRegister(nextInstr->arg1) = interrupts[static_cast<uint8_t>(nextInstr->arg2)].privilege;
					break;
				case Mnemonic::GetIntPrivlg_A_R:
					program[nextInstr->arg1] = interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].privilege;
					break;
				case Mnemonic::GetIntPrivlg_A_A:
					program[nextInstr->arg1] = interrupts[static_cast<uint8_t>(program[nextInstr->arg2])].privilege;
					break;
				case Mnemonic::GetIntPrivlg_A_I:
					program[nextInstr->arg1] = interrupts[static_cast<uint8_t>(program[registers[nextInstr->arg2]])].privilege;
					break;
				case Mnemonic::GetIntPrivlg_A_V:
					program[nextInstr->arg1] = interrupts[static_cast<uint8_t>(nextInstr->arg2)].privilege;
					break;
				case Mnemonic::GetIntPrivlg_I_R:
					program[registers[nextInstr->arg1]] = interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].privilege;
					break;
				case Mnemonic::GetIntPrivlg_I_A:
					program[registers[nextInstr->arg1]] = interrupts[static_cast<uint8_t>(program[nextInstr->arg2])].privilege;
					break;
				case Mnemonic::GetIntPrivlg_I_I:
					program[registers[nextInstr->arg1]] = interrupts[static_cast<uint8_t>(program[registers[nextInstr->arg2]])].privilege;
					break;
				case Mnemonic::GetIntPrivlg_I_V:
					program[registers[nextInstr->arg1]] = interrupts[static_cast<uint8_t>(nextInstr->arg2)].privilege;
					break;
				case Mnemonic::SetIntExecPrivlg_R_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetIntExecPrivlg_R_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired = program[nextInstr->arg2];
					break;
				case Mnemonic::SetIntExecPrivlg_R_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetIntExecPrivlg_R_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired = nextInstr->arg2;
					break;
				case Mnemonic::SetIntExecPrivlg_A_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetIntExecPrivlg_A_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired = program[nextInstr->arg2];
					break;
				case Mnemonic::SetIntExecPrivlg_A_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetIntExecPrivlg_A_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired = nextInstr->arg2;
					break;
				case Mnemonic::SetIntExecPrivlg_I_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					interrupts[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilegeRequired = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetIntExecPrivlg_I_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					interrupts[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilegeRequired = program[nextInstr->arg2];
					break;
				case Mnemonic::SetIntExecPrivlg_I_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					interrupts[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilegeRequired = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetIntExecPrivlg_I_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					interrupts[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilegeRequired = nextInstr->arg2;
					break;
				case Mnemonic::SetIntExecPrivlg_V_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetIntExecPrivlg_V_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired = program[nextInstr->arg2];
					break;
				case Mnemonic::SetIntExecPrivlg_V_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetIntExecPrivlg_V_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired = nextInstr->arg2;
					break;
				case Mnemonic::SetExtPrivlg_R_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetExtPrivlg_R_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = program[nextInstr->arg2];
					break;
				case Mnemonic::SetExtPrivlg_R_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetExtPrivlg_R_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = nextInstr->arg2;
					break;
				case Mnemonic::SetExtPrivlg_A_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					extensionData[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetExtPrivlg_A_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					extensionData[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = program[nextInstr->arg2];
					break;
				case Mnemonic::SetExtPrivlg_A_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					extensionData[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetExtPrivlg_A_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					extensionData[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = nextInstr->arg2;
					break;
				case Mnemonic::SetExtPrivlg_I_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					extensionData[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilege = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetExtPrivlg_I_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					extensionData[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilege = program[nextInstr->arg2];
					break;
				case Mnemonic::SetExtPrivlg_I_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					extensionData[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilege = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetExtPrivlg_I_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					extensionData[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].privilege = nextInstr->arg2;
					break;
				case Mnemonic::SetExtPrivlg_V_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					extensionData[static_cast<uint8_t>(nextInstr->arg1)].privilege = _accessRegister(nextInstr->arg2);
					break;
				case Mnemonic::SetExtPrivlg_V_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					extensionData[static_cast<uint8_t>(nextInstr->arg1)].privilege = program[nextInstr->arg2];
					break;
				case Mnemonic::SetExtPrivlg_V_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					extensionData[static_cast<uint8_t>(nextInstr->arg1)].privilege = program[registers[nextInstr->arg2]];
					break;
				case Mnemonic::SetExtPrivlg_V_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					extensionData[static_cast<uint8_t>(nextInstr->arg1)].privilege = nextInstr->arg2;
					break;
				case Mnemonic::GetExtPrivlg_R_R:
					_accessRegister(nextInstr->arg1) = extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].privilege;
					break;
				case Mnemonic::GetExtPrivlg_R_A:
					_accessRegister(nextInstr->arg1) = extensionData[static_cast<uint8_t>(program[nextInstr->arg2])].privilege;
					break;
				case Mnemonic::GetExtPrivlg_R_I:
					_accessRegister(nextInstr->arg1) = extensionData[static_cast<uint8_t>(program[registers[nextInstr->arg2]])].privilege;
					break;
				case Mnemonic::GetExtPrivlg_R_V:
					_accessRegister(nextInstr->arg1) = extensionData[static_cast<uint8_t>(nextInstr->arg2)].privilege;
					break;
				case Mnemonic::GetExtPrivlg_A_R:
					program[nextInstr->arg1] = extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].privilege;
					break;
				case Mnemonic::GetExtPrivlg_A_A:
					program[nextInstr->arg1] = extensionData[static_cast<uint8_t>(program[nextInstr->arg2])].privilege;
					break;
				case Mnemonic::GetExtPrivlg_A_I:
					program[nextInstr->arg1] = extensionData[static_cast<uint8_t>(program[registers[nextInstr->arg2]])].privilege;
					break;
				case Mnemonic::GetExtPrivlg_A_V:
					program[nextInstr->arg1] = extensionData[static_cast<uint8_t>(nextInstr->arg2)].privilege;
					break;
				case Mnemonic::GetExtPrivlg_I_R:
					program[registers[nextInstr->arg1]] = extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].privilege;
					break;
				case Mnemonic::GetExtPrivlg_I_A:
					program[registers[nextInstr->arg1]] = extensionData[static_cast<uint8_t>(program[nextInstr->arg2])].privilege;
					break;
				case Mnemonic::GetExtPrivlg_I_I:
					program[registers[nextInstr->arg1]] = extensionData[static_cast<uint8_t>(program[registers[nextInstr->arg2]])].privilege;
					break;
				case Mnemonic::GetExtPrivlg_I_V:
					program[registers[nextInstr->arg1]] = extensionData[static_cast<uint8_t>(nextInstr->arg2)].privilege;
					break;
				case Mnemonic::PCall_R_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					++callDepth;
					return _executeFunc(_accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::PCall_R_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					++callDepth;
					return _executeFunc(_accessRegister(nextInstr->arg1), program[nextInstr->arg2]);
					break;
				case Mnemonic::PCall_R_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					++callDepth;
					return _executeFunc(_accessRegister(nextInstr->arg1), program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::PCall_R_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					++callDepth;
					return _executeFunc(_accessRegister(nextInstr->arg1), nextInstr->arg2);
					break;
				case Mnemonic::PCall_A_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					++callDepth;
					return _executeFunc(nextInstr->arg1, _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::PCall_A_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					++callDepth;
					return _executeFunc(nextInstr->arg1, program[nextInstr->arg2]);
					break;
				case Mnemonic::PCall_A_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					++callDepth;
					return _executeFunc(nextInstr->arg1, program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::PCall_A_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					++callDepth;
					return _executeFunc(nextInstr->arg1, nextInstr->arg2);
					break;
				case Mnemonic::PCall_I_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					++callDepth;
					return _executeFunc(program[registers[nextInstr->arg1]], _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::PCall_I_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					++callDepth;
					return _executeFunc(program[registers[nextInstr->arg1]], program[nextInstr->arg2]);
					break;
				case Mnemonic::PCall_I_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					++callDepth;
					return _executeFunc(program[registers[nextInstr->arg1]], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::PCall_I_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					++callDepth;
					return _executeFunc(program[registers[nextInstr->arg1]], nextInstr->arg2);
					break;
				case Mnemonic::RPCall_R_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					++callDepth;
					return _executeFunc(instrPtr + _accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::RPCall_R_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					++callDepth;
					return _executeFunc(instrPtr + _accessRegister(nextInstr->arg1), program[nextInstr->arg2]);
					break;
				case Mnemonic::RPCall_R_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					++callDepth;
					return _executeFunc(instrPtr + _accessRegister(nextInstr->arg1), program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::RPCall_R_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					++callDepth;
					return _executeFunc(instrPtr + _accessRegister(nextInstr->arg1), nextInstr->arg2);
					break;
				case Mnemonic::RPCall_A_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					++callDepth;
					return _executeFunc(instrPtr + nextInstr->arg1, _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::RPCall_A_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					++callDepth;
					return _executeFunc(instrPtr + nextInstr->arg1, program[nextInstr->arg2]);
					break;
				case Mnemonic::RPCall_A_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					++callDepth;
					return _executeFunc(instrPtr + nextInstr->arg1, program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::RPCall_A_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					++callDepth;
					return _executeFunc(instrPtr + nextInstr->arg1, nextInstr->arg2);
					break;
				case Mnemonic::RPCall_I_R:
					if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2))))
						return false;
					++callDepth;
					return _executeFunc(instrPtr + program[registers[nextInstr->arg1]], _accessRegister(nextInstr->arg2));
					break;
				case Mnemonic::RPCall_I_A:
					if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2])))
						return false;
					++callDepth;
					return _executeFunc(instrPtr + program[registers[nextInstr->arg1]], program[nextInstr->arg2]);
					break;
				case Mnemonic::RPCall_I_I:
					if (!_testPrivilege(static_cast<uint8_t>(program[registers[nextInstr->arg2]])))
						return false;
					++callDepth;
					return _executeFunc(instrPtr + program[registers[nextInstr->arg1]], program[registers[nextInstr->arg2]]);
					break;
				case Mnemonic::RPCall_I_V:
					if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2)))
						return false;
					++callDepth;
					return _executeFunc(instrPtr + program[registers[nextInstr->arg1]], nextInstr->arg2);
					break;
				case Mnemonic::PrintC_R:
					std::cout << static_cast<char>(_accessRegister(nextInstr->arg1));
					break;
				case Mnemonic::PrintC_A:
					std::cout << static_cast<char>(program[nextInstr->arg1]);
					break;
				case Mnemonic::PrintC_I:
					std::cout << static_cast<char>(program[registers[nextInstr->arg1]]);
					break;
				case Mnemonic::PrintC_V:
					std::cout << static_cast<char>(nextInstr->arg1);
					break;
				case Mnemonic::EnableExt_R:
					if (!_testPrivilege(255))
						return false;
					extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].enabled = true;
					break;
				case Mnemonic::EnableExt_A:
					if (!_testPrivilege(255))
						return false;
					extensionData[static_cast<uint8_t>(program[nextInstr->arg1])].enabled = true;
					break;
				case Mnemonic::EnableExt_I:
					if (!_testPrivilege(255))
						return false;
					extensionData[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].enabled = true;
					break;
				case Mnemonic::EnableExt_V:
					if (!_testPrivilege(255))
						return false;
					extensionData[static_cast<uint8_t>(nextInstr->arg1)].enabled = true;
					break;
				case Mnemonic::DisableExt_R:
					if (!_testPrivilege(255))
						return false;
					extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].enabled = false;
					break;
				case Mnemonic::DisableExt_A:
					if (!_testPrivilege(255))
						return false;
					extensionData[static_cast<uint8_t>(program[nextInstr->arg1])].enabled = false;
					break;
				case Mnemonic::DisableExt_I:
					if (!_testPrivilege(255))
						return false;
					extensionData[static_cast<uint8_t>(program[registers[nextInstr->arg1]])].enabled = false;
					break;
				case Mnemonic::DisableExt_V:
					if (!_testPrivilege(255))
						return false;
					extensionData[static_cast<uint8_t>(nextInstr->arg1)].enabled = false;
					break;
				case Mnemonic::IsExtEnabled_R_R:
					_accessRegister(nextInstr->arg1) = extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].enabled;
					break;
				case Mnemonic::IsExtEnabled_R_A:
					_accessRegister(nextInstr->arg1) = extensionData[static_cast<uint8_t>(program[nextInstr->arg2])].enabled;
					break;
				case Mnemonic::IsExtEnabled_R_I:
					_accessRegister(nextInstr->arg1) = extensionData[static_cast<uint8_t>(program[registers[nextInstr->arg2]])].enabled;
					break;
				case Mnemonic::IsExtEnabled_R_V:
					_accessRegister(nextInstr->arg1) = extensionData[static_cast<uint8_t>(nextInstr->arg2)].enabled;
					break;
				case Mnemonic::IsExtEnabled_A_R:
					program[nextInstr->arg1] = extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].enabled;
					break;
				case Mnemonic::IsExtEnabled_A_A:
					program[nextInstr->arg1] = extensionData[static_cast<uint8_t>(program[nextInstr->arg2])].enabled;
					break;
				case Mnemonic::IsExtEnabled_A_I:
					program[nextInstr->arg1] = extensionData[static_cast<uint8_t>(program[registers[nextInstr->arg2]])].enabled;
					break;
				case Mnemonic::IsExtEnabled_A_V:
					program[nextInstr->arg1] = extensionData[static_cast<uint8_t>(nextInstr->arg2)].enabled;
					break;
				case Mnemonic::IsExtEnabled_I_R:
					program[registers[nextInstr->arg1]] = extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].enabled;
					break;
				case Mnemonic::IsExtEnabled_I_A:
					program[registers[nextInstr->arg1]] = extensionData[static_cast<uint8_t>(program[nextInstr->arg2])].enabled;
					break;
				case Mnemonic::IsExtEnabled_I_I:
					program[registers[nextInstr->arg1]] = extensionData[static_cast<uint8_t>(program[registers[nextInstr->arg2]])].enabled;
					break;
				case Mnemonic::IsExtEnabled_I_V:
					program[registers[nextInstr->arg1]] = extensionData[static_cast<uint8_t>(nextInstr->arg2)].enabled;
					break;

				default:
					return _performFloats(nextInstr);
			}
			
			return true;
		}

		bool _performFloats(Instruction* nextInstr) {
			if (!_testPrivilege(extensionData[static_cast<uint8_t>(Extensions::FloatOperations)].privilege))
				return false;

			if (!extensionData[static_cast<uint8_t>(Extensions::FloatOperations)].enabled) {
				error = { ErrorCode::DisabledExtensionUse, instrPtr };
				running = false;
				return false;
			}

			switch (nextInstr->mnemonic) {
				case Mnemonic::FpMov_R_R:
					_accessFpRegister(nextInstr->arg1) = _accessFpRegister(nextInstr->arg2);
					break;
				case Mnemonic::FpMov_R_V:
					_accessFpRegister(nextInstr->arg1) = static_cast<float>(nextInstr->arg2);
					break;
				case Mnemonic::FpAdd_R_R:
					_accessFpRegister(nextInstr->arg1) += _accessFpRegister(nextInstr->arg2);
					break;
				case Mnemonic::FpAdd_R_V:
					_accessFpRegister(nextInstr->arg1) += static_cast<float>(nextInstr->arg2);
					break;
				case Mnemonic::FpSub_R_R:
					_accessFpRegister(nextInstr->arg1) -= _accessFpRegister(nextInstr->arg2);
					break;
				case Mnemonic::FpSub_R_V:
					_accessFpRegister(nextInstr->arg1) -= static_cast<float>(nextInstr->arg2);
					break;
				case Mnemonic::FpMul_R_R:
					_accessFpRegister(nextInstr->arg1) *= _accessFpRegister(nextInstr->arg2);
					break;
				case Mnemonic::FpMul_R_V:
					_accessFpRegister(nextInstr->arg1) *= static_cast<float>(nextInstr->arg2);
					break;
				case Mnemonic::FpDiv_R_R:
					_accessFpRegister(nextInstr->arg1) /= _accessFpRegister(nextInstr->arg2);
					break;
				case Mnemonic::FpDiv_R_V:
					_accessFpRegister(nextInstr->arg1) /= static_cast<float>(nextInstr->arg2);
					break;
				case Mnemonic::FpSign_R_R:
				case Mnemonic::FpSign_R_V:
					if (_accessFpRegister(nextInstr->arg1) < 0) {
						controlByte &= ~TestFloatNegative;
						controlByte |= TestFloatPositive;
					}
					else {
						controlByte &= ~TestFloatPositive;
						controlByte |= TestFloatNegative;
					}
					break;
				case Mnemonic::FpRound_R:
					_accessFpRegister(nextInstr->arg1) = std::round(_accessFpRegister(nextInstr->arg1));
					break;
				case Mnemonic::FpMod_R_R:
					_accessFpRegister(nextInstr->arg1) = _accessFpRegister(nextInstr->arg1) / std::floor(_accessFpRegister(nextInstr->arg1) / _accessFpRegister(nextInstr->arg2)) - _accessFpRegister(nextInstr->arg2);
					break;
				case Mnemonic::FpMod_R_V:
					_accessFpRegister(nextInstr->arg1) = _accessFpRegister(nextInstr->arg1) / std::floor(_accessFpRegister(nextInstr->arg1) / static_cast<float>(nextInstr->arg2)) - static_cast<float>(nextInstr->arg2);
					break;
				case Mnemonic::FpTest_R_R:
					setFloatControl(_accessFpRegister(nextInstr->arg1), _accessFpRegister(nextInstr->arg2));
					break;
				case Mnemonic::FpTest_R_V:
					setFloatControl(_accessFpRegister(nextInstr->arg1), static_cast<float>(nextInstr->arg2));
					break;
				case Mnemonic::FpTest_V_R:
					setFloatControl(static_cast<float>(nextInstr->arg1), _accessFpRegister(nextInstr->arg2));
					break;
				case Mnemonic::FpTest_V_V:
					setFloatControl(static_cast<float>(nextInstr->arg1), static_cast<float>(nextInstr->arg2));
					break;
				case Mnemonic::FpNan_R:
					setControl(std::isnan(_accessFpRegister(nextInstr->arg1)), TestFloatNan);
					break;
				case Mnemonic::FpInf_R:
					setControl(std::isinf(_accessFpRegister(nextInstr->arg1)), TestFloatInf);
					break;
				case Mnemonic::FpPi_R:
					_accessFpRegister(nextInstr->arg1) = 3.1415926536f;
					break;
				case Mnemonic::FpE_R:
					_accessFpRegister(nextInstr->arg1) = 2.7182818285f;
					break;
				case Mnemonic::FpLn2_R:
					_accessFpRegister(nextInstr->arg1) = 0.6931471806f;
					break;
				case Mnemonic::FpLn10_R:
					_accessFpRegister(nextInstr->arg1) = 2.302585093f;
					break;
				case Mnemonic::FpLog10_R:
					_accessFpRegister(nextInstr->arg1) = 0.3010299957f;
					break;
				case Mnemonic::FpPrint_R:
					std::cout << _accessFpRegister(nextInstr->arg1);
					break;
				case Mnemonic::FpPrint_V:
					std::cout << static_cast<float>(nextInstr->arg1);
					break;

				default:
					_onInvalidDecode();
					break;
			}

			return true;
		}

		void _printError() {
			std::cout << formatErrorCode() << "\n";
			std::cout << "While running instruction on address 0x" << std::hex << error.instrPtr << ".\n";
		}
	public:
		VM() : innerState(*this) {}

		template <class T>
		bool run(uint32_t memoffset, T& bytecode) {
			program = &bytecode[0];
			registers.fill(0);
			fpregisters.fill(0.);
			error = { ErrorCode::None, 0 };
			stackPtr = 1000;
			running = true;
			instrCount = 0;
			controlByte = 0;
			instrPtr = memoffset;
			handling = InterruptType::NoInterrupt;
			nextInstrCountInterrupt = 0;
			interrupts.fill({});
			instrPrivileges.fill(0);
			extensionData.fill({});

			privilegeLevel = 255;
			callDepth = 0;
			
			auto oldSync = std::ios::sync_with_stdio(false);

			startExecTime = ch::high_resolution_clock::now().time_since_epoch();

			try {
				while (_execute()) {
				}
			}
			catch (ErrorCode&) {
			}
			catch (...) {
				error = { ErrorCode::UnknownError, instrPtr };
				running = false;
			}

			endExecTime = ch::high_resolution_clock::now().time_since_epoch();

			std::ios::sync_with_stdio(oldSync);

			if (!running && error.code != ErrorCode::None) {
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
			return error;
		}

		std::string formatErrorCode() const {
			std::string errorMsg = "Error: ";
			switch (error.code) {
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
				case ErrorCode::StackOverrflow:
					errorMsg += "Ran out of stack storage";
					break;
				case ErrorCode::UnpirivlegedInstrExec:
					errorMsg += "Unprivileged access to instruction";
					break;
				case ErrorCode::UnprivilegedIntRaise:
					errorMsg += "Unprivileged attempt to raise an interrupt";
					break;
				case ErrorCode::InvalidRegisterId:
					errorMsg += "Register outside of range of available registers used";
					break;
				case ErrorCode::DisabledExtensionUse:
					errorMsg += "Attempt to use instruction from disabled extension";
					break;
				case ErrorCode::UnknownError:
					errorMsg += "Unknown error";
					break;
			}
			errorMsg += '.';
			return errorMsg;
		}
	};

	inline vm::State::State(VM& vm) : vm(&vm)
	{
	}

	inline uint32_t vm::State::nextParameter() {
		return vm->program[vm->stackPtr++];
	}

	inline std::vector<uint32_t> vm::State::nextParameters(size_t count) {
		std::vector<uint32_t> v;
		while (count--) {
			v.push_back(nextParameter());
		}
		return v;
	}

	inline void vm::State::returnValue(uint32_t value) {
		if (!vm->stackPtr) {
			vm->error = { ErrorCode::StackOverrflow, vm->instrPtr };
			vm->running = false;
		}
		vm->program[--vm->stackPtr] = value;
	}

	inline uint32_t vm::State::instrPtr() const {
		return vm->instrPtr;
	}
	
	inline Error vm::State::currentError() const {
		return vm->error;
	}

	inline InterruptType vm::State::currentInterrupt() const {
		return vm->handling;
	}

	inline uint32_t vm::State::currentPrivilege() const {
		return vm->privilegeLevel;
	}

	inline bool vm::State::isIntEnabled(uint8_t intCode) const {
		return vm->interrupts[intCode].enabled;
	}

	inline uint32_t vm::State::instrPrivilegeRequired(Mnemonic m) const {
		return vm->instrPrivileges[static_cast<uint32_t>(m)];
	}
	
	inline uint32_t vm::State::interruptPrivilege(uint8_t intCode) const {
		return vm->interrupts[intCode].privilege;
	}

	inline void vm::State::addArgument(uint32_t arg) {
		vm->_pushStack(arg);
	}

	inline void vm::State::runFunction(uint32_t address) {
		vm->_executeFunc(address);
	}

	inline void vm::State::runFunction(uint32_t address, uint8_t privilege) {
		vm->_executeFunc(address, privilege);
	}
}

#endif	//BUFFED_VM_HEADER_H_