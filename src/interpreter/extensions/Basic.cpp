
#include "../VM.hpp"
#include "../../common/Instruction.hpp"

namespace sbl::vm {
	bool VM::_performBasic(Instruction* instr) {
		switch (instr->mnemonic) {
			case Mnemonic::Nop:

				break;
			case Mnemonic::Halt:
			case Mnemonic::End:
				return false;
				break;
			case Mnemonic::Ret:
				instrPtr = _popStack();
				--callDepth;
				break;
			case Mnemonic::Loop:
				_pushStack(loopPtr);
				loopPtr = instrPtr;
				break;
			case Mnemonic::Endloop:
				if (!registers[10]) {
					loopPtr = _popStack();
				}
				else if (--registers[10]) {
					instrPtr = loopPtr;
				}
				else {
					loopPtr = _popStack();
				}
				break;
			case Mnemonic::Push_R:
				_pushStack(_tryRead(Register{ instr->arg1 }));
				break;
			case Mnemonic::Push_A:
				_pushStack(_tryRead(Address{ instr->arg1 }));
				break;
			case Mnemonic::Push_I:
				_pushStack(_tryRead(Indirect{ instr->arg1 }));
				break;
			case Mnemonic::Push_V:
				_pushStack(_tryRead(Value{ instr->arg1 }));
				break;
			case Mnemonic::Push_All:
				for (size_t i = 0, j = registers.size(); i < j; ++i) {
					_pushStack(registers[i]);
				}
				break;
			case Mnemonic::Pop_R:
				_tryWrite(Register{ instr->arg1 }, _popStack());
				break;
			case Mnemonic::Pop_A:
				_tryWrite(Address{ instr->arg1 }, _popStack());
				break;
			case Mnemonic::Pop_I:
				_tryWrite(Indirect{ instr->arg1 }, _popStack());
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
			case Mnemonic::Inc_R:
				_tryAdd(Register{ instr->arg1 }, 1);
				break;
			case Mnemonic::Inc_A:
				_tryAdd(Address{ instr->arg1 }, 1);
				break;
			case Mnemonic::Inc_I:
				_tryAdd(Indirect{ instr->arg1 }, 1);
				break;
			case Mnemonic::Dec_R:
				_tryAdd(Register{ instr->arg1 }, -1);
				break;
			case Mnemonic::Dec_A:
				_tryAdd(Address{ instr->arg1 }, -1);
				break;
			case Mnemonic::Dec_I:
				_tryAdd(Indirect{ instr->arg1 }, -1);
				break;
			case Mnemonic::Call_R:
				_pushStack(instrPtr);
				instrPtr = _tryReadDeref(Register{ instr->arg1 });
				++callDepth;
				break;
			case Mnemonic::Call_A:
				_pushStack(instrPtr);
				instrPtr = _tryReadDeref(Address{ instr->arg1 });
				++callDepth;
				break;
			case Mnemonic::Call_I:
				_pushStack(instrPtr);
				instrPtr = _tryReadDeref(Indirect{ instr->arg1 });
				++callDepth;
				break;
			case Mnemonic::RCall_R:
				_pushStack(instrPtr);
				instrPtr += _tryReadDeref(Register{ instr->arg1 });
				++callDepth;
				break;
			case Mnemonic::RCall_A:
				_pushStack(instrPtr);
				instrPtr += _tryReadDeref(Address{ instr->arg1 });
				++callDepth;
				break;
			case Mnemonic::RCall_I:
				_pushStack(instrPtr);
				instrPtr += _tryReadDeref(Indirect{ instr->arg1 });
				++callDepth;
				break;
			case Mnemonic::Read_R:
				std::cin >> _tryRead(Register{ instr->arg1 });
				break;
			case Mnemonic::Read_A:
				std::cin >> _tryRead(Address{ instr->arg1 });
				break;
			case Mnemonic::Read_I:
				std::cin >> _tryRead(Indirect{ instr->arg1 });
				break;
			case Mnemonic::Readstr_A:
				std::cin >> reinterpret_cast<char*>(&_tryRead(Address{ instr->arg1 }));
				break;
			case Mnemonic::Readstr_I:
				std::cin >> reinterpret_cast<char*>(&_tryRead(Indirect{ instr->arg1 }));
				break;
			case Mnemonic::Print_R:
				std::cout << _tryRead(Register{ instr->arg1 });
				break;
			case Mnemonic::Print_A:
				std::cout << _tryRead(Address{ instr->arg1 });
				break;
			case Mnemonic::Print_I:
				std::cout << _tryRead(Indirect{ instr->arg1 });
				break;
			case Mnemonic::Print_V:
				std::cout << _tryRead(Value{ instr->arg1 });
				break;
			case Mnemonic::PrintS_R:
				std::cout << static_cast<int32_t>(_tryRead(Register{ instr->arg1 }));
				break;
			case Mnemonic::PrintS_A:
				std::cout << static_cast<int32_t>(_tryRead(Address{ instr->arg1 }));
				break;
			case Mnemonic::PrintS_I:
				std::cout << static_cast<int32_t>(_tryRead(Indirect{ instr->arg1 }));
				break;
			case Mnemonic::PrintS_V:
				std::cout << static_cast<int32_t>(_tryRead(Value{ instr->arg1 }));
				break;
			case Mnemonic::PrintC_R:
				std::cout << static_cast<char>(_tryRead(Register{ instr->arg1 }));
				break;
			case Mnemonic::PrintC_A:
				std::cout << static_cast<char>(_tryRead(Address{ instr->arg1 }));
				break;
			case Mnemonic::PrintC_I:
				std::cout << static_cast<char>(_tryRead(Indirect{ instr->arg1 }));
				break;
			case Mnemonic::PrintC_V:
				std::cout << static_cast<char>(_tryRead(Value{ instr->arg1 }));
				break;
			case Mnemonic::Printstr_A:
				std::cout << reinterpret_cast<char*>(&_tryRead(Address{ instr->arg1 }));
				break;
			case Mnemonic::Printstr_I:
				std::cout << reinterpret_cast<char*>(&_tryRead(Indirect{ instr->arg1 }));
				break;
			case Mnemonic::Jmp_R:
				instrPtr = _tryReadDeref(Register{ instr->arg1 });
				break;
			case Mnemonic::Jmp_A:
				instrPtr = _tryReadDeref(Address{ instr->arg1 });
				break;
			case Mnemonic::Jmp_I:
				instrPtr = _tryReadDeref(Indirect{ instr->arg1 });
				break;
			case Mnemonic::Jb_R:
			case Mnemonic::Jnle_R:
				if (controlByte & TestBigger) {
					instrPtr = _tryReadDeref(Register{ instr->arg1 });
				}
				break;
			case Mnemonic::Jb_A:
			case Mnemonic::Jnle_A:
				if (controlByte & TestBigger) {
					instrPtr = _tryReadDeref(Address{ instr->arg1 });
				}
				break;
			case Mnemonic::Jb_I:
			case Mnemonic::Jnle_I:
				if (controlByte & TestBigger) {
					instrPtr = _tryReadDeref(Indirect{ instr->arg1 });
				}
				break;
			case Mnemonic::Jnb_R:
			case Mnemonic::Jle_R:
				if (controlByte & TestSmallerEqual) {
					instrPtr = _tryReadDeref(Register{ instr->arg1 });
				}
				break;
			case Mnemonic::Jnb_A:
			case Mnemonic::Jle_A:
				if (controlByte & TestSmallerEqual) {
					instrPtr = _tryReadDeref(Address{ instr->arg1 });
				}
				break;
			case Mnemonic::Jnb_I:
			case Mnemonic::Jle_I:
				if (controlByte & TestSmallerEqual) {
					instrPtr = _tryReadDeref(Indirect{ instr->arg1 });
				}
				break;
			case Mnemonic::Jbe_R:
			case Mnemonic::Jnl_R:
				if (controlByte & TestBiggerEqual) {
					instrPtr = _tryReadDeref(Register{ instr->arg1 });
				}
				break;
			case Mnemonic::Jbe_A:
			case Mnemonic::Jnl_A:
				if (controlByte & TestBiggerEqual) {
					instrPtr = _tryReadDeref(Address{ instr->arg1 });
				}
				break;
			case Mnemonic::Jbe_I:
			case Mnemonic::Jnl_I:
				if (controlByte & TestBiggerEqual) {
					instrPtr = _tryReadDeref(Indirect{ instr->arg1 });
				}
				break;
			case Mnemonic::Jnbe_R:
			case Mnemonic::Jl_R:
				if (controlByte & TestSmaller) {
					instrPtr = _tryReadDeref(Register{ instr->arg1 });
				}
				break;
			case Mnemonic::Jnbe_A:
			case Mnemonic::Jl_A:
				if (controlByte & TestSmaller) {
					instrPtr = _tryReadDeref(Address{ instr->arg1 });
				}
				break;
			case Mnemonic::Jnbe_I:
			case Mnemonic::Jl_I:
				if (controlByte & TestSmaller) {
					instrPtr = _tryReadDeref(Indirect{ instr->arg1 });
				}
				break;
			case Mnemonic::Jz_R:
			case Mnemonic::Je_R:
				if (controlByte & TestEqual) {
					instrPtr = _tryReadDeref(Register{ instr->arg1 });
				}
				break;
			case Mnemonic::Jz_A:
			case Mnemonic::Je_A:
				if (controlByte & TestEqual) {
					instrPtr = _tryReadDeref(Address{ instr->arg1 });
				}
				break;
			case Mnemonic::Jz_I:
			case Mnemonic::Je_I:
				if (controlByte & TestEqual) {
					instrPtr = _tryReadDeref(Indirect{ instr->arg1 });
				}
				break;
			case Mnemonic::Jnz_R:
			case Mnemonic::Jne_R:
				if (controlByte & TestUnequal) {
					instrPtr = _tryReadDeref(Register{ instr->arg1 });
				}
				break;
			case Mnemonic::Jnz_A:
			case Mnemonic::Jne_A:
				if (controlByte & TestUnequal) {
					instrPtr = _tryReadDeref(Address{ instr->arg1 });
				}
				break;
			case Mnemonic::Jnz_I:
			case Mnemonic::Jne_I:
				if (controlByte & TestUnequal) {
					instrPtr = _tryReadDeref(Indirect{ instr->arg1 });
				}
				break;
			case Mnemonic::RJmp_R:
				instrPtr += _tryReadDeref(Register{ instr->arg1 });
				break;
			case Mnemonic::RJmp_A:
				instrPtr += _tryReadDeref(Address{ instr->arg1 });
				break;
			case Mnemonic::RJmp_I:
				instrPtr += _tryReadDeref(Indirect{ instr->arg1 });
				break;
			case Mnemonic::RJb_R:
			case Mnemonic::RJnle_R:
				if (controlByte & TestBigger) {
					instrPtr += _tryReadDeref(Register{ instr->arg1 });
				}
				break;
			case Mnemonic::RJb_A:
			case Mnemonic::RJnle_A:
				if (controlByte & TestBigger) {
					instrPtr += _tryReadDeref(Address{ instr->arg1 });
				}
				break;
			case Mnemonic::RJb_I:
			case Mnemonic::RJnle_I:
				if (controlByte & TestBigger) {
					instrPtr += _tryReadDeref(Indirect{ instr->arg1 });
				}
				break;
			case Mnemonic::RJnb_R:
			case Mnemonic::RJle_R:
				if (controlByte & TestSmallerEqual) {
					instrPtr += _tryReadDeref(Register{ instr->arg1 });
				}
				break;
			case Mnemonic::RJnb_A:
			case Mnemonic::RJle_A:
				if (controlByte & TestSmallerEqual) {
					instrPtr += _tryReadDeref(Address{ instr->arg1 });
				}
				break;
			case Mnemonic::RJnb_I:
			case Mnemonic::RJle_I:
				if (controlByte & TestSmallerEqual) {
					instrPtr += _tryReadDeref(Indirect{ instr->arg1 });
				}
				break;
			case Mnemonic::RJbe_R:
			case Mnemonic::RJnl_R:
				if (controlByte & TestBiggerEqual) {
					instrPtr += _tryReadDeref(Register{ instr->arg1 });
				}
				break;
			case Mnemonic::RJbe_A:
			case Mnemonic::RJnl_A:
				if (controlByte & TestBiggerEqual) {
					instrPtr += _tryReadDeref(Address{ instr->arg1 });
				}
				break;
			case Mnemonic::RJbe_I:
			case Mnemonic::RJnl_I:
				if (controlByte & TestBiggerEqual) {
					instrPtr += _tryReadDeref(Indirect{ instr->arg1 });
				}
				break;
			case Mnemonic::RJnbe_R:
			case Mnemonic::RJl_R:
				if (controlByte & TestSmaller) {
					instrPtr += _tryReadDeref(Register{ instr->arg1 });
				}
				break;
			case Mnemonic::RJnbe_A:
			case Mnemonic::RJl_A:
				if (controlByte & TestSmaller) {
					instrPtr += _tryReadDeref(Address{ instr->arg1 });
				}
				break;
			case Mnemonic::RJnbe_I:
			case Mnemonic::RJl_I:
				if (controlByte & TestSmaller) {
					instrPtr += _tryReadDeref(Indirect{ instr->arg1 });
				}
				break;
			case Mnemonic::RJz_R:
			case Mnemonic::RJe_R:
				if (controlByte & TestEqual) {
					instrPtr += _tryReadDeref(Register{ instr->arg1 });
				}
				break;
			case Mnemonic::RJz_A:
			case Mnemonic::RJe_A:
				if (controlByte & TestEqual) {
					instrPtr += _tryReadDeref(Address{ instr->arg1 });
				}
				break;
			case Mnemonic::RJz_I:
			case Mnemonic::RJe_I:
				if (controlByte & TestEqual) {
					instrPtr += _tryReadDeref(Indirect{ instr->arg1 });
				}
				break;
			case Mnemonic::RJnz_R:
			case Mnemonic::RJne_R:
				if (controlByte & TestUnequal) {
					instrPtr += _tryReadDeref(Register{ instr->arg1 });
				}
				break;
			case Mnemonic::RJnz_A:
			case Mnemonic::RJne_A:
				if (controlByte & TestUnequal) {
					instrPtr += _tryReadDeref(Address{ instr->arg1 });
				}
				break;
			case Mnemonic::RJnz_I:
			case Mnemonic::RJne_I:
				if (controlByte & TestUnequal) {
					instrPtr += _tryReadDeref(Indirect{ instr->arg1 });
				}
				break;
			case Mnemonic::Time_R:
				_tryWrite(Register{ instr->arg1 }, static_cast<uint32_t>(getTime(startExecTime)));
				break;
			case Mnemonic::Time_A:
				_tryWrite(Address{ instr->arg1 }, static_cast<uint32_t>(getTime(startExecTime)));
				break;
			case Mnemonic::Time_I:
				_tryWrite(Indirect{ instr->arg1 }, static_cast<uint32_t>(getTime(startExecTime)));
				break;
			case Mnemonic::Time64_R:
			{
				auto& v1 = _tryRead(Register{ instr->arg1 });
				auto& v2 = _tryRead(Register{ instr->arg1 + 1 });
				_writeTime(getTime(startExecTime), v1, v2);
			}
			break;
			case Mnemonic::Time64_A:
			{
				auto& v1 = _tryRead(Address{ instr->arg1 });
				auto& v2 = _tryRead(Address{ instr->arg1 + 1 });
				_writeTime(getTime(startExecTime), v1, v2);
			}
			break;
			case Mnemonic::Time64_I:
			{
				auto& v1 = _tryRead(Indirect{ instr->arg1 });
				auto& v2 = _tryRead(Indirect{ instr->arg1 + 1 });
				_writeTime(getTime(startExecTime), v1, v2);
			}
			break;
			case Mnemonic::ICount_R:
				_tryWrite(Register{ instr->arg1 }, static_cast<uint32_t>(instrCount));
				break;
			case Mnemonic::ICount_A:
				_tryWrite(Address{ instr->arg1 }, static_cast<uint32_t>(instrCount));
				break;
			case Mnemonic::ICount_I:
				_tryWrite(Indirect{ instr->arg1 }, static_cast<uint32_t>(instrCount));
				break;
			case Mnemonic::ICount64_R:
			{
				auto& v1 = _tryRead(Register{ instr->arg1 });
				auto& v2 = _tryRead(Register{ instr->arg1 + 1 });
				_writeTime(instrCount, v1, v2);
			}
			break;
			case Mnemonic::ICount64_A:
			{
				auto& v1 = _tryRead(Address{ instr->arg1 });
				auto& v2 = _tryRead(Address{ instr->arg1 + 1 });
				_writeTime(instrCount, v1, v2);
			}
			break;
			case Mnemonic::ICount64_I:
			{
				auto& v1 = _tryRead(Indirect{ instr->arg1 });
				auto& v2 = _tryRead(Indirect{ instr->arg1 + 1 });
				_writeTime(instrCount, v1, v2);
			}
			break;
			case Mnemonic::ExecInstr_A:
			{
				auto derefed = _tryReadDeref(Address{ instr->arg1 });
				if (!_checkExecutable(derefed))
					return false;
				auto memPtr = memory.memory.tryAccess(derefed, SegmentAccessType::Executable);
				if (!memPtr) {
					error = { ErrorCode::UnallowedSegmentExec, instrPtr };
					running = false;
					return false;
				}
				return _perform(Instruction::fromAddress(memPtr));
			}
			break;
			case Mnemonic::ExecInstr_I:
			{
				auto derefed = _tryReadDeref(Indirect{ instr->arg1 });
				if (!_checkExecutable(derefed))
					return false;
				auto memPtr = memory.memory.tryAccess(derefed, SegmentAccessType::Executable);
				if (!memPtr) {
					error = { ErrorCode::UnallowedSegmentExec, instrPtr };
					running = false;
					return false;
				}
				return _perform(Instruction::fromAddress(memPtr));
			}
			break;
			case Mnemonic::Mov_R_R:
				_tryWrite(Register{ instr->arg1 }, _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Mov_R_A:
				_tryWrite(Register{ instr->arg1 }, _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Mov_R_I:
				_tryWrite(Register{ instr->arg1 }, _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Mov_R_V:
				_tryWrite(Register{ instr->arg1 }, _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Mov_A_R:
				_tryWrite(Address{ instr->arg1 }, _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Mov_A_A:
				_tryWrite(Address{ instr->arg1 }, _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Mov_A_I:
				_tryWrite(Address{ instr->arg1 }, _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Mov_A_V:
				_tryWrite(Address{ instr->arg1 }, _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Mov_I_R:
				_tryWrite(Indirect{ instr->arg1 }, _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Mov_I_A:
				_tryWrite(Indirect{ instr->arg1 }, _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Mov_I_I:
				_tryWrite(Indirect{ instr->arg1 }, _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Mov_I_V:
				_tryWrite(Indirect{ instr->arg1 }, _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Move_R_R:
			case Mnemonic::Movz_R_R:
				if (controlByte & TestEqual) {
					_tryWrite(Register{ instr->arg1 }, _tryRead(Register{ instr->arg2 }));
				}
				break;
			case Mnemonic::Move_R_A:
			case Mnemonic::Movz_R_A:
				if (controlByte & TestEqual) {
					_tryWrite(Register{ instr->arg1 }, _tryRead(Address{ instr->arg2 }));
				}
				break;
			case Mnemonic::Move_R_I:
			case Mnemonic::Movz_R_I:
				if (controlByte & TestEqual) {
					_tryWrite(Register{ instr->arg1 }, _tryRead(Indirect{ instr->arg2 }));
				}
				break;
			case Mnemonic::Move_R_V:
			case Mnemonic::Movz_R_V:
				if (controlByte & TestEqual) {
					_tryWrite(Register{ instr->arg1 }, _tryRead(Value{ instr->arg2 }));
				}
				break;
			case Mnemonic::Move_A_R:
			case Mnemonic::Movz_A_R:
				if (controlByte & TestEqual) {
					_tryWrite(Address{ instr->arg1 }, _tryRead(Register{ instr->arg2 }));
				}
				break;
			case Mnemonic::Move_A_A:
			case Mnemonic::Movz_A_A:
				if (controlByte & TestEqual) {
					_tryWrite(Address{ instr->arg1 }, _tryRead(Address{ instr->arg2 }));
				}
				break;
			case Mnemonic::Move_A_I:
			case Mnemonic::Movz_A_I:
				if (controlByte & TestEqual) {
					_tryWrite(Address{ instr->arg1 }, _tryRead(Indirect{ instr->arg2 }));
				}
				break;
			case Mnemonic::Move_A_V:
			case Mnemonic::Movz_A_V:
				if (controlByte & TestEqual) {
					_tryWrite(Address{ instr->arg1 }, _tryRead(Value{ instr->arg2 }));
				}
				break;
			case Mnemonic::Move_I_R:
			case Mnemonic::Movz_I_R:
				if (controlByte & TestEqual) {
					_tryWrite(Indirect{ instr->arg1 }, _tryRead(Register{ instr->arg2 }));
				}
				break;
			case Mnemonic::Move_I_A:
			case Mnemonic::Movz_I_A:
				if (controlByte & TestEqual) {
					_tryWrite(Indirect{ instr->arg1 }, _tryRead(Address{ instr->arg2 }));
				}
				break;
			case Mnemonic::Move_I_I:
			case Mnemonic::Movz_I_I:
				if (controlByte & TestEqual) {
					_tryWrite(Indirect{ instr->arg1 }, _tryRead(Indirect{ instr->arg2 }));
				}
				break;
			case Mnemonic::Move_I_V:
			case Mnemonic::Movz_I_V:
				if (controlByte & TestEqual) {
					_tryWrite(Indirect{ instr->arg1 }, _tryRead(Value{ instr->arg2 }));
				}
				break;
			case Mnemonic::Movne_R_R:
			case Mnemonic::Movnz_R_R:
				if (controlByte & TestUnequal) {
					_tryWrite(Register{ instr->arg1 }, _tryRead(Register{ instr->arg2 }));
				}
				break;
			case Mnemonic::Movne_R_A:
			case Mnemonic::Movnz_R_A:
				if (controlByte & TestUnequal) {
					_tryWrite(Register{ instr->arg1 }, _tryRead(Address{ instr->arg2 }));
				}
				break;
			case Mnemonic::Movne_R_I:
			case Mnemonic::Movnz_R_I:
				if (controlByte & TestUnequal) {
					_tryWrite(Register{ instr->arg1 }, _tryRead(Indirect{ instr->arg2 }));
				}
				break;
			case Mnemonic::Movne_R_V:
			case Mnemonic::Movnz_R_V:
				if (controlByte & TestUnequal) {
					_tryWrite(Register{ instr->arg1 }, _tryRead(Value{ instr->arg2 }));
				}
				break;
			case Mnemonic::Movne_A_R:
			case Mnemonic::Movnz_A_R:
				if (controlByte & TestUnequal) {
					_tryWrite(Address{ instr->arg1 }, _tryRead(Register{ instr->arg2 }));
				}
				break;
			case Mnemonic::Movne_A_A:
			case Mnemonic::Movnz_A_A:
				if (controlByte & TestUnequal) {
					_tryWrite(Address{ instr->arg1 }, _tryRead(Address{ instr->arg2 }));
				}
				break;
			case Mnemonic::Movne_A_I:
			case Mnemonic::Movnz_A_I:
				if (controlByte & TestUnequal) {
					_tryWrite(Address{ instr->arg1 }, _tryRead(Indirect{ instr->arg2 }));
				}
				break;
			case Mnemonic::Movne_A_V:
			case Mnemonic::Movnz_A_V:
				if (controlByte & TestUnequal) {
					_tryWrite(Address{ instr->arg1 }, _tryRead(Value{ instr->arg2 }));
				}
				break;
			case Mnemonic::Movne_I_R:
			case Mnemonic::Movnz_I_R:
				if (controlByte & TestUnequal) {
					_tryWrite(Indirect{ instr->arg1 }, _tryRead(Register{ instr->arg2 }));
				}
				break;
			case Mnemonic::Movne_I_A:
			case Mnemonic::Movnz_I_A:
				if (controlByte & TestUnequal) {
					_tryWrite(Indirect{ instr->arg1 }, _tryRead(Address{ instr->arg2 }));
				}
				break;
			case Mnemonic::Movne_I_I:
			case Mnemonic::Movnz_I_I:
				if (controlByte & TestUnequal) {
					_tryWrite(Indirect{ instr->arg1 }, _tryRead(Indirect{ instr->arg2 }));
				}
				break;
			case Mnemonic::Movne_I_V:
			case Mnemonic::Movnz_I_V:
				if (controlByte & TestUnequal) {
					_tryWrite(Indirect{ instr->arg1 }, _tryRead(Value{ instr->arg2 }));
				}
				break;
			case Mnemonic::Laddr_R_A:
				_tryWrite(Register{ instr->arg1 }, _tryReadAddr(Address{ instr->arg2 }));
				break;
			case Mnemonic::Laddr_R_I:
				_tryWrite(Register{ instr->arg1 }, _tryReadAddr(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Laddr_A_A:
				_tryWrite(Address{ instr->arg1 }, _tryReadAddr(Address{ instr->arg2 }));
				break;
			case Mnemonic::Laddr_A_I:
				_tryWrite(Address{ instr->arg1 }, _tryReadAddr(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Laddr_I_A:
				_tryWrite(Indirect{ instr->arg1 }, _tryReadAddr(Address{ instr->arg2 }));
				break;
			case Mnemonic::Laddr_I_I:
				_tryWrite(Indirect{ instr->arg1 }, _tryReadAddr(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Loadload_R_A:
				_tryWrite(Register{ instr->arg1 }, _tryRead(Address{ _tryRead(Address{ instr->arg2 }) }));
				break;
			case Mnemonic::Loadload_R_I:
				_tryWrite(Register{ instr->arg1 }, _tryRead(Address{ _tryRead(Indirect{ instr->arg2 }) }));
				break;
			case Mnemonic::Loadload_A_A:
				_tryWrite(Address{ instr->arg1 }, _tryRead(Address{ _tryRead(Address{ instr->arg2 }) }));
				break;
			case Mnemonic::Loadload_A_I:
				_tryWrite(Address{ instr->arg1 }, _tryRead(Address{ _tryRead(Indirect{ instr->arg2 }) }));
				break;
			case Mnemonic::Loadload_I_A:
				_tryWrite(Indirect{ instr->arg1 }, _tryRead(Address{ _tryRead(Address{ instr->arg2 }) }));
				break;
			case Mnemonic::Loadload_I_I:
				_tryWrite(Indirect{ instr->arg1 }, _tryRead(Address{ _tryRead(Indirect{ instr->arg2 }) }));
				break;
			case Mnemonic::Vcall_R_R:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Register{ instr->arg1 }) + _tryRead(Register{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_R_A:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Register{ instr->arg1 }) + _tryRead(Address{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_R_I:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Register{ instr->arg1 }) + _tryRead(Indirect{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_R_V:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Register{ instr->arg1 }) + _tryRead(Value{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_A_R:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Address{ instr->arg1 }) + _tryRead(Register{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_A_A:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Address{ instr->arg1 }) + _tryRead(Address{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_A_I:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Address{ instr->arg1 }) + _tryRead(Indirect{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_A_V:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Address{ instr->arg1 }) + _tryRead(Value{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_I_R:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Indirect{ instr->arg1 }) + _tryRead(Register{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_I_A:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Indirect{ instr->arg1 }) + _tryRead(Address{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_I_I:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Indirect{ instr->arg1 }) + _tryRead(Indirect{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_I_V:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Indirect{ instr->arg1 }) + _tryRead(Value{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_V_R:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Value{ instr->arg1 }) + _tryRead(Register{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_V_A:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Value{ instr->arg1 }) + _tryRead(Address{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_V_I:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Value{ instr->arg1 }) + _tryRead(Indirect{ instr->arg2 }) });
				break;
			case Mnemonic::Vcall_V_V:
				_pushStack(instrPtr);
				instrPtr = _tryRead(Address{ _tryRead(Value{ instr->arg1 }) + _tryRead(Value{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_R_R:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Register{ instr->arg1 }) + _tryRead(Register{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_R_A:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Register{ instr->arg1 }) + _tryRead(Address{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_R_I:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Register{ instr->arg1 }) + _tryRead(Indirect{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_R_V:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Register{ instr->arg1 }) + _tryRead(Value{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_A_R:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Address{ instr->arg1 }) + _tryRead(Register{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_A_A:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Address{ instr->arg1 }) + _tryRead(Address{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_A_I:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Address{ instr->arg1 }) + _tryRead(Indirect{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_A_V:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Address{ instr->arg1 }) + _tryRead(Value{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_I_R:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Indirect{ instr->arg1 }) + _tryRead(Register{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_I_A:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Indirect{ instr->arg1 }) + _tryRead(Address{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_I_I:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Indirect{ instr->arg1 }) + _tryRead(Indirect{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_I_V:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Indirect{ instr->arg1 }) + _tryRead(Value{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_V_R:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Value{ instr->arg1 }) + _tryRead(Register{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_V_A:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Value{ instr->arg1 }) + _tryRead(Address{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_V_I:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Value{ instr->arg1 }) + _tryRead(Indirect{ instr->arg2 }) });
				break;
			case Mnemonic::RVcall_V_V:
				_pushStack(instrPtr);
				instrPtr += _tryRead(Address{ _tryRead(Value{ instr->arg1 }) + _tryRead(Value{ instr->arg2 }) });
				break;
			case Mnemonic::NtvCall_R:
				_doNativeCall(_tryRead(Register{ instr->arg1 }));
				break;
			case Mnemonic::NtvCall_A:
				_doNativeCall(_tryRead(Address{ instr->arg1 }));
				break;
			case Mnemonic::NtvCall_I:
				_doNativeCall(_tryRead(Indirect{ instr->arg1 }));
				break;
			case Mnemonic::NtvCall_V:
				_doNativeCall(_tryRead(Value{ instr->arg1 }));
				break;
			case Mnemonic::GetNtvId_R_A:
				_tryWrite(Register{ instr->arg1 }, _findNativeByName(reinterpret_cast<char*>(&_tryRead(Address{ instr->arg2 }))));
				break;
			case Mnemonic::GetNtvId_R_I:
				_tryWrite(Register{ instr->arg1 }, _findNativeByName(reinterpret_cast<char*>(&_tryRead(Indirect{ instr->arg2 }))));
				break;
			case Mnemonic::GetNtvId_A_A:
				_tryWrite(Address{ instr->arg1 }, _findNativeByName(reinterpret_cast<char*>(&_tryRead(Address{ instr->arg2 }))));
				break;
			case Mnemonic::GetNtvId_A_I:
				_tryWrite(Address{ instr->arg1 }, _findNativeByName(reinterpret_cast<char*>(&_tryRead(Indirect{ instr->arg2 }))));
				break;
			case Mnemonic::GetNtvId_I_A:
				_tryWrite(Indirect{ instr->arg1 }, _findNativeByName(reinterpret_cast<char*>(&_tryRead(Address{ instr->arg2 }))));
				break;
			case Mnemonic::GetNtvId_I_I:
				_tryWrite(Indirect{ instr->arg1 }, _findNativeByName(reinterpret_cast<char*>(&_tryRead(Indirect{ instr->arg2 }))));
				break;
			case Mnemonic::Xchg_R_R:
				std::swap(_tryRead(Register{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Xchg_R_A:
				std::swap(_tryRead(Register{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Xchg_R_I:
				std::swap(_tryRead(Register{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Xchg_A_R:
				std::swap(_tryRead(Address{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Xchg_A_A:
				std::swap(_tryRead(Address{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Xchg_A_I:
				std::swap(_tryRead(Address{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Xchg_I_R:
				std::swap(_tryRead(Indirect{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Xchg_I_A:
				std::swap(_tryRead(Indirect{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Xchg_I_I:
				std::swap(_tryRead(Indirect{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::ClrCb:
				controlByte = 0;
				break;
			case Mnemonic::Hotpatch_A_A:
				_writeN(&_tryRead(Address{ instr->arg2 }), 3, &_tryRead(Address{ instr->arg1 }));
				break;
			case Mnemonic::Hotpatch_A_I:
				_writeN(&_tryRead(Indirect{ instr->arg2 }), 3, &_tryRead(Address{ instr->arg1 }));
				break;
			case Mnemonic::Hotpatch_I_A:
				_writeN(&_tryRead(Address{ instr->arg2 }), 3, &_tryRead(Indirect{ instr->arg1 }));
				break;
			case Mnemonic::Hotpatch_I_I:
				_writeN(&_tryRead(Indirect{ instr->arg2 }), 3, &_tryRead(Indirect{ instr->arg1 }));
				break;
			case Mnemonic::RHotpatch_A_A:
				_writeN(&_tryRead(Address{ instr->arg2 }), 3, &_tryRead(_tryAdd(Address{ instrPtr }, Address{ instr->arg1 })));
				break;
			case Mnemonic::RHotpatch_A_I:
				_writeN(&_tryRead(Indirect{ instr->arg2 }), 3, &_tryRead(_tryAdd(Address{ instrPtr }, Address{ instr->arg1 })));
				break;
			case Mnemonic::RHotpatch_I_A:
				_writeN(&_tryRead(Address{ instr->arg2 }), 3, &_tryRead(_tryAdd(Address{ instrPtr }, Indirect{ instr->arg1 })));
				break;
			case Mnemonic::RHotpatch_I_I:
				_writeN(&_tryRead(Indirect{ instr->arg2 }), 3, &_tryRead(_tryAdd(Address{ instrPtr }, Indirect{ instr->arg1 })));
				break;
			case Mnemonic::EnableExt_R:
				extensionData[static_cast<uint8_t>(_tryRead(Register{ instr->arg1 }))].enabled = true;
				break;
			case Mnemonic::EnableExt_A:
				extensionData[static_cast<uint8_t>(_tryRead(Address{ instr->arg1 }))].enabled = true;
				break;
			case Mnemonic::EnableExt_I:
				extensionData[static_cast<uint8_t>(_tryRead(Indirect{ instr->arg1 }))].enabled = true;
				break;
			case Mnemonic::EnableExt_V:
				extensionData[static_cast<uint8_t>(_tryRead(Value{ instr->arg1 }))].enabled = true;
				break;
			case Mnemonic::DisableExt_R:
				extensionData[static_cast<uint8_t>(_tryRead(Register{ instr->arg1 }))].enabled = false;
				break;
			case Mnemonic::DisableExt_A:
				extensionData[static_cast<uint8_t>(_tryRead(Address{ instr->arg1 }))].enabled = false;
				break;
			case Mnemonic::DisableExt_I:
				extensionData[static_cast<uint8_t>(_tryRead(Indirect{ instr->arg1 }))].enabled = false;
				break;
			case Mnemonic::DisableExt_V:
				extensionData[static_cast<uint8_t>(_tryRead(Value{ instr->arg1 }))].enabled = false;
				break;
			case Mnemonic::IsExtEnabled_R_R:
				_tryWrite(Register{ instr->arg1 }, extensionData[static_cast<uint8_t>(_tryRead(Register{ instr->arg2 }))].enabled);
				break;
			case Mnemonic::IsExtEnabled_R_A:
				_tryWrite(Register{ instr->arg1 }, extensionData[static_cast<uint8_t>(_tryRead(Address{ instr->arg2 }))].enabled);
				break;
			case Mnemonic::IsExtEnabled_R_I:
				_tryWrite(Register{ instr->arg1 }, extensionData[static_cast<uint8_t>(_tryRead(Indirect{ instr->arg2 }))].enabled);
				break;
			case Mnemonic::IsExtEnabled_R_V:
				_tryWrite(Register{ instr->arg1 }, extensionData[static_cast<uint8_t>(_tryRead(Value{ instr->arg2 }))].enabled);
				break;
			case Mnemonic::IsExtEnabled_A_R:
				_tryWrite(Address{ instr->arg1 }, extensionData[static_cast<uint8_t>(_tryRead(Register{ instr->arg2 }))].enabled);
				break;
			case Mnemonic::IsExtEnabled_A_A:
				_tryWrite(Address{ instr->arg1 }, extensionData[static_cast<uint8_t>(_tryRead(Address{ instr->arg2 }))].enabled);
				break;
			case Mnemonic::IsExtEnabled_A_I:
				_tryWrite(Address{ instr->arg1 }, extensionData[static_cast<uint8_t>(_tryRead(Indirect{ instr->arg2 }))].enabled);
				break;
			case Mnemonic::IsExtEnabled_A_V:
				_tryWrite(Address{ instr->arg1 }, extensionData[static_cast<uint8_t>(_tryRead(Value{ instr->arg2 }))].enabled);
				break;
			case Mnemonic::IsExtEnabled_I_R:
				_tryWrite(Indirect{ instr->arg1 }, extensionData[static_cast<uint8_t>(_tryRead(Register{ instr->arg2 }))].enabled);
				break;
			case Mnemonic::IsExtEnabled_I_A:
				_tryWrite(Indirect{ instr->arg1 }, extensionData[static_cast<uint8_t>(_tryRead(Address{ instr->arg2 }))].enabled);
				break;
			case Mnemonic::IsExtEnabled_I_I:
				_tryWrite(Indirect{ instr->arg1 }, extensionData[static_cast<uint8_t>(_tryRead(Indirect{ instr->arg2 }))].enabled);
				break;
			case Mnemonic::IsExtEnabled_I_V:
				_tryWrite(Indirect{ instr->arg1 }, extensionData[static_cast<uint8_t>(_tryRead(Value{ instr->arg2 }))].enabled);
				break;
			case Mnemonic::SetSegmntAccs_R_R:
				_setSegmentAccess(_tryRead(Register{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_R_A:
				_setSegmentAccess(_tryRead(Register{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_R_I:
				_setSegmentAccess(_tryRead(Register{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_R_V:
				_setSegmentAccess(_tryRead(Register{ instr->arg1 }), _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_A_R:
				_setSegmentAccess(_tryRead(Address{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_A_A:
				_setSegmentAccess(_tryRead(Address{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_A_I:
				_setSegmentAccess(_tryRead(Address{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_A_V:
				_setSegmentAccess(_tryRead(Address{ instr->arg1 }), _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_I_R:
				_setSegmentAccess(_tryRead(Indirect{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_I_A:
				_setSegmentAccess(_tryRead(Indirect{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_I_I:
				_setSegmentAccess(_tryRead(Indirect{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_I_V:
				_setSegmentAccess(_tryRead(Indirect{ instr->arg1 }), _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_V_R:
				_setSegmentAccess(_tryRead(Value{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_V_A:
				_setSegmentAccess(_tryRead(Value{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_V_I:
				_setSegmentAccess(_tryRead(Value{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::SetSegmntAccs_V_V:
				_setSegmentAccess(_tryRead(Value{ instr->arg1 }), _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::GetSegmntAccs_R_R:
				_getSegmentAccess(_tryRead(Register{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::GetSegmntAccs_R_A:
				_getSegmentAccess(_tryRead(Register{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::GetSegmntAccs_R_I:
				_getSegmentAccess(_tryRead(Register{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::GetSegmntAccs_R_V:
				_getSegmentAccess(_tryRead(Register{ instr->arg1 }), _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::GetSegmntAccs_A_R:
				_getSegmentAccess(_tryRead(Address{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::GetSegmntAccs_A_A:
				_getSegmentAccess(_tryRead(Address{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::GetSegmntAccs_A_I:
				_getSegmentAccess(_tryRead(Address{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::GetSegmntAccs_A_V:
				_getSegmentAccess(_tryRead(Address{ instr->arg1 }), _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::GetSegmntAccs_I_R:
				_getSegmentAccess(_tryRead(Indirect{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::GetSegmntAccs_I_A:
				_getSegmentAccess(_tryRead(Indirect{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::GetSegmntAccs_I_I:
				_getSegmentAccess(_tryRead(Indirect{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::GetSegmntAccs_I_V:
				_getSegmentAccess(_tryRead(Indirect{ instr->arg1 }), _tryRead(Value{ instr->arg2 }));
				break;
		}

		return true;
	}

}	//sbl::vm