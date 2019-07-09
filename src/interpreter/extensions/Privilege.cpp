
#include "../VM.hpp"
#include "../../common/Instruction.hpp"

namespace sbl::vm {
	bool VM::_performPrivilege(Instruction* instr) {
		switch (instr->mnemonic) {
			case Mnemonic::GetPrivlg_R:
				_tryWrite(Register{ instr->arg1 }, privilegeLevel);
				break;
			case Mnemonic::GetPrivlg_A:
				_tryWrite(Address{ instr->arg1 }, privilegeLevel);
				break;
			case Mnemonic::GetPrivlg_I:
				_tryWrite(Indirect{ instr->arg1 }, privilegeLevel);
				break;
			case Mnemonic::SetPrivlg_R:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				if (!_testPrivilege(v1, instr)) return false;
				privilegeLevel = v1;
			}
			break;
			case Mnemonic::SetPrivlg_A:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				if (!_testPrivilege(v1, instr)) return false;
				privilegeLevel = v1;
			}
			break;
			case Mnemonic::SetPrivlg_I:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				if (!_testPrivilege(v1, instr)) return false;
				privilegeLevel = v1;
			}
			break;
			case Mnemonic::SetPrivlg_V:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				if (!_testPrivilege(v1, instr)) return false;
				privilegeLevel = v1;
			}
			break;
			case Mnemonic::SetInstrPrivlg_R_R:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_R_A:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_R_I:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_R_V:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_A_R:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_A_A:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_A_I:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_A_V:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_I_R:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_I_A:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_I_I:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_I_V:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_V_R:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_V_A:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_V_I:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::SetInstrPrivlg_V_V:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				instrPrivileges[v1] = v2;
			}
			break;
			case Mnemonic::GetInstrPrivlg_R_R:
			{
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v2 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, instrPrivileges[v2]);
			}
			break;
			case Mnemonic::GetInstrPrivlg_R_A:
			{
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v2 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, instrPrivileges[v2]);
			}
			break;
			case Mnemonic::GetInstrPrivlg_R_I:
			{
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v2 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, instrPrivileges[v2]);
			}
			break;
			case Mnemonic::GetInstrPrivlg_R_V:
			{
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v2 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, instrPrivileges[v2]);
			}
			break;
			case Mnemonic::GetInstrPrivlg_A_R:
			{
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v2 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, instrPrivileges[v2]);
			}
			break;
			case Mnemonic::GetInstrPrivlg_A_A:
			{
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v2 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, instrPrivileges[v2]);
			}
			break;
			case Mnemonic::GetInstrPrivlg_A_I:
			{
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v2 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, instrPrivileges[v2]);
			}
			break;
			case Mnemonic::GetInstrPrivlg_A_V:
			{
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v2 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, instrPrivileges[v2]);
			}
			break;
			case Mnemonic::GetInstrPrivlg_I_R:
			{
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v2 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, instrPrivileges[v2]);
			}
			break;
			case Mnemonic::GetInstrPrivlg_I_A:
			{
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v2 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, instrPrivileges[v2]);
			}
			break;
			case Mnemonic::GetInstrPrivlg_I_I:
			{
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v2 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, instrPrivileges[v2]);
			}
			break;
			case Mnemonic::GetInstrPrivlg_I_V:
			{
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v2 >= instrPrivileges.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, instrPrivileges[v2]);
			}
			break;
			case Mnemonic::SetIntPrivlg_R_R:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_R_A:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_R_I:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_R_V:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_A_R:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_A_A:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_A_I:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_A_V:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_I_R:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_I_A:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_I_I:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_I_V:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_V_R:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_V_A:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_V_I:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetIntPrivlg_V_V:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilege = v2;
			}
			break;
			case Mnemonic::GetIntPrivlg_R_R:
			{
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, interrupts[v2].privilege);
			}
			break;
			case Mnemonic::GetIntPrivlg_R_A:
			{
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, interrupts[v2].privilege);
			}
			break;
			case Mnemonic::GetIntPrivlg_R_I:
			{
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, interrupts[v2].privilege);
			}
			break;
			case Mnemonic::GetIntPrivlg_R_V:
			{
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, interrupts[v2].privilege);
			}
			break;
			case Mnemonic::GetIntPrivlg_A_R:
			{
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, interrupts[v2].privilege);
			}
			break;
			case Mnemonic::GetIntPrivlg_A_A:
			{
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, interrupts[v2].privilege);
			}
			break;
			case Mnemonic::GetIntPrivlg_A_I:
			{
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, interrupts[v2].privilege);
			}
			break;
			case Mnemonic::GetIntPrivlg_A_V:
			{
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, interrupts[v2].privilege);
			}
			break;
			case Mnemonic::GetIntPrivlg_I_R:
			{
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, interrupts[v2].privilege);
			}
			break;
			case Mnemonic::GetIntPrivlg_I_A:
			{
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, interrupts[v2].privilege);
			}
			break;
			case Mnemonic::GetIntPrivlg_I_I:
			{
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, interrupts[v2].privilege);
			}
			break;
			case Mnemonic::GetIntPrivlg_I_V:
			{
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, interrupts[v2].privilege);
			}
			break;
			case Mnemonic::SetIntExecPrivlg_R_R:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_R_A:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_R_I:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_R_V:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_A_R:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_A_A:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_A_I:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_A_V:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_I_R:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_I_A:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_I_I:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_I_V:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_V_R:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_V_A:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_V_I:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::SetIntExecPrivlg_V_V:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				interrupts[v1].privilegeRequired = v2;
			}
			break;
			case Mnemonic::GetIntExecPrivlg_R_R:
			{
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetIntExecPrivlg_R_A:
			{
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetIntExecPrivlg_R_I:
			{
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetIntExecPrivlg_R_V:
			{
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetIntExecPrivlg_A_R:
			{
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetIntExecPrivlg_A_A:
			{
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetIntExecPrivlg_A_I:
			{
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetIntExecPrivlg_A_V:
			{
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetIntExecPrivlg_I_R:
			{
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetIntExecPrivlg_I_A:
			{
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetIntExecPrivlg_I_I:
			{
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetIntExecPrivlg_I_V:
			{
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v2 > interrupts.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::SetExtPrivlg_R_R:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_R_A:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_R_I:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_R_V:
			{
				auto v1 = _tryRead(Register{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_A_R:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_A_A:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_A_I:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_A_V:
			{
				auto v1 = _tryRead(Address{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_I_R:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_I_A:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_I_I:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_I_V:
			{
				auto v1 = _tryRead(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_V_R:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_V_A:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_V_I:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::SetExtPrivlg_V_V:
			{
				auto v1 = _tryRead(Value{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v1 > interrupts.size()) {
					error = { ErrorCode::InvalidExtensionId, instrPtr };
					running = false;
					return false;
				}
				else if (v2 > 255) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}

				if (!_testPrivilege(v2, instr)) return false;
				extensionData[v1].privilege = v2;
			}
			break;
			case Mnemonic::GetExtPrivlg_R_R:
			{
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v2 > extensionData.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetExtPrivlg_R_A:
			{
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v2 > extensionData.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetExtPrivlg_R_I:
			{
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v2 > extensionData.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetExtPrivlg_R_V:
			{
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v2 > extensionData.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Register{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetExtPrivlg_A_R:
			{
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v2 > extensionData.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetExtPrivlg_A_A:
			{
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v2 > extensionData.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetExtPrivlg_A_I:
			{
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v2 > extensionData.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetExtPrivlg_A_V:
			{
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v2 > extensionData.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Address{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetExtPrivlg_I_R:
			{
				auto v2 = _tryRead(Register{ instr->arg2 });
				if (v2 > extensionData.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetExtPrivlg_I_A:
			{
				auto v2 = _tryRead(Address{ instr->arg2 });
				if (v2 > extensionData.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetExtPrivlg_I_I:
			{
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				if (v2 > extensionData.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::GetExtPrivlg_I_V:
			{
				auto v2 = _tryRead(Value{ instr->arg2 });
				if (v2 > extensionData.size()) {
					error = { ErrorCode::InvalidPrivilegeId, instrPtr };
					running = false;
					return false;
				}
				_tryWrite(Indirect{ instr->arg1 }, interrupts[v2].privilegeRequired);
			}
			break;
			case Mnemonic::PCall_R_R:
			{
				auto v1 = _forceReadDeref(Register{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				return _executeFunc(v1, v2, instr);
			}
			break;
			case Mnemonic::PCall_R_A:
			{
				auto v1 = _forceReadDeref(Register{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				return _executeFunc(v1, v2, instr);
			}
			break;
			case Mnemonic::PCall_R_I:
			{
				auto v1 = _forceReadDeref(Register{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				return _executeFunc(v1, v2, instr);
			}
			break;
			case Mnemonic::PCall_R_V:
			{
				auto v1 = _forceReadDeref(Register{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				return _executeFunc(v1, v2, instr);
			}
			break;
			case Mnemonic::PCall_A_R:
			{
				auto v1 = _forceReadDeref(Address{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				return _executeFunc(v1, v2, instr);
			}
			break;
			case Mnemonic::PCall_A_A:
			{
				auto v1 = _forceReadDeref(Address{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				return _executeFunc(v1, v2, instr);
			}
			break;
			case Mnemonic::PCall_A_I:
			{
				auto v1 = _forceReadDeref(Address{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				return _executeFunc(v1, v2, instr);
			}
			break;
			case Mnemonic::PCall_A_V:
			{
				auto v1 = _forceReadDeref(Address{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				return _executeFunc(v1, v2, instr);
			}
			break;
			case Mnemonic::PCall_I_R:
			{
				auto v1 = _forceReadDeref(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				return _executeFunc(v1, v2, instr);
			}
			break;
			case Mnemonic::PCall_I_A:
			{
				auto v1 = _forceReadDeref(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				return _executeFunc(v1, v2, instr);
			}
			break;
			case Mnemonic::PCall_I_I:
			{
				auto v1 = _forceReadDeref(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				return _executeFunc(v1, v2, instr);
			}
			break;
			case Mnemonic::PCall_I_V:
			{
				auto v1 = _forceReadDeref(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				return _executeFunc(v1, v2, instr);
			}
			break;
			case Mnemonic::RPCall_R_R:
			{
				auto v1 = _forceReadDeref(Register{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				return _executeFunc(static_cast<uint32_t>(instrPtr) + v1, v2, instr);
			}
			break;
			case Mnemonic::RPCall_R_A:
			{
				auto v1 = _forceReadDeref(Register{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				return _executeFunc(static_cast<uint32_t>(instrPtr) + v1, v2, instr);
			}
			break;
			case Mnemonic::RPCall_R_I:
			{
				auto v1 = _forceReadDeref(Register{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				return _executeFunc(static_cast<uint32_t>(instrPtr) + v1, v2, instr);
			}
			break;
			case Mnemonic::RPCall_R_V:
			{
				auto v1 = _forceReadDeref(Register{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				return _executeFunc(static_cast<uint32_t>(instrPtr) + v1, v2, instr);
			}
			break;
			case Mnemonic::RPCall_A_R:
			{
				auto v1 = _forceReadDeref(Address{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				return _executeFunc(static_cast<uint32_t>(instrPtr) + v1, v2, instr);
			}
			break;
			case Mnemonic::RPCall_A_A:
			{
				auto v1 = _forceReadDeref(Address{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				return _executeFunc(static_cast<uint32_t>(instrPtr) + v1, v2, instr);
			}
			break;
			case Mnemonic::RPCall_A_I:
			{
				auto v1 = _forceReadDeref(Address{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				return _executeFunc(static_cast<uint32_t>(instrPtr) + v1, v2, instr);
			}
			break;
			case Mnemonic::RPCall_A_V:
			{
				auto v1 = _forceReadDeref(Address{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				return _executeFunc(static_cast<uint32_t>(instrPtr) + v1, v2, instr);
			}
			break;
			case Mnemonic::RPCall_I_R:
			{
				auto v1 = _forceReadDeref(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Register{ instr->arg2 });
				return _executeFunc(static_cast<uint32_t>(instrPtr) + v1, v2, instr);
			}
			break;
			case Mnemonic::RPCall_I_A:
			{
				auto v1 = _forceReadDeref(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Address{ instr->arg2 });
				return _executeFunc(static_cast<uint32_t>(instrPtr) + v1, v2, instr);
			}
			break;
			case Mnemonic::RPCall_I_I:
			{
				auto v1 = _forceReadDeref(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Indirect{ instr->arg2 });
				return _executeFunc(static_cast<uint32_t>(instrPtr) + v1, v2, instr);
			}
			break;
			case Mnemonic::RPCall_I_V:
			{
				auto v1 = _forceReadDeref(Indirect{ instr->arg1 });
				auto v2 = _tryRead(Value{ instr->arg2 });
				return _executeFunc(static_cast<uint32_t>(instrPtr) + v1, v2, instr);
			}
			break;
			case Mnemonic::PNtvCall_R_R:
				_doNativeCall(_tryRead(Register{ instr->arg1 }), _tryRead(Register{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_R_A:
				_doNativeCall(_tryRead(Register{ instr->arg1 }), _tryRead(Address{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_R_I:
				_doNativeCall(_tryRead(Register{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_R_V:
				_doNativeCall(_tryRead(Register{ instr->arg1 }), _tryRead(Value{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_A_R:
				_doNativeCall(_tryRead(Address{ instr->arg1 }), _tryRead(Register{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_A_A:
				_doNativeCall(_tryRead(Address{ instr->arg1 }), _tryRead(Address{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_A_I:
				_doNativeCall(_tryRead(Address{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_A_V:
				_doNativeCall(_tryRead(Address{ instr->arg1 }), _tryRead(Value{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_I_R:
				_doNativeCall(_tryRead(Indirect{ instr->arg1 }), _tryRead(Register{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_I_A:
				_doNativeCall(_tryRead(Indirect{ instr->arg1 }), _tryRead(Address{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_I_I:
				_doNativeCall(_tryRead(Indirect{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_I_V:
				_doNativeCall(_tryRead(Indirect{ instr->arg1 }), _tryRead(Value{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_V_R:
				_doNativeCall(_tryRead(Value{ instr->arg1 }), _tryRead(Register{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_V_A:
				_doNativeCall(_tryRead(Value{ instr->arg1 }), _tryRead(Address{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_V_I:
				_doNativeCall(_tryRead(Value{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }), instr);
				break;
			case Mnemonic::PNtvCall_V_V:
				_doNativeCall(_tryRead(Value{ instr->arg1 }), _tryRead(Value{ instr->arg2 }), instr);
				break;
		}

		return true;
	}

}	//sbl::vm