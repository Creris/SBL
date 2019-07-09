#pragma once

#ifndef INTERPRETER_VM_HEADER_H_
#define INTERPRETER_VM_HEADER_H_

#include <filesystem>
#include <fstream>
#include <vector>
#include <array>
#include <cstdint>
#include <string>
#include <chrono>
#include <iostream>
#include <memory>
#include <iomanip>

#include "../common/Instruction.hpp"
#include "../common/FixedVector.hpp"

#include "CompiledHeader.hpp"
#include "Memory.hpp"

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

	//So we dont need to sprinkle 700 lines of code with cmn::Mnemonic
	using namespace cmn;

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
		StackOverflow,
		StackUnderflow,
		UnpirivlegedInstrExec,
		UnprivilegedIntRaise,
		UnprivilegedIntManip,

		InvalidRegisterId,
		InvalidSegmentId,

		InvalidPrivilegeId,
		InvalidExtensionId,

		DisabledExtensionUse,

		UnallowedSegmentRead,
		UnallowedSegmentWrite,
		UnallowedSegmentExec,

		OutOfMemoryAccess,

		InvalidDynamicId,
		InvalidDynamicOffset,
		InvalidDynamicSize,

		InvalidFileLoad,

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

		uint32_t popStack();
		std::vector<uint32_t> popStack(size_t count);
		void pushToStack(uint32_t value);

		uint32_t instrPtr() const;
		Error currentError() const;
		InterruptType currentInterrupt() const;
		bool isIntEnabled(uint8_t intCode) const;

		uint32_t currentPrivilege() const;
		uint32_t instrPrivilegeRequired(Mnemonic m) const;
		uint32_t interruptPrivilege(uint8_t intCode) const;
		uint32_t interruptPrivilegeRequired(uint8_t intCode) const;

		void addArgument(uint32_t arg);
		bool runFunction(uint32_t address);
		bool runFunction(uint32_t address, uint8_t privilege);

		bool raiseInterrupt(uint8_t code);
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

	struct MemoryMap {
		Memory<> memory;
		size_t globalsBase = 0;
		size_t programBase = 0;
		size_t dynamicBase = 0;
		size_t stackBase = 0;
		size_t stackSize = 0;

		MemoryMap() : memory() {}
		explicit MemoryMap(size_t segmentSize) : memory() {}

		bool initialize(uint32_t segmentSize, uint32_t globalSegments, uint32_t programSegments,
						uint32_t dynamicSegments, uint32_t stackSegments, const uint32_t* programImage) {
			memory.clear(segmentSize);
			globalsBase = 0;
			programBase = globalSegments * memory.getSegmentSize();
			dynamicBase = programBase + (programSegments * memory.getSegmentSize());
			stackBase = dynamicBase + (dynamicSegments * memory.getSegmentSize());
			stackSize = stackSegments * memory.getSegmentSize();

			if (!memory.addSegments(globalSegments, SegmentAccessType::Readable | SegmentAccessType::Writable, programImage))
				return false;
			if (!memory.addSegments(programSegments, SegmentAccessType::Executable, programImage + programBase))
				return false;
			if (!memory.addSegments(dynamicSegments, SegmentAccessType::Readable | SegmentAccessType::Writable))
				return false;
			if (!memory.addSegments(stackSegments, SegmentAccessType::Readable | SegmentAccessType::Writable))
				return false;

			return true;
		}
	};

	struct DynamicMemoryHandler {
		std::vector<std::vector<uint32_t>> storage;
		uint32_t lastFreedIdx = -1;

		uint32_t allocateNew(sbl::vm::VM* vm, uint32_t size);
		void deallocate(sbl::vm::VM* vm, uint32_t idx);
		uint32_t* getDynamic(sbl::vm::VM* vm, uint32_t addr);
		uint32_t& getDynamic(sbl::vm::VM* vm, uint32_t addr, uint32_t offset);

		uint32_t getBucketSize(sbl::vm::VM* vm, uint32_t addr);
		void clear();
	};

	class VM {
	public:
		using NativeFunc = void(*)(vm::State&);
	private:
		//Friend State so it can access private parts
		//of the VM without the whole VM being exposed
		//to the user in native functions
		friend vm::State;
		friend vm::DynamicMemoryHandler;

		//Helper functions to disambiguate argument types
		//depending on the instruction argument type
		struct Value { uint32_t value; };
		struct FpValue { float value; };
		struct Register { uint32_t regId; };
		struct FpRegister { uint32_t regId; };
		struct Address { uint32_t addr; };
		struct Indirect { uint32_t regId; };

		enum ControlFlags {
			TestSmaller = 1 << 0,
			TestBigger = 1 << 1,
			TestEqual = 1 << 2,
			TestBiggerEqual = 1 << 3,
			TestSmallerEqual = 1 << 4,
			TestUnequal = 1 << 5,
			TestFloatPositive = 1 << 6,
			TestFloatNegative = 1 << 7,
			TestFloatZero = 1 << 8,
			TestFloatNan = 1 << 9,
			TestFloatInf = 1 << 10,
		};

		uint64_t instrCount = 0;
		uint64_t nextInstrCountInterrupt = 0;
		uint32_t controlByte = 0;
		std::array<uint32_t, 64> registers;
		std::array<float, 16> fpregisters;

		MemoryMap memory;
		DynamicMemoryHandler dynamicHandler;
		uint32_t callDepth;

		bool running = true;
		uint8_t privilegeLevel;
		uint8_t intPrivSet;

		uint32_t& stackPtr = registers[63];
		uint32_t& instrPtr = registers[62];
		uint32_t& basePtr = registers[61];
		uint32_t& loopPtr = registers[60];

		Instruction* lastExecuted = nullptr;

		enum Extensions : uint8_t {
			BasicOperations,
			ArithmeticOperations,
			LogicalOperations,
			AllocationOperations,
			InterruptOperations,
			PrivilegeOperations,
			FloatOperations,

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

		std::vector<std::pair<NativeFunc, std::string>> natives;
		bool namesSorted = false;

		State innerState;
		bool oldSync;
		uint32_t lastExecSegment;
		uint32_t dynamicOffset;

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
				auto cmp = natives[middle].second.compare(val);
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

		__forceinline bool _inMemory(size_t address) {
			return memory.memory.segmentIdx(address) != -1;
		}

		__forceinline bool _onInvalidDecode() {
			error = { ErrorCode::InvalidInstruction, instrPtr };
			running = false;
			return false;
		}

		__forceinline auto _pushStack(uint32_t value) {
			_tryWrite(Address{ --stackPtr }, value);
		};

		__forceinline auto _popStack() {
			auto ret = _tryRead(Address{ stackPtr }, ErrorCode::StackUnderflow);
			++stackPtr;
			return ret;
		};

		__forceinline auto _popStackRef(uint32_t& ref) {
			ref = _popStack();
		};

		__forceinline auto _write(uint32_t* from, uint32_t* to) {
			if (!_inMemory(from - memory.memory.baseAddress())
				|| !_inMemory(to - memory.memory.baseAddress())) {
				error = { ErrorCode::OutOfMemoryAccess, instrPtr };
				running = false;
				throw ErrorCode::OutOfMemoryAccess;
			}

			while (*from)
				*(to++) = *(from++);
		}

		__forceinline auto _writeN(uint32_t* from, size_t count, uint32_t* to) {
			if (!_inMemory(from - memory.memory.baseAddress())
				|| !_inMemory(to - memory.memory.baseAddress())) {
				error = { ErrorCode::OutOfMemoryAccess, instrPtr };
				running = false;
				throw ErrorCode::OutOfMemoryAccess;
			}

			while (count--)
				*(to++) = *(from++);
		}

		__forceinline void _writeTime(uint64_t time, uint32_t& lower, uint32_t& higher) {
			lower = time & 0xFFFFFFFF;
			higher = (time >> 32) & 0xFFFFFFFF;
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
			if (code >= natives.size()) {
				error = { ErrorCode::InvalidNativeId, instrPtr };
				running = false;
				return;
			}

			natives[code].first(innerState);
		}

		__forceinline void _doNativeCall(uint32_t code, uint8_t withPrivilege, Instruction* instr) {
			if (code >= natives.size()) {
				error = { ErrorCode::InvalidNativeId, instrPtr };
				running = false;
				return;
			}
			else if (!_testPrivilege(withPrivilege, instr)) {
				return;
			}

			std::swap(privilegeLevel, withPrivilege);
			natives[code].first(innerState);
			std::swap(privilegeLevel, withPrivilege);
		}

		__forceinline bool _validateInterruptCode(uint32_t code) {
			if (code >= 255) {
				error = { ErrorCode::InvalidInterruptId, instrPtr };
				running = false;
				return false;
			}
			return true;
		}

		__forceinline void _setSegmentAccess(uint32_t segmentId, uint32_t into) {
			if (segmentId >= memory.memory.getSegmentCount()) {
				error = { ErrorCode::InvalidSegmentId, instrPtr };
				running = false;
				return;
			}
			memory.memory.setSegmentAccess(segmentId, static_cast<SegmentAccessType>(into));
		}

		__forceinline void _getSegmentAccess(uint32_t& into, uint32_t segmentId) {
			if (segmentId >= memory.memory.getSegmentCount()) {
				error = { ErrorCode::InvalidSegmentId, instrPtr };
				running = false;
				return;
			}
			into = static_cast<uint32_t>(memory.memory.getSegmentAccess(segmentId));
		}

		__forceinline uint32_t* _checkExecutable(size_t index) {
			uint32_t* memAddr = nullptr;

			//If the next instruction is in the same segment as the last instruction
			//we know for sure it can run
			//Otherwise, (if lastExecSegment is 0 or they are not equal), we
			//need to validate
			auto segId = memory.memory.segmentIdx(index);
			if (!(lastExecSegment != -1 && lastExecSegment == segId)) {
				memAddr = memory.memory.tryAccess(index, SegmentAccessType::Executable);
				if (!memAddr) {
					error = { ErrorCode::UnallowedSegmentExec, instrPtr };
					running = false;
					return memAddr;
				}

				lastExecSegment = segId;
			}
			else {
				memAddr = &memory.memory._getNocheck(index);
			}

			return memAddr;
		}

		//Try read from various types, according to the rules
		//such as no out of bounds index for register, or no
		//read from non-readable segment
		__forceinline uint32_t _tryRead(Value v) {
			return v.value;
		}

		__forceinline float _tryRead(FpValue v) {
			return v.value;
		}

		__forceinline uint32_t& _tryRead(Register reg) {
			return _accessRegister(reg.regId);
		}

		__forceinline float& _tryRead(FpRegister reg) {
			return _accessFpRegister(reg.regId);
		}

		__forceinline uint32_t& _tryRead(Address addr, ErrorCode ec = ErrorCode::UnallowedSegmentRead) {
			auto ptr = memory.memory.tryAccess(addr.addr, SegmentAccessType::Readable);
			if (!ptr) {
				if (_inMemory(addr.addr)) {
					ec = ErrorCode::UnallowedSegmentRead;
				}
				error = { ec, instrPtr };
				running = false;
				throw ec;
			}
			return *ptr;
		}

		__forceinline uint32_t& _tryRead(Indirect indr) {
			return _tryRead(Address{ _tryRead(Register{ indr.regId }) });
		}

		//Try read from dynamic memory range
		__forceinline uint32_t& _tryReadDynamic(uint32_t value, uint32_t offset) {
			return dynamicHandler.getDynamic(this, value, offset);
		}

		__forceinline void _tryWriteDynamic(uint32_t dest, uint32_t offset, uint32_t value) {
			dynamicHandler.getDynamic(this, dest, offset) = value;
		}

		__forceinline uint32_t _getDynSize(uint32_t value) {
			return dynamicHandler.getBucketSize(this, value);
		}

		//ForceRead will read regardless of what the segment has
		//Should only be called if prior test on segment access rights has been performed
		__forceinline uint32_t& _forceRead(Register reg) {
			return _accessRegister(reg.regId);
		}

		__forceinline float& _forceRead(FpRegister reg) {
			return _accessFpRegister(reg.regId);
		}

		__forceinline uint32_t& _forceRead(Address addr) {
			return memory.memory._getNocheck(addr.addr);
		}

		__forceinline uint32_t& _forceRead(Indirect indr) {
			return memory.memory._getNocheck(_tryRead(Register{ indr.regId }));
		}

		//Same as _forceRead, but does the Deref instead
		__forceinline uint32_t& _forceReadDeref(Register reg) {
			return _accessRegister(reg.regId);
		}

		__forceinline float& _forceReadDeref(FpRegister reg) {
			return _accessFpRegister(reg.regId);
		}

		__forceinline uint32_t _forceReadDeref(Address addr) {
			return addr.addr;
		}

		__forceinline uint32_t& _forceReadDeref(Indirect indr) {
			return _tryRead(Register{ indr.regId });
		}


		//Same as _tryRead, but is used for jumps and calls, where
		//the address operand is actually regarded as value
		__forceinline uint32_t _tryReadDeref(Value value) {
			return value.value;
		}

		__forceinline float _tryReadDeref(FpValue v) {
			return v.value;
		}

		__forceinline uint32_t& _tryReadDeref(Register reg) {
			return _accessRegister(reg.regId);
		}

		__forceinline float& _tryReadDeref(FpRegister reg) {
			return _accessFpRegister(reg.regId);
		}

		__forceinline uint32_t _tryReadDeref(Address addr) {
			return _tryReadDeref(Value{ addr.addr });
		}

		__forceinline uint32_t& _tryReadDeref(Indirect indr) {
			return _tryRead(indr);
		}

		//Helpers for Laddr instruction
		__forceinline uint32_t _tryReadAddr(Address addr) {
			return addr.addr;
		}

		__forceinline uint32_t _tryReadAddr(Indirect indr) {
			return _tryRead(Register{ indr.regId });
		}

		//Try to write into a destination according to rules set by its type
		//such as no out of bounds register index or no write to non-writable segment
		__forceinline void _tryWrite(Value dest, uint32_t value) {
			//No-op, writing to temporary value
		}

		__forceinline void _tryWrite(FpValue v) {
		}

		__forceinline void _tryWrite(Register dest, uint32_t value) {
			_accessRegister(dest.regId) = value;
		}

		__forceinline void _tryWrite(FpRegister reg, float value) {
			_accessFpRegister(reg.regId) = value;
		}

		__forceinline void _tryWrite(Address dest, uint32_t value,
										ErrorCode ec = ErrorCode::UnallowedSegmentWrite) {
			auto ptr = memory.memory.tryAccess(dest.addr, SegmentAccessType::Writable);
			if (!ptr) {
				if (_inMemory(dest.addr)) {
					ec = ErrorCode::UnallowedSegmentWrite;
				}
				error = { ec, instrPtr };
				running = false;
				throw ec;
			}
			*ptr = value;
		}

		__forceinline void _tryWrite(Indirect dest, uint32_t value) {
			_tryWrite(Address{ _tryRead(Register{ dest.regId }) }, value);
		}

		//Same as with _forceRead, but for writing
		__forceinline void _forceWrite(Register dest, uint32_t value) {
			_accessRegister(dest.regId) = value;
		}

		__forceinline void _forceWrite(FpRegister dest, float value) {
			_accessFpRegister(dest.regId) = value;
		}

		__forceinline void _forceWrite(Address dest, uint32_t value) {
			memory.memory._getNocheck(dest.addr) = value;
		}

		__forceinline void _forceWrite(Indirect dest, uint32_t value) {
			memory.memory._getNocheck(_tryRead(Register{ dest.regId })) = value;
		}

		//Try to add into a memory according to the rules of its types
		//such as no out of bounds register id or no try to read/write from
		//memory segment that disallows either of these operations
		__forceinline void _tryAdd(Value dest, uint32_t) {
			//No-op, trying to read and write to temporary
		}

		__forceinline void _tryAdd(Register dest, uint32_t value) {
			_accessRegister(dest.regId) += value;
		}

		__forceinline void _tryAdd(Address dest, uint32_t value) {
			auto ptr = memory.memory.tryAccess(dest.addr, SegmentAccessType::Readable | SegmentAccessType::Writable);

			if (!ptr) {
				//One of these two will for sure throw an error before completing
				_tryRead(dest);
				_tryWrite(dest, 0);
			}

			*ptr += value;
		}

		__forceinline void _tryAdd(Indirect dest, uint32_t value) {
			_tryAdd(Address{ _tryRead(Register{ dest.regId }) }, value);
		}

		//Helpers for LoadLoad and Hotpatch
		__forceinline Address _tryAdd(Address left, Address right) {
			return Address{ _tryReadDeref(left) + _tryReadDeref(right) };
		}

		__forceinline Address _tryAdd(Indirect left, Indirect right) {
			return Address{ _tryReadDeref(left) + _tryReadDeref(right) };
		}

		__forceinline Address _tryAdd(Address left, Indirect right) {
			return Address{ _tryReadDeref(left) + _tryReadDeref(right) };
		}

		__forceinline Address _tryAdd(Indirect left, Address right) {
			return Address{ _tryReadDeref(left) + _tryReadDeref(right) };
		}

		//For operations that read AND write to the same operand(most of them)
		__forceinline bool _isReadWrite(Address addr) {
			return memory.memory.tryAccess(addr.addr, SegmentAccessType::Readable | SegmentAccessType::Writable);
		}

		__forceinline bool _isReadWrite(Indirect indr) {
			return _isReadWrite(Address{ _tryRead(Register{ indr.regId }) });
		}

		void _assertReadWrite(Register r) {}
		void _assertReadWrite(FpRegister r) {}
		void _assertReadWrite(Address addr) {
			if (!_isReadWrite(addr)) {
				//One of these two will for sure throw an error before completing
				_tryRead(addr);
				_tryWrite(addr, 0);
			}
		}
		void _assertReadWrite(Indirect indr) {
			if (!_isReadWrite(indr)) {
				//One of these two will for sure throw an error before completing
				_tryRead(indr);
				_tryWrite(indr, 0);
			}
		}

		bool _executeInterrupt() {
			if (!running)	return false;
			++instrCount;

			auto memPtr = _checkExecutable(instrPtr);

			if (!memPtr)
				return false;

			// Literally the entire decode in 2 lines of code:
			auto nextInstr = Instruction::fromAddress(memPtr);
			instrPtr += 3;
			lastExecuted = nextInstr;

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
			if (handling != InterruptType::NoInterrupt) {
				error = { ErrorCode::NestedInterrupt, instrPtr };
				running = false;
				return 0;
			}

			if (!_validateInterruptCode(code))	return 0;
			else if (!interrupts[code].enabled)	return 1;

			auto addr = interrupts[code].addr;
			if (!addr)	return 0;

			handling = static_cast<InterruptType>(code);

			//Store old instruction pointer
			auto oldInstrPtr = instrPtr;
			//Set to the handler
			instrPtr = addr;

			//store old privilege
			intPrivSet = privilegeLevel;
			//set privilege level to the one designated for the interrupt
			privilegeLevel = interrupts[code].privilege;

			while (_executeInterrupt()) {
			}

			bool wasEnd = lastExecuted && lastExecuted->mnemonic == Mnemonic::End;

			//Restore privilege
			privilegeLevel = intPrivSet;

			if (wasEnd)
				return 2;

			if (!running)	return 0;

			//Restore instrPtr, only if we handled it
			instrPtr = oldInstrPtr;

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

		__forceinline bool _testPrivilege(uint8_t totest, Instruction* instr,
							ErrorCode ec = ErrorCode::UnpirivlegedInstrExec) {
			if (totest > privilegeLevel) {
				auto oldR59 = registers[59];
				auto oldR58 = registers[58];
				auto oldR57 = registers[57];
				auto oldR56 = registers[56];
				registers[59] = totest;
				registers[56] = static_cast<uint32_t>(instr->mnemonic);
				registers[57] = instr->arg1;
				registers[58] = instr->arg2;

				bool b = _runInterruptCode(static_cast<uint32_t>(InterruptType::InsufficientPrivilege));

				registers[56] = oldR56;
				registers[57] = oldR57;
				registers[58] = oldR58;
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

			while (nestedDepth != callDepth && _execute()) {
				/*
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

				auto memPtr = _checkExecutable(instrPtr);
				if (!memPtr)
					return false;

				// Literally the entire decode:
				auto nextInstr = Instruction::fromAddress(memPtr);
				instrPtr += 3;
				lastExecuted = nextInstr;

				bool b = _perform(nextInstr);
				if (!b) {
					instrPtr = oldInstr;
					return false;
				}*/
			}

			instrPtr = oldInstr;
			return running;
		}

		__forceinline bool _executeFunc(uint32_t addr, uint8_t withPrivilege, Instruction* executor) {
			if (!_testPrivilege(withPrivilege, executor))
				return false;
			++callDepth;
			auto oldPriv = privilegeLevel;
			privilegeLevel = withPrivilege;
			bool b = _executeFunc(addr);
			privilegeLevel = oldPriv;
			--callDepth;
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

			auto memPtr = _checkExecutable(instrPtr);
			if (!memPtr)
				return false;

			// Literally the entire decode:
			auto nextInstr = Instruction::fromAddress(memPtr);
			instrPtr += 3;
			lastExecuted = nextInstr;

			return _perform(nextInstr);
		}

		bool _perform(Instruction* nextInstr) {
			auto chunkId = static_cast<uint32_t>(nextInstr->mnemonic) / 128;

			if (chunkId >= runners.size()) {
				_onInvalidDecode();
				return false;
			}

			if (!_testPrivilege(instrPrivileges[static_cast<uint32_t>(nextInstr->mnemonic)], nextInstr)) {
				return false;
			}

			/*
				Categories are rounded to 128 sized chunks
				Therefore we go retreive the id of the chunk
			*/
			return (this->*runners[chunkId])(nextInstr);
		}

		void _printError() {
			std::cout << formatErrorCode() << "\n";
			std::cout << "While running instruction on address 0x" << std::hex << error.instrPtr << ".\n";
		}

		void _initRun() {
			error = { ErrorCode::None, 0 };
			stackPtr = (uint32_t)memory.stackBase + (uint32_t)memory.stackSize;
			running = true;
			instrCount = 0;
			controlByte = 0;
			handling = InterruptType::NoInterrupt;
			nextInstrCountInterrupt = 0;
			interrupts.fill({});
			instrPrivileges.fill(0);
			for (size_t i = 240; i < interrupts.size(); ++i)
				interrupts[i].privilegeRequired = 255;

			extensionData.fill({});

			privilegeLevel = 255;
			intPrivSet = 0;
			callDepth = 0;
			lastExecuted = nullptr;

			oldSync = std::ios::sync_with_stdio(false);
			lastExecSegment = -1;
			dynamicOffset = 0;

			startExecTime = ch::high_resolution_clock::now().time_since_epoch();
		}

		void _finalizeRun() {
			endExecTime = ch::high_resolution_clock::now().time_since_epoch();

			std::ios::sync_with_stdio(oldSync);
		}

		bool _performBasic(Instruction* instr);
		bool _performArithmetic(Instruction* instr);
		bool _performLogical(Instruction* instr);
		bool _performAllocation(Instruction* instr);
		bool _performInterrupt(Instruction* instr);
		bool _performPrivilege(Instruction* instr);
		bool _performFloat(Instruction* instr);

		using InstrRunner = bool(VM::*)(Instruction*);

		static constexpr std::array<InstrRunner, 
							static_cast<uint32_t>(Mnemonic::TotalCount) / 128> runners = {
			&_performBasic,			/* BasicOperations,		 */ //Chunk 1
			&_performBasic,			/* BasicOperations,		 */ //Chunk 2
			&_performBasic,			/* BasicOperations,		 */ //Chunk 3
			&_performArithmetic,	/* ArithmeticOperations, */
			&_performLogical,		/* LogicalOperations,	 */
			&_performAllocation,	/* AllocationOperations, */
			&_performInterrupt,		/* InterruptOperations,	 */
			&_performPrivilege,		/* PrivilegeOperations,  */ //Chunk 1
			&_performPrivilege,		/* PrivilegeOperations,  */ //Chunk 2
			&_performFloat,			/* FloatOperations,		 */
		};

		void _loop() {
			try {	
				while (_execute()) {
				}
			} catch (ErrorCode&) {
			} catch (...) {
				error = { ErrorCode::UnknownError, instrPtr };
				running = false;
			}
		}

		bool _initMemory(const std::vector<uint32_t>& stream) {
			CompiledHeader header;
			header.fromStream(stream);
			if (!header.validate()) {
				error = { ErrorCode::InvalidFileLoad, 0 };
				running = false;
				return false;
			}
			
			auto _sc = [](uint32_t byteCount, uint32_t segmentSize) {
				return (byteCount + segmentSize - 1) / segmentSize;
			};

			//Size of each segment in memory in bytes
			uint32_t SegmentSize = (uint32_t)memory.memory.getSegmentSize();

			dynamicHandler.clear();
			if (!memory.initialize(SegmentSize, _sc(header.staticBlockSize, SegmentSize),
												_sc(header.programSize, SegmentSize),
												_sc(header.heapPtrCount, SegmentSize),
												_sc(header.stackSize, SegmentSize),
									&stream[sizeof(CompiledHeader) / sizeof(uint32_t)]))
				return false;
			instrPtr = header.startAddress + (uint32_t)memory.programBase;
			return true;
		}

	public:
		VM() : innerState(*this) {}

		bool run(std::string filename) {
			namespace fs = std::filesystem;
			
			//Construct a path
			fs::path path{ filename };
			std::error_code fe;

			//Get file size
			auto fsize = fs::file_size(path, fe);

			//If there was an error
			//   or the file size is smaller than the necessary header
			if (fe || fsize < sizeof(CompiledHeader)) {
				error = { ErrorCode::InvalidFileLoad, 0 };
				running = false;
				return false;
			}

			//Open the file
			std::ifstream file{ filename, std::ios_base::binary };
			
			//If we failed to open, bail
			if (!file) {
				error = { ErrorCode::InvalidFileLoad, 0 };
				running = false;
				return false;
			}

			//Read contents to memory
			std::vector<uint32_t> fileconts;
			fileconts.resize(fsize / 4);
			file.read(reinterpret_cast<char*>(&fileconts[0]), fsize);

			return run(fileconts);
		}

		bool run(const std::vector<uint32_t>& stream) {
			registers.fill(0);
			fpregisters.fill(0.);
			if (!_initMemory(stream))	return false;
			_initRun();
			_loop();
			_finalizeRun();
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
			natives.push_back(std::make_pair(native, identifier));
			if (immediateSort)	finalizeNatives();
			namesSorted = !immediateSort;
			return id;
		}

		void removeNativeFunction(const std::string& identifier) {
			removeNativeFunction(_findNativeByName(identifier));
		}

		void removeNativeFunction(size_t index) {
			if (index >= natives.size())
				return;
			natives.erase(natives.begin() + index);
		}

		void finalizeNatives() {
			std::sort(natives.begin(), natives.end(), [](const auto& left, const auto& right) {
				return left.second < right.second;
			});

			namesSorted = true;
		}

		Error getError() const {
			return error;
		}

		std::string formatErrorCode() const {
			static std::string msgs[] = {
				"",
				"Attempting to execute invalid instruction",
				"Ret instruction used inside Interrupt handler instead of IRet",
				"Interrupt raised that is enabled and has no handler",
				"Failed to handle interrupt registered with (R)ICountInt(64)",
				"Attempting to raise interrupt with id outside of valid range [0, 255)",
				"Attempting to raise interrupt inside an interrupt handler",
				"Attempt to execute nonexistent Native function",
				"Stack overflow",
				"Stack underflow",
				"Unprivileged attempt to execute an instruction",
				"Attempt to raise an interrupt that requires higher privilege to raise",
				"Attempt to modify state of an interrupt that requires higher privilege to modify",
				"Register index out of range",
				"Segment index out of range",
				"Privilege index out of range",
				"Extension index out of range",
				"Attempt to execute an instruction from disallowed extension",
				"Trying to read from a segment that is not accessible for reading",
				"Trying to write to a segment that is not accessible for reading",
				"Trying to execute code from a segment that is not accessible for executing",
				"Attempting to access an address out of memory",
				"Attempting to access dynamic memory at invalid index",
				"Offset for dynamic memory access set by DynOffset is out of range of accessed dynamic memory",
				"Cannot create dynamic memory block of specified size",
				"Failed to load the program to memory from file/stream"
				"Unknown error",
			};

			return "Error: " + msgs[static_cast<uint32_t>(error.code)] + '.';
		}
	};

	inline vm::State::State(VM& vm) : vm(&vm)
	{
	}

	inline uint32_t vm::State::popStack() {
		return vm->_popStack();
	}

	inline std::vector<uint32_t> vm::State::popStack(size_t count) {
		std::vector<uint32_t> v;
		while (count--) {
			v.push_back(popStack());
		}
		return v;
	}

	inline void vm::State::pushToStack(uint32_t value) {
		vm->_pushStack(value);
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

	inline uint32_t vm::State::interruptPrivilegeRequired(uint8_t intCode) const {
		return vm->interrupts[intCode].privilegeRequired;
	}

	inline void vm::State::addArgument(uint32_t arg) {
		vm->_pushStack(arg);
	}

	inline bool vm::State::runFunction(uint32_t address) {
		return vm->_executeFunc(address);
	}

	inline bool vm::State::runFunction(uint32_t address, uint8_t privilege) {
		Instruction* _executor = nullptr;
		return vm->_executeFunc(address, privilege, _executor);
	}

	inline bool vm::State::raiseInterrupt(uint8_t code) {
		return vm->_runInterruptCode(code);
	}

	inline uint32_t DynamicMemoryHandler::allocateNew(sbl::vm::VM* vm, uint32_t size) {
		if (size == 0) {
			vm->error = { ErrorCode::InvalidDynamicSize, vm->instrPtr };
			vm->running = false;
			throw ErrorCode::InvalidDynamicSize;
		}

		if (lastFreedIdx >= storage.size()) {
			storage.push_back(std::vector<uint32_t>(size));
			return (uint32_t)storage.size() - 1;
		}
		else {
			storage[lastFreedIdx] = std::move(std::vector<uint32_t>(size));
			auto idx = lastFreedIdx;
			while (lastFreedIdx < storage.size() && storage[lastFreedIdx].size()) {
				++lastFreedIdx;
			}
			if (lastFreedIdx == storage.size())
				lastFreedIdx = -1;
			return idx;
		}
	}

	inline void DynamicMemoryHandler::deallocate(sbl::vm::VM* vm, uint32_t idx) {
		if (idx >= storage.size()) {
			vm->error = { ErrorCode::InvalidDynamicId, vm->instrPtr };
			vm->running = false;
			throw ErrorCode::InvalidDynamicId;
		}
		storage[idx].clear();
		lastFreedIdx = idx;
	}

	inline uint32_t* DynamicMemoryHandler::getDynamic(sbl::vm::VM* vm, uint32_t addr) {
		if (addr >= storage.size() || !storage[addr].size()) {
			vm->error = { ErrorCode::InvalidDynamicId, vm->instrPtr };
			vm->running = false;
			throw ErrorCode::InvalidDynamicId;
		}
		return &storage[addr][0];
	}

	inline uint32_t& DynamicMemoryHandler::getDynamic(sbl::vm::VM* vm, uint32_t addr, uint32_t offset) {
		if (addr >= storage.size() || !storage[addr].size()) {
			vm->error = { ErrorCode::InvalidDynamicId, vm->instrPtr };
			vm->running = false;
			throw ErrorCode::InvalidDynamicId;
		}
		auto& dynMem = storage[addr];
		if (offset >= dynMem.size()) {
			vm->error = { ErrorCode::InvalidDynamicOffset, vm->instrPtr };
			vm->running = false;
			throw ErrorCode::InvalidDynamicOffset;
		}
		return dynMem[offset];
	}

	inline uint32_t DynamicMemoryHandler::getBucketSize(sbl::vm::VM* vm, uint32_t addr) {
		if (addr >= storage.size() || !storage[addr].size()) {
			vm->error = { ErrorCode::InvalidDynamicId, vm->instrPtr };
			vm->running = false;
			throw ErrorCode::InvalidDynamicId;
		}
		return (uint32_t)storage[addr].size();
	}

	inline void DynamicMemoryHandler::clear() {
		storage.clear();
		lastFreedIdx = -1;
	}
}

#endif	//INTERPRETER_VM_HEADER_H_