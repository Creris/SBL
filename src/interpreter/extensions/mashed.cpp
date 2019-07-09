#include "../VM.hpp"
#include "../../common/Instruction.hpp"

namespace sbl::vm{

bool VM::_perform(Instruction* nextInstr) {
	auto mnem_v = static_cast<uint32_t>(nextInstr->mnemonic);
	if (mnem_v >= Mnemonic::TotalCount) {
		_onInvalidDecode();
		return false;
	}

	if (!_testPrivilege(instrPrivileges[mnem_v], static_cast<Mnemonic>(mnem_v))) {
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
			_pushStack(program[_accessRegister(nextInstr->arg1)]);
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
			program[_accessRegister(nextInstr->arg1)] = _popStack();
			break;
		case Mnemonic::Inc_R:
			_accessRegister(nextInstr->arg1)++;
			break;
		case Mnemonic::Inc_A:
			program[nextInstr->arg1]++;
			break;
		case Mnemonic::Inc_I:
			program[_accessRegister(nextInstr->arg1)]++;
			break;
		case Mnemonic::Dec_R:
			_accessRegister(nextInstr->arg1)--;
			break;
		case Mnemonic::Dec_A:
			program[nextInstr->arg1]--;
			break;
		case Mnemonic::Dec_I:
			program[_accessRegister(nextInstr->arg1)]--;
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
			instrPtr = program[_accessRegister(nextInstr->arg1)];
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
			instrPtr += program[_accessRegister(nextInstr->arg1)];
			++callDepth;
			break;
		case Mnemonic::Read_R:
			std::cin >> _accessRegister(nextInstr->arg1);
			break;
		case Mnemonic::Read_A:
			std::cin >> program[nextInstr->arg1];
			break;
		case Mnemonic::Read_I:
			std::cin >> program[_accessRegister(nextInstr->arg1)];
			break;
		case Mnemonic::Readstr_A:
			std::cin >> reinterpret_cast<char*>(&program[nextInstr->arg1]);
			break;
		case Mnemonic::Readstr_I:
			std::cin >> reinterpret_cast<char*>(&program[_accessRegister(nextInstr->arg1)]);
			break;
		case Mnemonic::Print_R:
			std::cout << _accessRegister(nextInstr->arg1);
			break;
		case Mnemonic::Print_A:
			std::cout << program[nextInstr->arg1];
			break;
		case Mnemonic::Print_I:
			std::cout << program[_accessRegister(nextInstr->arg1)];
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
			std::cout << static_cast<int32_t>(program[_accessRegister(nextInstr->arg1)]);
			break;
		case Mnemonic::PrintS_V:
			std::cout << static_cast<int32_t>(nextInstr->arg1);
			break;
		case Mnemonic::Printstr_A:
			std::cout << reinterpret_cast<char*>(&program[nextInstr->arg1]);
			break;
		case Mnemonic::Printstr_I:
			std::cout << reinterpret_cast<char*>(&program[_accessRegister(nextInstr->arg1)]);
			break;
		case Mnemonic::Jmp_R:
			instrPtr = _accessRegister(nextInstr->arg1);
			break;
		case Mnemonic::Jmp_A:
			instrPtr = nextInstr->arg1;
			break;
		case Mnemonic::Jmp_I:
			instrPtr = program[_accessRegister(nextInstr->arg1)];
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
				instrPtr = program[_accessRegister(nextInstr->arg1)];
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
				instrPtr = program[_accessRegister(nextInstr->arg1)];
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
				instrPtr = program[_accessRegister(nextInstr->arg1)];
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
				instrPtr = program[_accessRegister(nextInstr->arg1)];
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
				instrPtr = program[_accessRegister(nextInstr->arg1)];
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
				instrPtr = program[_accessRegister(nextInstr->arg1)];
			}
			break;
		case Mnemonic::RJmp_R:
			instrPtr += _accessRegister(nextInstr->arg1);
			break;
		case Mnemonic::RJmp_A:
			instrPtr += nextInstr->arg1;
			break;
		case Mnemonic::RJmp_I:
			instrPtr += program[_accessRegister(nextInstr->arg1)];
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
				instrPtr += program[_accessRegister(nextInstr->arg1)];
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
				instrPtr += program[_accessRegister(nextInstr->arg1)];
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
				instrPtr += program[_accessRegister(nextInstr->arg1)];
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
				instrPtr += program[_accessRegister(nextInstr->arg1)];
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
				instrPtr += program[_accessRegister(nextInstr->arg1)];
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
				instrPtr += program[_accessRegister(nextInstr->arg1)];
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
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired, Mnemonic::Raise_R, ErrorCode::UnprivilegedIntRaise))
				return false;
			_runInterruptCode(_accessRegister(nextInstr->arg1));
			break;
		case Mnemonic::Raise_A:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired, Mnemonic::Raise_A, ErrorCode::UnprivilegedIntRaise))
				return false;
			_runInterruptCode(program[nextInstr->arg1]);
			break;
		case Mnemonic::Raise_I:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilegeRequired, Mnemonic::Raise_I, ErrorCode::UnprivilegedIntRaise))
				return false;
			_runInterruptCode(program[_accessRegister(nextInstr->arg1)]);
			break;
		case Mnemonic::Raise_V:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired, Mnemonic::Raise_V, ErrorCode::UnprivilegedIntRaise))
				return false;
			_runInterruptCode(nextInstr->arg1);
			break;
		case Mnemonic::DisableInt_R:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired, Mnemonic::DisableInt_R, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptEnabled(_accessRegister(nextInstr->arg1), false);
			break;
		case Mnemonic::DisableInt_V:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired, Mnemonic::DisableInt_V, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptEnabled(nextInstr->arg1, false);
			break;
		case Mnemonic::EnableInt_R:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired, Mnemonic::EnableInt_R, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptEnabled(_accessRegister(nextInstr->arg1), true);
			break;
		case Mnemonic::EnableInt_V:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired, Mnemonic::EnableInt_V, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptEnabled(nextInstr->arg1, true);
			break;
		case Mnemonic::ICountInt_R:
			nextInstrCountInterrupt = _accessRegister(nextInstr->arg1);
			break;
		case Mnemonic::ICountInt_V:
			nextInstrCountInterrupt = nextInstr->arg1;
			break;
		case Mnemonic::ExecInstr_A:
			return _perform(Instruction::fromStream(program, nextInstr->arg1));
			break;
		case Mnemonic::ExecInstr_I:
			return _perform(Instruction::fromStream(program, program[_accessRegister(nextInstr->arg1)]));
			break;
		case Mnemonic::Mov_R_R:
			_accessRegister(nextInstr->arg1) = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Mov_R_A:
			_accessRegister(nextInstr->arg1) = program[nextInstr->arg2];
			break;
		case Mnemonic::Mov_R_I:
			_accessRegister(nextInstr->arg1) = program[_accessRegister(nextInstr->arg2)];
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
			program[nextInstr->arg1] = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Mov_A_V:
		case Mnemonic::Laddr_A_A:
		case Mnemonic::Laddr_A_I:
			program[nextInstr->arg1] = nextInstr->arg2;
			break;
		case Mnemonic::Mov_I_R:
			program[_accessRegister(nextInstr->arg1)] = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Mov_I_A:
			program[_accessRegister(nextInstr->arg1)] = program[nextInstr->arg2];
			break;
		case Mnemonic::Mov_I_I:
			program[_accessRegister(nextInstr->arg1)] = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Mov_I_V:
		case Mnemonic::Laddr_I_A:
		case Mnemonic::Laddr_I_I:
			program[_accessRegister(nextInstr->arg1)] = nextInstr->arg2;
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
				_accessRegister(nextInstr->arg1) = program[_accessRegister(nextInstr->arg2)];
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
				program[nextInstr->arg1] = program[_accessRegister(nextInstr->arg2)];
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
				program[_accessRegister(nextInstr->arg1)] = _accessRegister(nextInstr->arg2);
			}
			break;
		case Mnemonic::Move_I_A:
		case Mnemonic::Movz_I_A:
			if (controlByte & TestEqual) {
				program[_accessRegister(nextInstr->arg1)] = program[nextInstr->arg2];
			}
			break;
		case Mnemonic::Move_I_I:
		case Mnemonic::Movz_I_I:
			if (controlByte & TestEqual) {
				program[_accessRegister(nextInstr->arg1)] = program[_accessRegister(nextInstr->arg2)];
			}
			break;
		case Mnemonic::Move_I_V:
		case Mnemonic::Movz_I_V:
			if (controlByte & TestEqual) {
				program[_accessRegister(nextInstr->arg1)] = nextInstr->arg2;
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
				_accessRegister(nextInstr->arg1) = program[_accessRegister(nextInstr->arg2)];
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
				program[nextInstr->arg1] = program[_accessRegister(nextInstr->arg2)];
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
				program[_accessRegister(nextInstr->arg1)] = _accessRegister(nextInstr->arg2);
			}
			break;
		case Mnemonic::Movne_I_A:
		case Mnemonic::Movnz_I_A:
			if (controlByte & TestUnequal) {
				program[_accessRegister(nextInstr->arg1)] = program[nextInstr->arg2];
			}
			break;
		case Mnemonic::Movne_I_I:
		case Mnemonic::Movnz_I_I:
			if (controlByte & TestUnequal) {
				program[_accessRegister(nextInstr->arg1)] = program[_accessRegister(nextInstr->arg2)];
			}
			break;
		case Mnemonic::Movne_I_V:
		case Mnemonic::Movnz_I_V:
			if (controlByte & TestUnequal) {
				program[_accessRegister(nextInstr->arg1)] = nextInstr->arg2;
			}
			break;
		case Mnemonic::Add_R_R:
			_accessRegister(nextInstr->arg1) += _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Add_R_I:
			_accessRegister(nextInstr->arg1) += program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Add_R_A:
			_accessRegister(nextInstr->arg1) += program[nextInstr->arg2];
			break;
		case Mnemonic::Add_R_V:
			_accessRegister(nextInstr->arg1) += nextInstr->arg2;
			break;
		case Mnemonic::Add_I_R:
			program[_accessRegister(nextInstr->arg1)] += _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Add_I_I:
			program[_accessRegister(nextInstr->arg1)] += program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Add_I_A:
			program[_accessRegister(nextInstr->arg1)] += program[nextInstr->arg2];
			break;
		case Mnemonic::Add_I_V:
			program[_accessRegister(nextInstr->arg1)] += nextInstr->arg2;
			break;
		case Mnemonic::Add_A_R:
			program[nextInstr->arg1] += _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Add_A_I:
			program[nextInstr->arg1] += program[_accessRegister(nextInstr->arg2)];
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
			_accessRegister(nextInstr->arg1) -= program[_accessRegister(nextInstr->arg2)];
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
			program[nextInstr->arg1] -= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Sub_A_V:
			program[nextInstr->arg1] -= nextInstr->arg2;
			break;
		case Mnemonic::Sub_I_R:
			program[_accessRegister(nextInstr->arg1)] -= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Sub_I_A:
			program[_accessRegister(nextInstr->arg1)] -= program[nextInstr->arg2];
			break;
		case Mnemonic::Sub_I_I:
			program[_accessRegister(nextInstr->arg1)] -= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Sub_I_V:
			program[_accessRegister(nextInstr->arg1)] -= nextInstr->arg2;
			break;
		case Mnemonic::Mul_R_R:
			_accessRegister(nextInstr->arg1) *= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Mul_R_A:
			_accessRegister(nextInstr->arg1) *= program[nextInstr->arg2];
			break;
		case Mnemonic::Mul_R_I:
			_accessRegister(nextInstr->arg1) *= program[_accessRegister(nextInstr->arg2)];
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
			program[nextInstr->arg1] *= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Mul_A_V:
			program[nextInstr->arg1] *= nextInstr->arg2;
			break;
		case Mnemonic::Mul_I_R:
			program[_accessRegister(nextInstr->arg1)] *= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Mul_I_A:
			program[_accessRegister(nextInstr->arg1)] *= program[nextInstr->arg2];
			break;
		case Mnemonic::Mul_I_I:
			program[_accessRegister(nextInstr->arg1)] *= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Mul_I_V:
			program[_accessRegister(nextInstr->arg1)] *= nextInstr->arg2;
			break;
		case Mnemonic::Div_R_R:
			_accessRegister(nextInstr->arg1) /= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Div_R_A:
			_accessRegister(nextInstr->arg1) /= program[nextInstr->arg2];
			break;
		case Mnemonic::Div_R_I:
			_accessRegister(nextInstr->arg1) /= program[_accessRegister(nextInstr->arg2)];
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
			program[nextInstr->arg1] /= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Div_A_V:
			program[nextInstr->arg1] /= nextInstr->arg2;
			break;
		case Mnemonic::Div_I_R:
			program[_accessRegister(nextInstr->arg1)] /= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Div_I_A:
			program[_accessRegister(nextInstr->arg1)] /= program[nextInstr->arg2];
			break;
		case Mnemonic::Div_I_I:
			program[_accessRegister(nextInstr->arg1)] /= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Div_I_V:
			program[_accessRegister(nextInstr->arg1)] /= nextInstr->arg2;
			break;
		case Mnemonic::Mod_R_R:
			_accessRegister(nextInstr->arg1) %= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Mod_R_A:
			_accessRegister(nextInstr->arg1) %= program[nextInstr->arg2];
			break;
		case Mnemonic::Mod_R_I:
			_accessRegister(nextInstr->arg1) %= program[_accessRegister(nextInstr->arg2)];
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
			program[nextInstr->arg1] %= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Mod_A_V:
			program[nextInstr->arg1] %= nextInstr->arg2;
			break;
		case Mnemonic::Mod_I_R:
			program[_accessRegister(nextInstr->arg1)] %= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Mod_I_A:
			program[_accessRegister(nextInstr->arg1)] %= program[nextInstr->arg2];
			break;
		case Mnemonic::Mod_I_I:
			program[_accessRegister(nextInstr->arg1)] %= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Mod_I_V:
			program[_accessRegister(nextInstr->arg1)] %= nextInstr->arg2;
			break;
		case Mnemonic::Test_R_R:
			setControl(_accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Test_R_A:
			setControl(_accessRegister(nextInstr->arg1), program[nextInstr->arg2]);
			break;
		case Mnemonic::Test_R_I:
			setControl(_accessRegister(nextInstr->arg1), program[_accessRegister(nextInstr->arg2)]);
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
			setControl(program[nextInstr->arg1], program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::Test_A_V:
			setControl(program[nextInstr->arg1], nextInstr->arg2);
			break;
		case Mnemonic::Test_I_R:
			setControl(program[_accessRegister(nextInstr->arg1)], _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Test_I_A:
			setControl(program[_accessRegister(nextInstr->arg1)], program[nextInstr->arg2]);
			break;
		case Mnemonic::Test_I_I:
			setControl(program[_accessRegister(nextInstr->arg1)], program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::Test_I_V:
			setControl(program[_accessRegister(nextInstr->arg1)], nextInstr->arg2);
			break;
		case Mnemonic::Lsh_R_R:
			_accessRegister(nextInstr->arg1) <<= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Lsh_R_A:
			_accessRegister(nextInstr->arg1) <<= program[nextInstr->arg2];
			break;
		case Mnemonic::Lsh_R_I:
			_accessRegister(nextInstr->arg1) <<= program[_accessRegister(nextInstr->arg2)];
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
			program[nextInstr->arg1] <<= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Lsh_A_V:
			program[nextInstr->arg1] <<= nextInstr->arg2;
			break;
		case Mnemonic::Lsh_I_R:
			program[_accessRegister(nextInstr->arg1)] <<= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Lsh_I_A:
			program[_accessRegister(nextInstr->arg1)] <<= program[nextInstr->arg2];
			break;
		case Mnemonic::Lsh_I_I:
			program[_accessRegister(nextInstr->arg1)] <<= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Lsh_I_V:
			program[_accessRegister(nextInstr->arg1)] <<= nextInstr->arg2;
			break;
		case Mnemonic::Rlsh_R_R:
			_accessRegister(nextInstr->arg1) = _rotl(_accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Rlsh_R_A:
			_accessRegister(nextInstr->arg1) = _rotl(_accessRegister(nextInstr->arg1), program[nextInstr->arg2]);
			break;
		case Mnemonic::Rlsh_R_I:
			_accessRegister(nextInstr->arg1) = _rotl(_accessRegister(nextInstr->arg1), program[_accessRegister(nextInstr->arg2)]);
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
			program[nextInstr->arg1] = _rotl(program[nextInstr->arg1], program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::Rlsh_A_V:
			program[nextInstr->arg1] = _rotl(program[nextInstr->arg1], nextInstr->arg2);
			break;
		case Mnemonic::Rlsh_I_R:
			program[_accessRegister(nextInstr->arg1)] = _rotl(program[_accessRegister(nextInstr->arg1)], _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Rlsh_I_A:
			program[_accessRegister(nextInstr->arg1)] = _rotl(program[_accessRegister(nextInstr->arg1)], program[nextInstr->arg2]);
			break;
		case Mnemonic::Rlsh_I_I:
			program[_accessRegister(nextInstr->arg1)] = _rotl(program[_accessRegister(nextInstr->arg1)], program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::Rlsh_I_V:
			program[_accessRegister(nextInstr->arg1)] = _rotl(program[_accessRegister(nextInstr->arg1)], nextInstr->arg2);
			break;
		case Mnemonic::Rsh_R_R:
			_accessRegister(nextInstr->arg1) >>= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Rsh_R_A:
			_accessRegister(nextInstr->arg1) >>= program[nextInstr->arg2];
			break;
		case Mnemonic::Rsh_R_I:
			_accessRegister(nextInstr->arg1) >>= program[_accessRegister(nextInstr->arg2)];
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
			program[nextInstr->arg1] >>= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Rsh_A_V:
			program[nextInstr->arg1] >>= nextInstr->arg2;
			break;
		case Mnemonic::Rsh_I_R:
			program[_accessRegister(nextInstr->arg1)] >>= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Rsh_I_A:
			program[_accessRegister(nextInstr->arg1)] >>= program[nextInstr->arg2];
			break;
		case Mnemonic::Rsh_I_I:
			program[_accessRegister(nextInstr->arg1)] >>= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Rsh_I_V:
			program[_accessRegister(nextInstr->arg1)] >>= nextInstr->arg2;
			break;
		case Mnemonic::Rrsh_R_R:
			_accessRegister(nextInstr->arg1) = _rotr(_accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Rrsh_R_A:
			_accessRegister(nextInstr->arg1) = _rotr(_accessRegister(nextInstr->arg1), program[nextInstr->arg2]);
			break;
		case Mnemonic::Rrsh_R_I:
			_accessRegister(nextInstr->arg1) = _rotr(_accessRegister(nextInstr->arg1), program[_accessRegister(nextInstr->arg2)]);
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
			program[nextInstr->arg1] = _rotr(program[nextInstr->arg1], program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::Rrsh_A_V:
			program[nextInstr->arg1] = _rotr(program[nextInstr->arg1], nextInstr->arg2);
			break;
		case Mnemonic::Rrsh_I_R:
			program[_accessRegister(nextInstr->arg1)] = _rotr(program[_accessRegister(nextInstr->arg1)], _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Rrsh_I_A:
			program[_accessRegister(nextInstr->arg1)] = _rotr(program[_accessRegister(nextInstr->arg1)], program[nextInstr->arg2]);
			break;
		case Mnemonic::Rrsh_I_I:
			program[_accessRegister(nextInstr->arg1)] = _rotr(program[_accessRegister(nextInstr->arg1)], program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::Rrsh_I_V:
			program[_accessRegister(nextInstr->arg1)] = _rotr(program[_accessRegister(nextInstr->arg1)], nextInstr->arg2);
			break;
		case Mnemonic::And_R_R:
			_accessRegister(nextInstr->arg1) &= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::And_R_A:
			_accessRegister(nextInstr->arg1) &= program[nextInstr->arg2];
			break;
		case Mnemonic::And_R_I:
			_accessRegister(nextInstr->arg1) &= program[_accessRegister(nextInstr->arg2)];
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
			program[nextInstr->arg1] &= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::And_A_V:
			program[nextInstr->arg1] &= nextInstr->arg2;
			break;
		case Mnemonic::And_I_R:
			program[_accessRegister(nextInstr->arg1)] &= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::And_I_A:
			program[_accessRegister(nextInstr->arg1)] &= program[nextInstr->arg2];
			break;
		case Mnemonic::And_I_I:
			program[_accessRegister(nextInstr->arg1)] &= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::And_I_V:
			program[_accessRegister(nextInstr->arg1)] &= nextInstr->arg2;
			break;
		case Mnemonic::Or_R_R:
			_accessRegister(nextInstr->arg1) |= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Or_R_A:
			_accessRegister(nextInstr->arg1) |= program[nextInstr->arg2];
			break;
		case Mnemonic::Or_R_I:
			_accessRegister(nextInstr->arg1) |= program[_accessRegister(nextInstr->arg2)];
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
			program[nextInstr->arg1] |= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Or_A_V:
			program[nextInstr->arg1] |= nextInstr->arg2;
			break;
		case Mnemonic::Or_I_R:
			program[_accessRegister(nextInstr->arg1)] |= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Or_I_A:
			program[_accessRegister(nextInstr->arg1)] |= program[nextInstr->arg2];
			break;
		case Mnemonic::Or_I_I:
			program[_accessRegister(nextInstr->arg1)] |= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Or_I_V:
			program[_accessRegister(nextInstr->arg1)] |= nextInstr->arg2;
			break;
		case Mnemonic::Xor_R_R:
			_accessRegister(nextInstr->arg1) ^= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Xor_R_A:
			_accessRegister(nextInstr->arg1) ^= program[nextInstr->arg2];
			break;
		case Mnemonic::Xor_R_I:
			_accessRegister(nextInstr->arg1) ^= program[_accessRegister(nextInstr->arg2)];
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
			program[nextInstr->arg1] ^= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Xor_A_V:
			program[nextInstr->arg1] ^= nextInstr->arg2;
			break;
		case Mnemonic::Xor_I_R:
			program[_accessRegister(nextInstr->arg1)] ^= _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::Xor_I_A:
			program[_accessRegister(nextInstr->arg1)] ^= program[nextInstr->arg2];
			break;
		case Mnemonic::Xor_I_I:
			program[_accessRegister(nextInstr->arg1)] ^= program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Xor_I_V:
			program[_accessRegister(nextInstr->arg1)] ^= nextInstr->arg2;
			break;
		case Mnemonic::Eq_R_R:
			setControl(_accessRegister(nextInstr->arg1) == _accessRegister(nextInstr->arg2), TestEqual | TestBiggerEqual | TestSmallerEqual);
			break;
		case Mnemonic::Eq_R_A:
			setControl(_accessRegister(nextInstr->arg1) == program[nextInstr->arg2], TestEqual | TestBiggerEqual | TestSmallerEqual);
			break;
		case Mnemonic::Eq_R_I:
			setControl(_accessRegister(nextInstr->arg1) == program[_accessRegister(nextInstr->arg2)], TestEqual | TestBiggerEqual | TestSmallerEqual);
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
			setControl(program[nextInstr->arg1] == program[_accessRegister(nextInstr->arg2)], TestEqual | TestBiggerEqual | TestSmallerEqual);
			break;
		case Mnemonic::Eq_A_V:
			setControl(program[nextInstr->arg1] == nextInstr->arg2, TestEqual | TestBiggerEqual | TestSmallerEqual);
			break;
		case Mnemonic::Eq_I_R:
			setControl(program[_accessRegister(nextInstr->arg1)] == _accessRegister(nextInstr->arg2), TestEqual | TestBiggerEqual | TestSmallerEqual);
			break;
		case Mnemonic::Eq_I_A:
			setControl(program[_accessRegister(nextInstr->arg1)] == program[nextInstr->arg2], TestEqual | TestBiggerEqual | TestSmallerEqual);
			break;
		case Mnemonic::Eq_I_I:
			setControl(program[_accessRegister(nextInstr->arg1)] == program[_accessRegister(nextInstr->arg2)], TestEqual | TestBiggerEqual | TestSmallerEqual);
			break;
		case Mnemonic::Eq_I_V:
			setControl(program[_accessRegister(nextInstr->arg1)] == nextInstr->arg2, TestEqual | TestBiggerEqual | TestSmallerEqual);
			break;
		case Mnemonic::Neq_R_R:
			setControl(_accessRegister(nextInstr->arg1) != _accessRegister(nextInstr->arg2), TestUnequal);
			break;
		case Mnemonic::Neq_R_A:
			setControl(_accessRegister(nextInstr->arg1) != program[nextInstr->arg2], TestUnequal);
			break;
		case Mnemonic::Neq_R_I:
			setControl(_accessRegister(nextInstr->arg1) != program[_accessRegister(nextInstr->arg2)], TestUnequal);
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
			setControl(program[nextInstr->arg1] != program[_accessRegister(nextInstr->arg2)], TestUnequal);
			break;
		case Mnemonic::Neq_A_V:
			setControl(program[nextInstr->arg1] != nextInstr->arg2, TestUnequal);
			break;
		case Mnemonic::Neq_I_R:
			setControl(program[_accessRegister(nextInstr->arg1)] != _accessRegister(nextInstr->arg2), TestUnequal);
			break;
		case Mnemonic::Neq_I_A:
			setControl(program[_accessRegister(nextInstr->arg1)] != program[nextInstr->arg2], TestUnequal);
			break;
		case Mnemonic::Neq_I_I:
			setControl(program[_accessRegister(nextInstr->arg1)] != program[_accessRegister(nextInstr->arg2)], TestUnequal);
			break;
		case Mnemonic::Neq_I_V:
			setControl(program[_accessRegister(nextInstr->arg1)] != nextInstr->arg2, TestUnequal);
			break;
		case Mnemonic::Lt_R_R:
			setControl(_accessRegister(nextInstr->arg1) < _accessRegister(nextInstr->arg2), TestSmaller | TestUnequal | TestSmallerEqual);
			break;
		case Mnemonic::Lt_R_A:
			setControl(_accessRegister(nextInstr->arg1) < program[nextInstr->arg2], TestSmaller | TestUnequal | TestSmallerEqual);
			break;
		case Mnemonic::Lt_R_I:
			setControl(_accessRegister(nextInstr->arg1) < program[_accessRegister(nextInstr->arg2)], TestSmaller | TestUnequal | TestSmallerEqual);
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
			setControl(program[nextInstr->arg1] < program[_accessRegister(nextInstr->arg2)], TestSmaller | TestUnequal | TestSmallerEqual);
			break;
		case Mnemonic::Lt_A_V:
			setControl(program[nextInstr->arg1] < nextInstr->arg2, TestSmaller | TestUnequal | TestSmallerEqual);
			break;
		case Mnemonic::Lt_I_R:
			setControl(program[_accessRegister(nextInstr->arg1)] < _accessRegister(nextInstr->arg2), TestSmaller | TestUnequal | TestSmallerEqual);
			break;
		case Mnemonic::Lt_I_A:
			setControl(program[_accessRegister(nextInstr->arg1)] < program[nextInstr->arg2], TestSmaller | TestUnequal | TestSmallerEqual);
			break;
		case Mnemonic::Lt_I_I:
			setControl(program[_accessRegister(nextInstr->arg1)] < program[_accessRegister(nextInstr->arg2)], TestSmaller | TestUnequal | TestSmallerEqual);
			break;
		case Mnemonic::Lt_I_V:
			setControl(program[_accessRegister(nextInstr->arg1)] < nextInstr->arg2, TestSmaller | TestUnequal | TestSmallerEqual);
			break;
		case Mnemonic::Bt_R_R:
			setControl(_accessRegister(nextInstr->arg1) > _accessRegister(nextInstr->arg2), TestBigger | TestUnequal | TestBiggerEqual);
			break;
		case Mnemonic::Bt_R_A:
			setControl(_accessRegister(nextInstr->arg1) > program[nextInstr->arg2], TestBigger | TestUnequal | TestBiggerEqual);
			break;
		case Mnemonic::Bt_R_I:
			setControl(_accessRegister(nextInstr->arg1) > program[_accessRegister(nextInstr->arg2)], TestBigger | TestUnequal | TestBiggerEqual);
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
			setControl(program[nextInstr->arg1] > program[_accessRegister(nextInstr->arg2)], TestBigger | TestUnequal | TestBiggerEqual);
			break;
		case Mnemonic::Bt_A_V:
			setControl(program[nextInstr->arg1] > nextInstr->arg2, TestBigger | TestUnequal | TestBiggerEqual);
			break;
		case Mnemonic::Bt_I_R:
			setControl(program[_accessRegister(nextInstr->arg1)] > _accessRegister(nextInstr->arg2), TestBigger | TestUnequal | TestBiggerEqual);
			break;
		case Mnemonic::Bt_I_A:
			setControl(program[_accessRegister(nextInstr->arg1)] > program[nextInstr->arg2], TestBigger | TestUnequal | TestBiggerEqual);
			break;
		case Mnemonic::Bt_I_I:
			setControl(program[_accessRegister(nextInstr->arg1)] > program[_accessRegister(nextInstr->arg2)], TestBigger | TestUnequal | TestBiggerEqual);
			break;
		case Mnemonic::Bt_I_V:
			setControl(program[_accessRegister(nextInstr->arg1)] > nextInstr->arg2, TestBigger | TestUnequal | TestBiggerEqual);
			break;

		default:
			return _perform2(nextInstr);
	}

	return true;
}

bool VM::_perform2(Instruction* nextInstr) {
	switch (nextInstr->mnemonic) {
		case Mnemonic::Loadload_R_A:
			_accessRegister(nextInstr->arg1) = program[program[nextInstr->arg2]];
			break;
		case Mnemonic::Loadload_R_I:
			_accessRegister(nextInstr->arg1) = program[program[_accessRegister(nextInstr->arg2)]];
			break;
		case Mnemonic::Loadload_A_A:
			program[nextInstr->arg1] = program[program[nextInstr->arg2]];
			break;
		case Mnemonic::Loadload_A_I:
			program[nextInstr->arg1] = program[program[_accessRegister(nextInstr->arg2)]];
			break;
		case Mnemonic::Loadload_I_A:
			program[_accessRegister(nextInstr->arg1)] = program[program[nextInstr->arg2]];
			break;
		case Mnemonic::Loadload_I_I:
			program[_accessRegister(nextInstr->arg1)] = program[program[_accessRegister(nextInstr->arg2)]];
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
			instrPtr = program[_accessRegister(nextInstr->arg1) + program[_accessRegister(nextInstr->arg2)]];
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
			instrPtr = program[program[nextInstr->arg1] + program[_accessRegister(nextInstr->arg2)]];
			break;
		case Mnemonic::Vcall_A_V:
			_pushStack(instrPtr);
			instrPtr = program[program[nextInstr->arg1] + nextInstr->arg2];
			break;
		case Mnemonic::Vcall_I_R:
			_pushStack(instrPtr);
			instrPtr = program[program[_accessRegister(nextInstr->arg1)] + _accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::Vcall_I_A:
			_pushStack(instrPtr);
			instrPtr = program[program[_accessRegister(nextInstr->arg1)] + program[nextInstr->arg2]];
			break;
		case Mnemonic::Vcall_I_I:
			_pushStack(instrPtr);
			instrPtr = program[program[_accessRegister(nextInstr->arg1)] + program[_accessRegister(nextInstr->arg2)]];
			break;
		case Mnemonic::Vcall_I_V:
			_pushStack(instrPtr);
			instrPtr = program[program[_accessRegister(nextInstr->arg1)] + nextInstr->arg2];
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
			instrPtr = program[nextInstr->arg1 + program[_accessRegister(nextInstr->arg2)]];
			break;
		case Mnemonic::Vcall_V_V:
			_pushStack(instrPtr);
			instrPtr = program[nextInstr->arg1 + nextInstr->arg2];
			break;
		case Mnemonic::RegInt_R_R:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired, Mnemonic::RegInt_R_R, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(_accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Regint_R_I:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired, Mnemonic::Regint_R_I, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(_accessRegister(nextInstr->arg1), program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::Regint_R_A:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired, Mnemonic::Regint_R_A, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(_accessRegister(nextInstr->arg1), nextInstr->arg2);
			break;
		case Mnemonic::RegInt_A_R:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired, Mnemonic::RegInt_A_R, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(program[nextInstr->arg1], _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Regint_A_I:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired, Mnemonic::Regint_A_I, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(program[nextInstr->arg1], program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::Regint_A_A:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired, Mnemonic::Regint_A_A, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(program[nextInstr->arg1], nextInstr->arg2);
			break;
		case Mnemonic::RegInt_I_R:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilegeRequired, Mnemonic::RegInt_I_R, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(program[_accessRegister(nextInstr->arg1)], _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Regint_I_I:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilegeRequired, Mnemonic::Regint_I_I, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(program[_accessRegister(nextInstr->arg1)], program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::Regint_I_A:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilegeRequired, Mnemonic::Regint_I_A, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(program[_accessRegister(nextInstr->arg1)], nextInstr->arg2);
			break;
		case Mnemonic::RegInt_V_R:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired, Mnemonic::RegInt_V_R, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(nextInstr->arg1, _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Regint_V_I:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired, Mnemonic::Regint_V_I, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(nextInstr->arg1, program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::Regint_V_A:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired, Mnemonic::Regint_V_A, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(nextInstr->arg1, nextInstr->arg2);
			break;
		case Mnemonic::RRegInt_R_R:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired, Mnemonic::RRegInt_R_R, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(_accessRegister(nextInstr->arg1), instrPtr + _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::RRegInt_R_I:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired, Mnemonic::RRegInt_R_I, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(_accessRegister(nextInstr->arg1), instrPtr + program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::RRegInt_R_A:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired, Mnemonic::RRegInt_R_A, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(_accessRegister(nextInstr->arg1), instrPtr + nextInstr->arg2);
			break;
		case Mnemonic::RRegInt_A_R:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired, Mnemonic::RRegInt_A_R, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(program[nextInstr->arg1], instrPtr + _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::RRegInt_A_I:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired, Mnemonic::RRegInt_A_I, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(program[nextInstr->arg1], instrPtr + program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::RRegInt_A_A:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired, Mnemonic::RRegInt_A_A, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(program[nextInstr->arg1], instrPtr + nextInstr->arg2);
			break;
		case Mnemonic::RRegInt_I_R:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilegeRequired, Mnemonic::RRegInt_I_R, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(program[_accessRegister(nextInstr->arg1)], instrPtr + _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::RRegInt_I_I:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilegeRequired, Mnemonic::RRegInt_I_I, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(program[_accessRegister(nextInstr->arg1)], instrPtr + program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::RRegInt_I_A:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilegeRequired, Mnemonic::RRegInt_I_A, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(program[_accessRegister(nextInstr->arg1)], instrPtr + nextInstr->arg2);
			break;
		case Mnemonic::RRegInt_V_R:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired, Mnemonic::RRegInt_V_R, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(nextInstr->arg1, instrPtr + _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::RRegInt_V_I:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired, Mnemonic::RRegInt_V_I, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(nextInstr->arg1, instrPtr + program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::RRegInt_V_A:
			if (!_testPrivilege(interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired, Mnemonic::RRegInt_V_A, ErrorCode::UnprivilegedIntManip))
				return false;
			_setInterruptHandler(nextInstr->arg1, instrPtr + nextInstr->arg2);
			break;
		case Mnemonic::Time_A:
			program[nextInstr->arg1] = static_cast<uint32_t>(getTime(startExecTime));
			break;
		case Mnemonic::Time_I:
			program[_accessRegister(nextInstr->arg1)] = static_cast<uint32_t>(getTime(startExecTime));
			break;
		case Mnemonic::Time64_R:
			_writeTime(getTime(startExecTime), &_accessRegister(nextInstr->arg1));
			break;
		case Mnemonic::Time64_A:
			_writeTime(getTime(startExecTime), &program[nextInstr->arg1]);
			break;
		case Mnemonic::Time64_I:
			_writeTime(getTime(startExecTime), &program[_accessRegister(nextInstr->arg1)]);
			break;
		case Mnemonic::ICount_A:
			program[nextInstr->arg1] = static_cast<uint32_t>(instrCount);
			break;
		case Mnemonic::ICount_I:
			program[_accessRegister(nextInstr->arg1)] = static_cast<uint32_t>(instrCount);
			break;
		case Mnemonic::ICount64_R:
			_writeTime(instrCount, &_accessRegister(nextInstr->arg1));
			break;
		case Mnemonic::ICount64_A:
			_writeTime(instrCount, &program[nextInstr->arg1]);
			break;
		case Mnemonic::ICount64_I:
			_writeTime(instrCount, &program[_accessRegister(nextInstr->arg1)]);
			break;
		case Mnemonic::NtvCall_R:
			_doNativeCall(_accessRegister(nextInstr->arg1));
			break;
		case Mnemonic::NtvCall_A:
			_doNativeCall(program[nextInstr->arg1]);
			break;
		case Mnemonic::NtvCall_I:
			_doNativeCall(program[_accessRegister(nextInstr->arg1)]);
			break;
		case Mnemonic::NtvCall_V:
			_doNativeCall(nextInstr->arg1);
			break;
		case Mnemonic::RICountInt_R:
			nextInstrCountInterrupt = static_cast<uint32_t>(instrCount) + _accessRegister(nextInstr->arg1);
			break;
		case Mnemonic::RICountInt_V:
			nextInstrCountInterrupt = static_cast<uint32_t>(instrCount) + nextInstr->arg1;
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
			_accessRegister(nextInstr->arg1) = _findNativeByName(reinterpret_cast<char*>(&program[_accessRegister(nextInstr->arg2)]));
			break;
		case Mnemonic::GetNtvId_A_A:
			program[nextInstr->arg1] = _findNativeByName(reinterpret_cast<char*>(&program[nextInstr->arg2]));
			break;
		case Mnemonic::GetNtvId_A_I:
			program[nextInstr->arg1] = _findNativeByName(reinterpret_cast<char*>(&program[_accessRegister(nextInstr->arg2)]));
			break;
		case Mnemonic::GetNtvId_I_A:
			program[_accessRegister(nextInstr->arg1)] = _findNativeByName(reinterpret_cast<char*>(&program[nextInstr->arg2]));
			break;
		case Mnemonic::GetNtvId_I_I:
			program[_accessRegister(nextInstr->arg1)] = _findNativeByName(reinterpret_cast<char*>(&program[_accessRegister(nextInstr->arg2)]));
			break;
		case Mnemonic::Xchg_R_R:
			std::swap(_accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Xchg_R_A:
			std::swap(_accessRegister(nextInstr->arg1), program[nextInstr->arg2]);
			break;
		case Mnemonic::Xchg_R_I:
			std::swap(_accessRegister(nextInstr->arg1), program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::Xchg_A_R:
			std::swap(program[nextInstr->arg1], _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Xchg_A_A:
			std::swap(program[nextInstr->arg1], program[nextInstr->arg2]);
			break;
		case Mnemonic::Xchg_A_I:
			std::swap(program[nextInstr->arg1], program[_accessRegister(nextInstr->arg2)]);
			break;
		case Mnemonic::Xchg_I_R:
			std::swap(program[_accessRegister(nextInstr->arg1)], _accessRegister(nextInstr->arg2));
			break;
		case Mnemonic::Xchg_I_A:
			std::swap(program[_accessRegister(nextInstr->arg1)], program[nextInstr->arg2]);
			break;
		case Mnemonic::Xchg_I_I:
			std::swap(program[_accessRegister(nextInstr->arg1)], program[_accessRegister(nextInstr->arg2)]);
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
		case Mnemonic::Hotpatch_A_A:
			_writeN(&program[nextInstr->arg2], 3, &program[nextInstr->arg1]);
			break;
		case Mnemonic::Hotpatch_A_I:
			_writeN(&program[_accessRegister(nextInstr->arg2)], 3, &program[nextInstr->arg1]);
			break;
		case Mnemonic::Hotpatch_I_A:
			_writeN(&program[nextInstr->arg2], 3, &program[_accessRegister(nextInstr->arg1)]);
			break;
		case Mnemonic::Hotpatch_I_I:
			_writeN(&program[_accessRegister(nextInstr->arg2)], 3, &program[_accessRegister(nextInstr->arg1)]);
			break;
		case Mnemonic::RHotpatch_A_A:
			_writeN(&program[nextInstr->arg2], 3, &program[instrPtr] + nextInstr->arg1);
			break;
		case Mnemonic::RHotpatch_A_I:
			_writeN(&program[_accessRegister(nextInstr->arg2)], 3, &program[instrPtr] + nextInstr->arg1);
			break;
		case Mnemonic::RHotpatch_I_A:
			_writeN(&program[nextInstr->arg2], 3, &program[instrPtr] + program[_accessRegister(nextInstr->arg1)]);
			break;
		case Mnemonic::RHotpatch_I_I:
			_writeN(&program[_accessRegister(nextInstr->arg2)], 3, &program[instrPtr] + program[_accessRegister(nextInstr->arg1)]);
			break;
		case Mnemonic::ICountInt64_R:
			_setNextInstrCountInt(_accessRegister(nextInstr->arg1), *(&_accessRegister(nextInstr->arg1) + 1));
			break;
		case Mnemonic::ICountInt64_A:
			_setNextInstrCountInt(program[nextInstr->arg1], *(&program[nextInstr->arg1] + 1));
			break;
		case Mnemonic::ICountInt64_I:
			_setNextInstrCountInt(program[_accessRegister(nextInstr->arg1)], *(&program[_accessRegister(nextInstr->arg1)] + 1));
			break;
		case Mnemonic::RICountInt64_R:
			_setNextInstrCountInt(nextInstrCountInterrupt, _accessRegister(nextInstr->arg1), *(&_accessRegister(nextInstr->arg1) + 1));
			break;
		case Mnemonic::RICountInt64_A:
			_setNextInstrCountInt(nextInstrCountInterrupt, program[nextInstr->arg1], *(&program[nextInstr->arg1] + 1));
			break;
		case Mnemonic::RICountInt64_I:
			_setNextInstrCountInt(nextInstrCountInterrupt, program[_accessRegister(nextInstr->arg1)], *(&program[_accessRegister(nextInstr->arg1)] + 1));
			break;
		case Mnemonic::GetPrivlg_R:
			_accessRegister(nextInstr->arg1) = privilegeLevel;
			break;
		case Mnemonic::GetPrivlg_A:
			program[nextInstr->arg1] = privilegeLevel;
			break;
		case Mnemonic::GetPrivlg_I:
			program[_accessRegister(nextInstr->arg1)] = privilegeLevel;
			break;
		case Mnemonic::SetInstrPrivlg_R_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetInstrPrivlg_R_R))
				return false;
			instrPrivileges[_accessRegister(nextInstr->arg1)] = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetInstrPrivlg_R_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetInstrPrivlg_R_A))
				return false;
			instrPrivileges[_accessRegister(nextInstr->arg1)] = program[nextInstr->arg2];
			break;
		case Mnemonic::SetInstrPrivlg_R_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetInstrPrivlg_R_I))
				return false;
			instrPrivileges[_accessRegister(nextInstr->arg1)] = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetInstrPrivlg_R_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetInstrPrivlg_R_V))
				return false;
			instrPrivileges[_accessRegister(nextInstr->arg1)] = nextInstr->arg2;
			break;
		case Mnemonic::SetInstrPrivlg_A_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetInstrPrivlg_A_R))
				return false;
			instrPrivileges[program[nextInstr->arg1]] = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetInstrPrivlg_A_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetInstrPrivlg_A_A))
				return false;
			instrPrivileges[program[nextInstr->arg1]] = program[nextInstr->arg2];
			break;
		case Mnemonic::SetInstrPrivlg_A_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetInstrPrivlg_A_I))
				return false;
			instrPrivileges[program[nextInstr->arg1]] = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetInstrPrivlg_A_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetInstrPrivlg_A_V))
				return false;
			instrPrivileges[program[nextInstr->arg1]] = nextInstr->arg2;
			break;
		case Mnemonic::SetInstrPrivlg_I_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetInstrPrivlg_I_R))
				return false;
			instrPrivileges[program[_accessRegister(nextInstr->arg1)]] = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetInstrPrivlg_I_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetInstrPrivlg_I_A))
				return false;
			instrPrivileges[program[_accessRegister(nextInstr->arg1)]] = program[nextInstr->arg2];
			break;
		case Mnemonic::SetInstrPrivlg_I_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetInstrPrivlg_I_I))
				return false;
			instrPrivileges[program[_accessRegister(nextInstr->arg1)]] = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetInstrPrivlg_I_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetInstrPrivlg_I_V))
				return false;
			instrPrivileges[program[_accessRegister(nextInstr->arg1)]] = nextInstr->arg2;
			break;
		case Mnemonic::SetInstrPrivlg_V_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetInstrPrivlg_V_R))
				return false;
			instrPrivileges[nextInstr->arg1] = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetInstrPrivlg_V_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetInstrPrivlg_V_A))
				return false;
			instrPrivileges[nextInstr->arg1] = program[nextInstr->arg2];
			break;
		case Mnemonic::SetInstrPrivlg_V_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetInstrPrivlg_V_I))
				return false;
			instrPrivileges[nextInstr->arg1] = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetInstrPrivlg_V_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetInstrPrivlg_V_V))
				return false;
			instrPrivileges[nextInstr->arg1] = nextInstr->arg2;
			break;
		case Mnemonic::SetPrivlg_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg1)), Mnemonic::SetPrivlg_R))
				return false;
			{
				auto& dest = handling == InterruptType::NoInterrupt ? privilegeLevel : intPrivSet;
				dest = _accessRegister(nextInstr->arg1);
			}
			break;
		case Mnemonic::SetPrivlg_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg1]), Mnemonic::SetPrivlg_A))
				return false;
			{
				auto& dest = handling == InterruptType::NoInterrupt ? privilegeLevel : intPrivSet;
				dest = program[nextInstr->arg1];
			}
			break;
		case Mnemonic::SetPrivlg_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)]), Mnemonic::SetPrivlg_I))
				return false;
			{
				auto& dest = handling == InterruptType::NoInterrupt ? privilegeLevel : intPrivSet;
				dest = program[_accessRegister(nextInstr->arg1)];
			}
			break;
		case Mnemonic::SetPrivlg_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg1), Mnemonic::SetPrivlg_V))
				return false;
			{
				auto& dest = handling == InterruptType::NoInterrupt ? privilegeLevel : intPrivSet;
				dest = nextInstr->arg1;
			}
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
			if (program[_accessRegister(nextInstr->arg2)] >= Mnemonic::TotalCount) {
				error = { ErrorCode::InvalidInstruction, instrPtr };
				running = false;
				return false;
			}
			_accessRegister(nextInstr->arg1) = instrPrivileges[program[_accessRegister(nextInstr->arg2)]];
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
			if (program[_accessRegister(nextInstr->arg2)] >= Mnemonic::TotalCount) {
				error = { ErrorCode::InvalidInstruction, instrPtr };
				running = false;
				return false;
			}
			program[nextInstr->arg1] = instrPrivileges[program[_accessRegister(nextInstr->arg2)]];
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
			program[_accessRegister(nextInstr->arg1)] = instrPrivileges[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::GetInstrPrivlg_I_A:
			if (program[nextInstr->arg2] >= Mnemonic::TotalCount) {
				error = { ErrorCode::InvalidInstruction, instrPtr };
				running = false;
				return false;
			}
			program[_accessRegister(nextInstr->arg1)] = instrPrivileges[program[nextInstr->arg2]];
			break;
		case Mnemonic::GetInstrPrivlg_I_I:
			if (program[_accessRegister(nextInstr->arg2)] >= Mnemonic::TotalCount) {
				error = { ErrorCode::InvalidInstruction, instrPtr };
				running = false;
				return false;
			}
			program[_accessRegister(nextInstr->arg1)] = instrPrivileges[program[_accessRegister(nextInstr->arg2)]];
			break;
		case Mnemonic::GetInstrPrivlg_I_V:
			if (nextInstr->arg2 >= Mnemonic::TotalCount) {
				error = { ErrorCode::InvalidInstruction, instrPtr };
				running = false;
				return false;
			}
			program[_accessRegister(nextInstr->arg1)] = instrPrivileges[nextInstr->arg2];
			break;
		case Mnemonic::SetIntPrivlg_R_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetIntPrivlg_R_R))
				return false;
			interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetIntPrivlg_R_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetIntPrivlg_R_A))
				return false;
			interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = program[nextInstr->arg2];
			break;
		case Mnemonic::SetIntPrivlg_R_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetIntPrivlg_R_I))
				return false;
			interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetIntPrivlg_R_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetIntPrivlg_R_V))
				return false;
			interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = nextInstr->arg2;
			break;
		case Mnemonic::SetIntPrivlg_A_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetIntPrivlg_A_R))
				return false;
			interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetIntPrivlg_A_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetIntPrivlg_A_A))
				return false;
			interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = program[nextInstr->arg2];
			break;
		case Mnemonic::SetIntPrivlg_A_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetIntPrivlg_A_I))
				return false;
			interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetIntPrivlg_A_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetIntPrivlg_A_V))
				return false;
			interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = nextInstr->arg2;
			break;
		case Mnemonic::SetIntPrivlg_I_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetIntPrivlg_I_R))
				return false;
			interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilege = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetIntPrivlg_I_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetIntPrivlg_I_A))
				return false;
			interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilege = program[nextInstr->arg2];
			break;
		case Mnemonic::SetIntPrivlg_I_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetIntPrivlg_I_I))
				return false;
			interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilege = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetIntPrivlg_I_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetIntPrivlg_I_V))
				return false;
			interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilege = nextInstr->arg2;
			break;
		case Mnemonic::SetIntPrivlg_V_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetIntPrivlg_V_R))
				return false;
			interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilege = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetIntPrivlg_V_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetIntPrivlg_V_A))
				return false;
			interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilege = program[nextInstr->arg2];
			break;
		case Mnemonic::SetIntPrivlg_V_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetIntPrivlg_V_I))
				return false;
			interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilege = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetIntPrivlg_V_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetIntPrivlg_V_V))
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
			_accessRegister(nextInstr->arg1) = interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)])].privilege;
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
			program[nextInstr->arg1] = interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)])].privilege;
			break;
		case Mnemonic::GetIntPrivlg_A_V:
			program[nextInstr->arg1] = interrupts[static_cast<uint8_t>(nextInstr->arg2)].privilege;
			break;
		case Mnemonic::GetIntPrivlg_I_R:
			program[_accessRegister(nextInstr->arg1)] = interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].privilege;
			break;
		case Mnemonic::GetIntPrivlg_I_A:
			program[_accessRegister(nextInstr->arg1)] = interrupts[static_cast<uint8_t>(program[nextInstr->arg2])].privilege;
			break;
		case Mnemonic::GetIntPrivlg_I_I:
			program[_accessRegister(nextInstr->arg1)] = interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)])].privilege;
			break;
		case Mnemonic::GetIntPrivlg_I_V:
			program[_accessRegister(nextInstr->arg1)] = interrupts[static_cast<uint8_t>(nextInstr->arg2)].privilege;
			break;
		case Mnemonic::SetIntExecPrivlg_R_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetIntExecPrivlg_R_R))
				return false;
			interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetIntExecPrivlg_R_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetIntExecPrivlg_R_A))
				return false;
			interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired = program[nextInstr->arg2];
			break;
		case Mnemonic::SetIntExecPrivlg_R_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetIntExecPrivlg_R_I))
				return false;
			interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetIntExecPrivlg_R_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetIntExecPrivlg_R_V))
				return false;
			interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilegeRequired = nextInstr->arg2;
			break;
		case Mnemonic::SetIntExecPrivlg_A_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetIntExecPrivlg_A_R))
				return false;
			interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetIntExecPrivlg_A_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetIntExecPrivlg_A_A))
				return false;
			interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired = program[nextInstr->arg2];
			break;
		case Mnemonic::SetIntExecPrivlg_A_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetIntExecPrivlg_A_I))
				return false;
			interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetIntExecPrivlg_A_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetIntExecPrivlg_A_V))
				return false;
			interrupts[static_cast<uint8_t>(program[nextInstr->arg1])].privilegeRequired = nextInstr->arg2;
			break;
		case Mnemonic::SetIntExecPrivlg_I_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetIntExecPrivlg_I_R))
				return false;
			interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilegeRequired = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetIntExecPrivlg_I_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetIntExecPrivlg_I_A))
				return false;
			interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilegeRequired = program[nextInstr->arg2];
			break;
		case Mnemonic::SetIntExecPrivlg_I_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetIntExecPrivlg_I_I))
				return false;
			interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilegeRequired = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetIntExecPrivlg_I_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetIntExecPrivlg_I_V))
				return false;
			interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilegeRequired = nextInstr->arg2;
			break;
		case Mnemonic::SetIntExecPrivlg_V_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetIntExecPrivlg_V_R))
				return false;
			interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetIntExecPrivlg_V_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetIntExecPrivlg_V_A))
				return false;
			interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired = program[nextInstr->arg2];
			break;
		case Mnemonic::SetIntExecPrivlg_V_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetIntExecPrivlg_V_I))
				return false;
			interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetIntExecPrivlg_V_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetIntExecPrivlg_V_V))
				return false;
			interrupts[static_cast<uint8_t>(nextInstr->arg1)].privilegeRequired = nextInstr->arg2;
			break;
		case Mnemonic::GetIntExecPrivlg_R_R:
			_accessRegister(nextInstr->arg1) = interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].privilegeRequired;
			break;
		case Mnemonic::GetIntExecPrivlg_R_A:
			_accessRegister(nextInstr->arg1) = interrupts[static_cast<uint8_t>(program[nextInstr->arg2])].privilegeRequired;
			break;
		case Mnemonic::GetIntExecPrivlg_R_I:
			_accessRegister(nextInstr->arg1) = interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)])].privilegeRequired;
			break;
		case Mnemonic::GetIntExecPrivlg_R_V:
			_accessRegister(nextInstr->arg1) = interrupts[static_cast<uint8_t>(nextInstr->arg2)].privilegeRequired;
			break;
		case Mnemonic::GetIntExecPrivlg_A_R:
			program[nextInstr->arg1] = interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].privilegeRequired;
			break;
		case Mnemonic::GetIntExecPrivlg_A_A:
			program[nextInstr->arg1] = interrupts[static_cast<uint8_t>(program[nextInstr->arg2])].privilegeRequired;
			break;
		case Mnemonic::GetIntExecPrivlg_A_I:
			program[nextInstr->arg1] = interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)])].privilegeRequired;
			break;
		case Mnemonic::GetIntExecPrivlg_A_V:
			program[nextInstr->arg1] = interrupts[static_cast<uint8_t>(nextInstr->arg2)].privilegeRequired;
			break;
		case Mnemonic::GetIntExecPrivlg_I_R:
			program[_accessRegister(nextInstr->arg1)] = interrupts[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].privilegeRequired;
			break;
		case Mnemonic::GetIntExecPrivlg_I_A:
			program[_accessRegister(nextInstr->arg1)] = interrupts[static_cast<uint8_t>(program[nextInstr->arg2])].privilegeRequired;
			break;
		case Mnemonic::GetIntExecPrivlg_I_I:
			program[_accessRegister(nextInstr->arg1)] = interrupts[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)])].privilegeRequired;
			break;
		case Mnemonic::GetIntExecPrivlg_I_V:
			program[_accessRegister(nextInstr->arg1)] = interrupts[static_cast<uint8_t>(nextInstr->arg2)].privilegeRequired;
			break;
		case Mnemonic::SetExtPrivlg_R_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetExtPrivlg_R_R))
				return false;
			extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetExtPrivlg_R_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetExtPrivlg_R_A))
				return false;
			extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = program[nextInstr->arg2];
			break;
		case Mnemonic::SetExtPrivlg_R_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetExtPrivlg_R_I))
				return false;
			extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetExtPrivlg_R_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetExtPrivlg_R_V))
				return false;
			extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].privilege = nextInstr->arg2;
			break;
		case Mnemonic::SetExtPrivlg_A_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetExtPrivlg_A_R))
				return false;
			extensionData[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetExtPrivlg_A_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetExtPrivlg_A_A))
				return false;
			extensionData[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = program[nextInstr->arg2];
			break;
		case Mnemonic::SetExtPrivlg_A_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetExtPrivlg_A_I))
				return false;
			extensionData[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetExtPrivlg_A_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetExtPrivlg_A_V))
				return false;
			extensionData[static_cast<uint8_t>(program[nextInstr->arg1])].privilege = nextInstr->arg2;
			break;
		case Mnemonic::SetExtPrivlg_I_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetExtPrivlg_I_R))
				return false;
			extensionData[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilege = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetExtPrivlg_I_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetExtPrivlg_I_A))
				return false;
			extensionData[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilege = program[nextInstr->arg2];
			break;
		case Mnemonic::SetExtPrivlg_I_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetExtPrivlg_I_I))
				return false;
			extensionData[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilege = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetExtPrivlg_I_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetExtPrivlg_I_V))
				return false;
			extensionData[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].privilege = nextInstr->arg2;
			break;
		case Mnemonic::SetExtPrivlg_V_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::SetExtPrivlg_V_R))
				return false;
			extensionData[static_cast<uint8_t>(nextInstr->arg1)].privilege = _accessRegister(nextInstr->arg2);
			break;
		case Mnemonic::SetExtPrivlg_V_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::SetExtPrivlg_V_A))
				return false;
			extensionData[static_cast<uint8_t>(nextInstr->arg1)].privilege = program[nextInstr->arg2];
			break;
		case Mnemonic::SetExtPrivlg_V_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::SetExtPrivlg_V_I))
				return false;
			extensionData[static_cast<uint8_t>(nextInstr->arg1)].privilege = program[_accessRegister(nextInstr->arg2)];
			break;
		case Mnemonic::SetExtPrivlg_V_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::SetExtPrivlg_V_V))
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
			_accessRegister(nextInstr->arg1) = extensionData[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)])].privilege;
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
			program[nextInstr->arg1] = extensionData[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)])].privilege;
			break;
		case Mnemonic::GetExtPrivlg_A_V:
			program[nextInstr->arg1] = extensionData[static_cast<uint8_t>(nextInstr->arg2)].privilege;
			break;
		case Mnemonic::GetExtPrivlg_I_R:
			program[_accessRegister(nextInstr->arg1)] = extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].privilege;
			break;
		case Mnemonic::GetExtPrivlg_I_A:
			program[_accessRegister(nextInstr->arg1)] = extensionData[static_cast<uint8_t>(program[nextInstr->arg2])].privilege;
			break;
		case Mnemonic::GetExtPrivlg_I_I:
			program[_accessRegister(nextInstr->arg1)] = extensionData[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)])].privilege;
			break;
		case Mnemonic::GetExtPrivlg_I_V:
			program[_accessRegister(nextInstr->arg1)] = extensionData[static_cast<uint8_t>(nextInstr->arg2)].privilege;
			break;
		case Mnemonic::PCall_R_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::PCall_R_R))
				return false;
			++callDepth;
			return _executeFunc(_accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2), Mnemonic::PCall_R_R);
			break;
		case Mnemonic::PCall_R_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::PCall_R_A))
				return false;
			++callDepth;
			return _executeFunc(_accessRegister(nextInstr->arg1), program[nextInstr->arg2], Mnemonic::PCall_R_A);
			break;
		case Mnemonic::PCall_R_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::PCall_R_I))
				return false;
			++callDepth;
			return _executeFunc(_accessRegister(nextInstr->arg1), program[_accessRegister(nextInstr->arg2)], Mnemonic::PCall_R_I);
			break;
		case Mnemonic::PCall_R_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::PCall_R_V))
				return false;
			++callDepth;
			return _executeFunc(_accessRegister(nextInstr->arg1), nextInstr->arg2, Mnemonic::PCall_R_V);
			break;
		case Mnemonic::PCall_A_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::PCall_A_R))
				return false;
			++callDepth;
			return _executeFunc(nextInstr->arg1, _accessRegister(nextInstr->arg2), Mnemonic::PCall_A_R);
			break;
		case Mnemonic::PCall_A_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::PCall_A_A))
				return false;
			++callDepth;
			return _executeFunc(nextInstr->arg1, program[nextInstr->arg2], Mnemonic::PCall_A_A);
			break;
		case Mnemonic::PCall_A_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::PCall_A_I))
				return false;
			++callDepth;
			return _executeFunc(nextInstr->arg1, program[_accessRegister(nextInstr->arg2)], Mnemonic::PCall_A_I);
			break;
		case Mnemonic::PCall_A_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::PCall_A_V))
				return false;
			++callDepth;
			return _executeFunc(nextInstr->arg1, nextInstr->arg2, Mnemonic::PCall_A_V);
			break;
		case Mnemonic::PCall_I_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::PCall_I_R))
				return false;
			++callDepth;
			return _executeFunc(program[_accessRegister(nextInstr->arg1)], _accessRegister(nextInstr->arg2), Mnemonic::PCall_I_R);
			break;
		case Mnemonic::PCall_I_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::PCall_I_A))
				return false;
			++callDepth;
			return _executeFunc(program[_accessRegister(nextInstr->arg1)], program[nextInstr->arg2], Mnemonic::PCall_I_A);
			break;
		case Mnemonic::PCall_I_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::PCall_I_I))
				return false;
			++callDepth;
			return _executeFunc(program[_accessRegister(nextInstr->arg1)], program[_accessRegister(nextInstr->arg2)], Mnemonic::PCall_I_I);
			break;
		case Mnemonic::PCall_I_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::PCall_I_V))
				return false;
			++callDepth;
			return _executeFunc(program[_accessRegister(nextInstr->arg1)], nextInstr->arg2, Mnemonic::PCall_I_V);
			break;
		case Mnemonic::RPCall_R_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::RPCall_R_R))
				return false;
			++callDepth;
			return _executeFunc(instrPtr + _accessRegister(nextInstr->arg1), _accessRegister(nextInstr->arg2), Mnemonic::RPCall_R_R);
			break;
		case Mnemonic::RPCall_R_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::RPCall_R_A))
				return false;
			++callDepth;
			return _executeFunc(instrPtr + _accessRegister(nextInstr->arg1), program[nextInstr->arg2], Mnemonic::RPCall_R_A);
			break;
		case Mnemonic::RPCall_R_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::RPCall_R_I))
				return false;
			++callDepth;
			return _executeFunc(instrPtr + _accessRegister(nextInstr->arg1), program[_accessRegister(nextInstr->arg2)], Mnemonic::RPCall_R_I);
			break;
		case Mnemonic::RPCall_R_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::RPCall_R_V))
				return false;
			++callDepth;
			return _executeFunc(instrPtr + _accessRegister(nextInstr->arg1), nextInstr->arg2, Mnemonic::RPCall_R_V);
			break;
		case Mnemonic::RPCall_A_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::RPCall_A_R))
				return false;
			++callDepth;
			return _executeFunc(instrPtr + nextInstr->arg1, _accessRegister(nextInstr->arg2), Mnemonic::RPCall_A_R);
			break;
		case Mnemonic::RPCall_A_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::RPCall_A_A))
				return false;
			++callDepth;
			return _executeFunc(instrPtr + nextInstr->arg1, program[nextInstr->arg2], Mnemonic::RPCall_A_A);
			break;
		case Mnemonic::RPCall_A_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::RPCall_A_I))
				return false;
			++callDepth;
			return _executeFunc(instrPtr + nextInstr->arg1, program[_accessRegister(nextInstr->arg2)], Mnemonic::RPCall_A_I);
			break;
		case Mnemonic::RPCall_A_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::RPCall_A_V))
				return false;
			++callDepth;
			return _executeFunc(instrPtr + nextInstr->arg1, nextInstr->arg2, Mnemonic::RPCall_A_V);
			break;
		case Mnemonic::RPCall_I_R:
			if (!_testPrivilege(static_cast<uint8_t>(_accessRegister(nextInstr->arg2)), Mnemonic::RPCall_I_R))
				return false;
			++callDepth;
			return _executeFunc(instrPtr + program[_accessRegister(nextInstr->arg1)], _accessRegister(nextInstr->arg2), Mnemonic::RPCall_I_R);
			break;
		case Mnemonic::RPCall_I_A:
			if (!_testPrivilege(static_cast<uint8_t>(program[nextInstr->arg2]), Mnemonic::RPCall_I_A))
				return false;
			++callDepth;
			return _executeFunc(instrPtr + program[_accessRegister(nextInstr->arg1)], program[nextInstr->arg2], Mnemonic::RPCall_I_A);
			break;
		case Mnemonic::RPCall_I_I:
			if (!_testPrivilege(static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)]), Mnemonic::RPCall_I_I))
				return false;
			++callDepth;
			return _executeFunc(instrPtr + program[_accessRegister(nextInstr->arg1)], program[_accessRegister(nextInstr->arg2)], Mnemonic::RPCall_I_I);
			break;
		case Mnemonic::RPCall_I_V:
			if (!_testPrivilege(static_cast<uint8_t>(nextInstr->arg2), Mnemonic::RPCall_I_V))
				return false;
			++callDepth;
			return _executeFunc(instrPtr + program[_accessRegister(nextInstr->arg1)], nextInstr->arg2, Mnemonic::RPCall_I_V);
			break;
		case Mnemonic::PrintC_R:
			std::cout << static_cast<char>(_accessRegister(nextInstr->arg1));
			break;
		case Mnemonic::PrintC_A:
			std::cout << static_cast<char>(program[nextInstr->arg1]);
			break;
		case Mnemonic::PrintC_I:
			std::cout << static_cast<char>(program[_accessRegister(nextInstr->arg1)]);
			break;
		case Mnemonic::PrintC_V:
			std::cout << static_cast<char>(nextInstr->arg1);
			break;
		case Mnemonic::EnableExt_R:
			extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].enabled = true;
			break;
		case Mnemonic::EnableExt_A:
			extensionData[static_cast<uint8_t>(program[nextInstr->arg1])].enabled = true;
			break;
		case Mnemonic::EnableExt_I:
			extensionData[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].enabled = true;
			break;
		case Mnemonic::EnableExt_V:
			extensionData[static_cast<uint8_t>(nextInstr->arg1)].enabled = true;
			break;
		case Mnemonic::DisableExt_R:
			extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg1))].enabled = false;
			break;
		case Mnemonic::DisableExt_A:
			extensionData[static_cast<uint8_t>(program[nextInstr->arg1])].enabled = false;
			break;
		case Mnemonic::DisableExt_I:
			extensionData[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg1)])].enabled = false;
			break;
		case Mnemonic::DisableExt_V:
			extensionData[static_cast<uint8_t>(nextInstr->arg1)].enabled = false;
			break;
		case Mnemonic::IsExtEnabled_R_R:
			_accessRegister(nextInstr->arg1) = extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].enabled;
			break;
		case Mnemonic::IsExtEnabled_R_A:
			_accessRegister(nextInstr->arg1) = extensionData[static_cast<uint8_t>(program[nextInstr->arg2])].enabled;
			break;
		case Mnemonic::IsExtEnabled_R_I:
			_accessRegister(nextInstr->arg1) = extensionData[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)])].enabled;
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
			program[nextInstr->arg1] = extensionData[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)])].enabled;
			break;
		case Mnemonic::IsExtEnabled_A_V:
			program[nextInstr->arg1] = extensionData[static_cast<uint8_t>(nextInstr->arg2)].enabled;
			break;
		case Mnemonic::IsExtEnabled_I_R:
			program[_accessRegister(nextInstr->arg1)] = extensionData[static_cast<uint8_t>(_accessRegister(nextInstr->arg2))].enabled;
			break;
		case Mnemonic::IsExtEnabled_I_A:
			program[_accessRegister(nextInstr->arg1)] = extensionData[static_cast<uint8_t>(program[nextInstr->arg2])].enabled;
			break;
		case Mnemonic::IsExtEnabled_I_I:
			program[_accessRegister(nextInstr->arg1)] = extensionData[static_cast<uint8_t>(program[_accessRegister(nextInstr->arg2)])].enabled;
			break;
		case Mnemonic::IsExtEnabled_I_V:
			program[_accessRegister(nextInstr->arg1)] = extensionData[static_cast<uint8_t>(nextInstr->arg2)].enabled;
			break;

		default:
			return _performExtensions(nextInstr);
	}

	return true;
}

bool VM::_performExtensions(Instruction* nextInstr) {
	auto mnem = nextInstr->mnemonic;
	if (mnem >= Mnemonic::FpMov_R_R && mnem <= Mnemonic::FpPrint_V)
		return _performFloats(nextInstr);
	return false;
}

bool VM::_performFloats(Instruction* nextInstr) {
	if (!_testPrivilege(extensionData[static_cast<uint8_t>(Extensions::FloatOperations)].privilege, Mnemonic::FloatOperation))
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
		case Mnemonic::FpCstLn2_R:
			_accessFpRegister(nextInstr->arg1) = 0.6931471806f;
			break;
		case Mnemonic::FpCstLn10_R:
			_accessFpRegister(nextInstr->arg1) = 2.302585093f;
			break;
		case Mnemonic::FpCstLog10_R:
			_accessFpRegister(nextInstr->arg1) = 0.3010299957f;
			break;
		case Mnemonic::FpPrint_R:
			std::cout << _accessFpRegister(nextInstr->arg1);
			break;
		case Mnemonic::FpPrint_V:
			std::cout << static_cast<float>(nextInstr->arg1);
			break;

		default:
			return _onInvalidDecode();
	}

	return true;
}

}