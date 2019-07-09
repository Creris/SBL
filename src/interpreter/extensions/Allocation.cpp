
#include "../VM.hpp"
#include "../../common/Instruction.hpp"

namespace sbl::vm {
	bool VM::_performAllocation(Instruction* instr) {
		switch (instr->mnemonic) {
			case Mnemonic::Alloc_R_R:
				_tryWrite(Register{ instr->arg1 }, dynamicHandler.allocateNew(this, _tryRead(Register{ instr->arg2 })));
				break;
			case Mnemonic::Alloc_R_A:
				_tryWrite(Register{ instr->arg1 }, dynamicHandler.allocateNew(this, _tryRead(Address{ instr->arg2 })));
				break;
			case Mnemonic::Alloc_R_I:
				_tryWrite(Register{ instr->arg1 }, dynamicHandler.allocateNew(this, _tryRead(Indirect{ instr->arg2 })));
				break;
			case Mnemonic::Alloc_R_V:
				_tryWrite(Register{ instr->arg1 }, dynamicHandler.allocateNew(this, _tryRead(Value{ instr->arg2 })));
				break;
			case Mnemonic::Alloc_A_R:
				_tryWrite(Address{ instr->arg1 }, dynamicHandler.allocateNew(this, _tryRead(Register{ instr->arg2 })));
				break;
			case Mnemonic::Alloc_A_A:
				_tryWrite(Address{ instr->arg1 }, dynamicHandler.allocateNew(this, _tryRead(Address{ instr->arg2 })));
				break;
			case Mnemonic::Alloc_A_I:
				_tryWrite(Address{ instr->arg1 }, dynamicHandler.allocateNew(this, _tryRead(Indirect{ instr->arg2 })));
				break;
			case Mnemonic::Alloc_A_V:
				_tryWrite(Address{ instr->arg1 }, dynamicHandler.allocateNew(this, _tryRead(Value{ instr->arg2 })));
				break;
			case Mnemonic::Alloc_I_R:
				_tryWrite(Indirect{ instr->arg1 }, dynamicHandler.allocateNew(this, _tryRead(Register{ instr->arg2 })));
				break;
			case Mnemonic::Alloc_I_A:
				_tryWrite(Indirect{ instr->arg1 }, dynamicHandler.allocateNew(this, _tryRead(Address{ instr->arg2 })));
				break;
			case Mnemonic::Alloc_I_I:
				_tryWrite(Indirect{ instr->arg1 }, dynamicHandler.allocateNew(this, _tryRead(Indirect{ instr->arg2 })));
				break;
			case Mnemonic::Alloc_I_V:
				_tryWrite(Indirect{ instr->arg1 }, dynamicHandler.allocateNew(this, _tryRead(Value{ instr->arg2 })));
				break;
			case Mnemonic::Dealloc_R:
				dynamicHandler.deallocate(this, _tryRead(Register{ instr->arg1 }));
				break;
			case Mnemonic::Dealloc_A:
				dynamicHandler.deallocate(this, _tryRead(Address{ instr->arg1 }));
				break;
			case Mnemonic::Dealloc_I:
				dynamicHandler.deallocate(this, _tryRead(Indirect{ instr->arg1 }));
				break;
			case Mnemonic::DynOffset_R:
				dynamicOffset = _tryRead(Register{ instr->arg1 });
				break;
			case Mnemonic::DynOffset_A:
				dynamicOffset = _tryRead(Address{ instr->arg1 });
				break;
			case Mnemonic::DynOffset_I:
				dynamicOffset = _tryRead(Indirect{ instr->arg1 });
				break;
			case Mnemonic::DynOffset_V:
				dynamicOffset = _tryRead(Value{ instr->arg1 });
				break;
			case Mnemonic::ClrDynOffset:
				dynamicOffset = 0;
				break;
			case Mnemonic::LoadDyn_R_R:
				_tryWrite(Register{ instr->arg1 }, _tryReadDynamic(_tryRead(Register{ instr->arg2 }), dynamicOffset));
				break;
			case Mnemonic::LoadDyn_R_A:
				_tryWrite(Register{ instr->arg1 }, _tryReadDynamic(_tryRead(Address{ instr->arg2 }), dynamicOffset));
				break;
			case Mnemonic::LoadDyn_R_I:
				_tryWrite(Register{ instr->arg1 }, _tryReadDynamic(_tryRead(Indirect{ instr->arg2 }), dynamicOffset));
				break;
			case Mnemonic::LoadDyn_R_V:
				_tryWrite(Register{ instr->arg1 }, _tryReadDynamic(_tryRead(Value{ instr->arg2 }), dynamicOffset));
				break;
			case Mnemonic::LoadDyn_A_R:
				_tryWrite(Address{ instr->arg1 }, _tryReadDynamic(_tryRead(Register{ instr->arg2 }), dynamicOffset));
				break;
			case Mnemonic::LoadDyn_A_A:
				_tryWrite(Address{ instr->arg1 }, _tryReadDynamic(_tryRead(Address{ instr->arg2 }), dynamicOffset));
				break;
			case Mnemonic::LoadDyn_A_I:
				_tryWrite(Address{ instr->arg1 }, _tryReadDynamic(_tryRead(Indirect{ instr->arg2 }), dynamicOffset));
				break;
			case Mnemonic::LoadDyn_A_V:
				_tryWrite(Address{ instr->arg1 }, _tryReadDynamic(_tryRead(Value{ instr->arg2 }), dynamicOffset));
				break;
			case Mnemonic::LoadDyn_I_R:
				_tryWrite(Indirect{ instr->arg1 }, _tryReadDynamic(_tryRead(Register{ instr->arg2 }), dynamicOffset));
				break;
			case Mnemonic::LoadDyn_I_A:
				_tryWrite(Indirect{ instr->arg1 }, _tryReadDynamic(_tryRead(Address{ instr->arg2 }), dynamicOffset));
				break;
			case Mnemonic::LoadDyn_I_I:
				_tryWrite(Indirect{ instr->arg1 }, _tryReadDynamic(_tryRead(Indirect{ instr->arg2 }), dynamicOffset));
				break;
			case Mnemonic::LoadDyn_I_V:
				_tryWrite(Indirect{ instr->arg1 }, _tryReadDynamic(_tryRead(Value{ instr->arg2 }), dynamicOffset));
				break;
			case Mnemonic::WriteDyn_R_R:
				_tryWriteDynamic(_tryRead(Register{ instr->arg1 }), dynamicOffset, _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_R_A:
				_tryWriteDynamic(_tryRead(Register{ instr->arg1 }), dynamicOffset, _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_R_I:
				_tryWriteDynamic(_tryRead(Register{ instr->arg1 }), dynamicOffset, _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_R_V:
				_tryWriteDynamic(_tryRead(Register{ instr->arg1 }), dynamicOffset, _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_A_R:
				_tryWriteDynamic(_tryRead(Address{ instr->arg1 }), dynamicOffset, _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_A_A:
				_tryWriteDynamic(_tryRead(Address{ instr->arg1 }), dynamicOffset, _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_A_I:
				_tryWriteDynamic(_tryRead(Address{ instr->arg1 }), dynamicOffset, _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_A_V:
				_tryWriteDynamic(_tryRead(Address{ instr->arg1 }), dynamicOffset, _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_I_R:
				_tryWriteDynamic(_tryRead(Indirect{ instr->arg1 }), dynamicOffset, _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_I_A:
				_tryWriteDynamic(_tryRead(Indirect{ instr->arg1 }), dynamicOffset, _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_I_I:
				_tryWriteDynamic(_tryRead(Indirect{ instr->arg1 }), dynamicOffset, _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_I_V:
				_tryWriteDynamic(_tryRead(Indirect{ instr->arg1 }), dynamicOffset, _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_V_R:
				_tryWriteDynamic(_tryRead(Value{ instr->arg1 }), dynamicOffset, _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_V_A:
				_tryWriteDynamic(_tryRead(Value{ instr->arg1 }), dynamicOffset, _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_V_I:
				_tryWriteDynamic(_tryRead(Value{ instr->arg1 }), dynamicOffset, _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::WriteDyn_V_V:
				_tryWriteDynamic(_tryRead(Value{ instr->arg1 }), dynamicOffset, _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::GetDynSize_R_R:
				_tryWrite(Register{ instr->arg1 }, _getDynSize(_tryRead(Register{ instr->arg2 })));
				break;
			case Mnemonic::GetDynSize_R_A:
				_tryWrite(Register{ instr->arg1 }, _getDynSize(_tryRead(Address{ instr->arg2 })));
				break;
			case Mnemonic::GetDynSize_R_I:
				_tryWrite(Register{ instr->arg1 }, _getDynSize(_tryRead(Indirect{ instr->arg2 })));
				break;
			case Mnemonic::GetDynSize_R_V:
				_tryWrite(Register{ instr->arg1 }, _getDynSize(_tryRead(Value{ instr->arg2 })));
				break;
			case Mnemonic::GetDynSize_A_R:
				_tryWrite(Address{ instr->arg1 }, _getDynSize(_tryRead(Register{ instr->arg2 })));
				break;
			case Mnemonic::GetDynSize_A_A:
				_tryWrite(Address{ instr->arg1 }, _getDynSize(_tryRead(Address{ instr->arg2 })));
				break;
			case Mnemonic::GetDynSize_A_I:
				_tryWrite(Address{ instr->arg1 }, _getDynSize(_tryRead(Indirect{ instr->arg2 })));
				break;
			case Mnemonic::GetDynSize_A_V:
				_tryWrite(Address{ instr->arg1 }, _getDynSize(_tryRead(Value{ instr->arg2 })));
				break;
			case Mnemonic::GetDynSize_I_R:
				_tryWrite(Indirect{ instr->arg1 }, _getDynSize(_tryRead(Register{ instr->arg2 })));
				break;
			case Mnemonic::GetDynSize_I_A:
				_tryWrite(Indirect{ instr->arg1 }, _getDynSize(_tryRead(Address{ instr->arg2 })));
				break;
			case Mnemonic::GetDynSize_I_I:
				_tryWrite(Indirect{ instr->arg1 }, _getDynSize(_tryRead(Indirect{ instr->arg2 })));
				break;
			case Mnemonic::GetDynSize_I_V:
				_tryWrite(Indirect{ instr->arg1 }, _getDynSize(_tryRead(Value{ instr->arg2 })));
				break;
		}

		return true;
	}

}	//sbl::vm