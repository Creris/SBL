
#include "../VM.hpp"
#include "../../common/Instruction.hpp"

namespace sbl::vm {
	bool VM::_performInterrupt(Instruction* instr) {
		switch (instr->mnemonic) {
			case Mnemonic::Raise_R:
			{
				auto value = _tryRead(Register{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_runInterruptCode(value);
			}
			break;
			case Mnemonic::Raise_A:
			{
				auto value = _tryRead(Address{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_runInterruptCode(value);
			}
			break;
			case Mnemonic::Raise_I:
			{
				auto value = _tryRead(Indirect{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_runInterruptCode(value);
			}
			break;
			case Mnemonic::Raise_V:
			{
				auto value = _tryRead(Value{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_runInterruptCode(value);
			}
			break;
			case Mnemonic::DisableInt_R:
			{
				auto value = _tryRead(Register{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptEnabled(value, false);
			}
			break;
			case Mnemonic::DisableInt_V:
			{
				auto value = _tryRead(Value{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptEnabled(value, false);
			}
			break;
			case Mnemonic::EnableInt_R:
			{
				auto value = _tryRead(Register{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptEnabled(value, true);
			}
			break;
			case Mnemonic::EnableInt_V:
			{
				auto value = _tryRead(Value{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptEnabled(value, true);
			}
			break;
			case Mnemonic::ICountInt_R:
				nextInstrCountInterrupt = _tryRead(Register{ instr->arg1 });
				break;
			case Mnemonic::ICountInt_V:
				nextInstrCountInterrupt = _tryRead(Value{ instr->arg1 });
				break;
			case Mnemonic::RegInt_R_R:
			{
				auto value = _tryRead(Register{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Register{ instr->arg2 }));
			}
			break;
			case Mnemonic::Regint_R_A:
			{
				auto value = _tryRead(Register{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Address{ instr->arg2 }));
			}
			break;
			case Mnemonic::Regint_R_I:
			{
				auto value = _tryRead(Register{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Indirect{ instr->arg2 }));
			}
			break;
			case Mnemonic::RegInt_A_R:
			{
				auto value = _tryRead(Address{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Register{ instr->arg2 }));
			}
			break;
			case Mnemonic::Regint_A_A:
			{
				auto value = _tryRead(Address{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Address{ instr->arg2 }));
			}
			break;
			case Mnemonic::Regint_A_I:
			{
				auto value = _tryRead(Address{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Indirect{ instr->arg2 }));
			}
			break;
			case Mnemonic::RegInt_I_R:
			{
				auto value = _tryRead(Indirect{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Register{ instr->arg2 }));
			}
			break;
			case Mnemonic::Regint_I_A:
			{
				auto value = _tryRead(Indirect{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Address{ instr->arg2 }));
			}
			break;
			case Mnemonic::Regint_I_I:
			{
				auto value = _tryRead(Indirect{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Indirect{ instr->arg2 }));
			}
			break;
			case Mnemonic::RegInt_V_R:
			{
				auto value = _tryRead(Value{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Register{ instr->arg2 }));
			}
			break;
			case Mnemonic::Regint_V_A:
			{
				auto value = _tryRead(Value{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Address{ instr->arg2 }));
			}
			break;
			case Mnemonic::Regint_V_I:
			{
				auto value = _tryRead(Value{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Indirect{ instr->arg2 }));
			}
			break;
			case Mnemonic::RRegInt_R_R:
			{
				auto value = _tryRead(Register{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Register{ instr->arg2 }) + instrPtr);
			}
			break;
			case Mnemonic::RRegInt_R_A:
			{
				auto value = _tryRead(Register{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Address{ instr->arg2 }) + instrPtr);
			}
			break;
			case Mnemonic::RRegInt_R_I:
			{
				auto value = _tryRead(Register{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Indirect{ instr->arg2 }) + instrPtr);
			}
			break;
			case Mnemonic::RRegInt_A_R:
			{
				auto value = _tryRead(Address{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Register{ instr->arg2 }) + instrPtr);
			}
			break;
			case Mnemonic::RRegInt_A_A:
			{
				auto value = _tryRead(Address{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Address{ instr->arg2 }) + instrPtr);
			}
			break;
			case Mnemonic::RRegInt_A_I:
			{
				auto value = _tryRead(Address{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Indirect{ instr->arg2 }) + instrPtr);
			}
			break;
			case Mnemonic::RRegInt_I_R:
			{
				auto value = _tryRead(Indirect{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Register{ instr->arg2 }) + instrPtr);
			}
			break;
			case Mnemonic::RRegInt_I_A:
			{
				auto value = _tryRead(Indirect{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Address{ instr->arg2 }) + instrPtr);
			}
			break;
			case Mnemonic::RRegInt_I_I:
			{
				auto value = _tryRead(Indirect{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Indirect{ instr->arg2 }) + instrPtr);
			}
			break;
			case Mnemonic::RRegInt_V_R:
			{
				auto value = _tryRead(Value{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Register{ instr->arg2 }) + instrPtr);
			}
			break;
			case Mnemonic::RRegInt_V_A:
			{
				auto value = _tryRead(Value{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Address{ instr->arg2 }) + instrPtr);
			}
			break;
			case Mnemonic::RRegInt_V_I:
			{
				auto value = _tryRead(Value{ instr->arg1 });
				if (!_testPrivilege(interrupts[static_cast<uint8_t>(value)].privilegeRequired, instr, ErrorCode::UnprivilegedIntRaise))
					return false;
				_setInterruptHandler(value, _tryReadDeref(Indirect{ instr->arg2 }) + instrPtr);
			}
			break;
			case Mnemonic::IRet:

				break;
			case Mnemonic::RICountInt_R:
				nextInstrCountInterrupt = static_cast<uint32_t>(instrPtr) + _tryRead(Register{ instr->arg1 });
				break;
			case Mnemonic::RICountInt_V:
				nextInstrCountInterrupt = static_cast<uint32_t>(instrPtr) + _tryRead(Value{ instr->arg1 });
				break;
			case Mnemonic::ICountInt64_R:
			{
				auto& v1 = _tryRead(Register{ instr->arg1 });
				auto& v2 = _tryRead(Register{ instr->arg1 + 1 });
				_setNextInstrCountInt(v1, v2);
			}
			break;
			case Mnemonic::ICountInt64_A:
			{
				auto& v1 = _tryRead(Address{ instr->arg1 });
				auto& v2 = _tryRead(Address{ instr->arg1 + 1 });
				_setNextInstrCountInt(v1, v2);
			}
			break;
			case Mnemonic::ICountInt64_I:
			{
				auto& v1 = _tryRead(Indirect{ instr->arg1 });
				auto& v2 = _tryRead(Indirect{ instr->arg1 + 1 });
				_setNextInstrCountInt(v1, v2);
			}
			break;
			case Mnemonic::RICountInt64_R:
			{
				auto& v1 = _tryRead(Register{ instr->arg1 });
				auto& v2 = _tryRead(Register{ instr->arg1 + 1 });
				_setNextInstrCountInt(instrCount, v1, v2);
			}
			break;
			case Mnemonic::RICountInt64_A:
			{
				auto& v1 = _tryRead(Address{ instr->arg1 });
				auto& v2 = _tryRead(Address{ instr->arg1 + 1 });
				_setNextInstrCountInt(instrCount, v1, v2);
			}
			break;
			case Mnemonic::RICountInt64_I:
			{
				auto& v1 = _tryRead(Indirect{ instr->arg1 });
				auto& v2 = _tryRead(Indirect{ instr->arg1 + 1 });
				_setNextInstrCountInt(instrCount, v1, v2);
			}
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
		}

		return true;
	}

}	//sbl::vm