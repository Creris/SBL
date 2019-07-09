
#include "../VM.hpp"
#include "../../common/Instruction.hpp"

namespace sbl::vm {
	bool VM::_performArithmetic(Instruction* instr) {
		switch (instr->mnemonic) {
			case Mnemonic::Add_R_R:
				_tryAdd(Register{ instr->arg1 }, _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Add_R_I:
				_tryAdd(Register{ instr->arg1 }, _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Add_R_A:
				_tryAdd(Register{ instr->arg1 }, _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Add_R_V:
				_tryAdd(Register{ instr->arg1 }, _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Add_I_R:
				_tryAdd(Indirect{ instr->arg1 }, _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Add_I_I:
				_tryAdd(Indirect{ instr->arg1 }, _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Add_I_A:
				_tryAdd(Indirect{ instr->arg1 }, _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Add_I_V:
				_tryAdd(Indirect{ instr->arg1 }, _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Add_A_R:
				_tryAdd(Address{ instr->arg1 }, _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Add_A_I:
				_tryAdd(Address{ instr->arg1 }, _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Add_A_A:
				_tryAdd(Address{ instr->arg1 }, _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Add_A_V:
				_tryAdd(Address{ instr->arg1 }, _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Sub_R_R:
				_tryAdd(Register{ instr->arg1 }, ~_tryRead(Register{ instr->arg2 }) + 1);
				break;
			case Mnemonic::Sub_R_A:
				_tryAdd(Register{ instr->arg1 }, ~_tryRead(Address{ instr->arg2 }) + 1);
				break;
			case Mnemonic::Sub_R_I:
				_tryAdd(Register{ instr->arg1 }, ~_tryRead(Indirect{ instr->arg2 }) + 1);
				break;
			case Mnemonic::Sub_R_V:
				_tryAdd(Register{ instr->arg1 }, ~_tryRead(Value{ instr->arg2 }) + 1);
				break;
			case Mnemonic::Sub_A_R:
				_tryAdd(Address{ instr->arg1 }, ~_tryRead(Register{ instr->arg2 }) + 1);
				break;
			case Mnemonic::Sub_A_A:
				_tryAdd(Address{ instr->arg1 }, ~_tryRead(Address{ instr->arg2 }) + 1);
				break;
			case Mnemonic::Sub_A_I:
				_tryAdd(Address{ instr->arg1 }, ~_tryRead(Indirect{ instr->arg2 }) + 1);
				break;
			case Mnemonic::Sub_A_V:
				_tryAdd(Address{ instr->arg1 }, ~_tryRead(Value{ instr->arg2 }) + 1);
				break;
			case Mnemonic::Sub_I_R:
				_tryAdd(Indirect{ instr->arg1 }, ~_tryRead(Register{ instr->arg2 }) + 1);
				break;
			case Mnemonic::Sub_I_A:
				_tryAdd(Indirect{ instr->arg1 }, ~_tryRead(Address{ instr->arg2 }) + 1);
				break;
			case Mnemonic::Sub_I_I:
				_tryAdd(Indirect{ instr->arg1 }, ~_tryRead(Indirect{ instr->arg2 }) + 1);
				break;
			case Mnemonic::Sub_I_V:
				_tryAdd(Indirect{ instr->arg1 }, ~_tryRead(Value{ instr->arg2 }) + 1);
				break;
			case Mnemonic::Mul_R_R:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) * _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Mul_R_A:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) * _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Mul_R_I:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) * _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Mul_R_V:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) * _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Mul_A_R:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) * _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Mul_A_A:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) * _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Mul_A_I:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) * _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Mul_A_V:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) * _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Mul_I_R:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) * _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Mul_I_A:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) * _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Mul_I_I:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) * _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Mul_I_V:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) * _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Div_R_R:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) / _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Div_R_A:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) / _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Div_R_I:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) / _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Div_R_V:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) / _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Div_A_R:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) / _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Div_A_A:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) / _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Div_A_I:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) / _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Div_A_V:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) / _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Div_I_R:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) / _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Div_I_A:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) / _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Div_I_I:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) / _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Div_I_V:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) / _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Mod_R_R:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) % _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Mod_R_A:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) % _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Mod_R_I:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) % _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Mod_R_V:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) % _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Mod_A_R:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) % _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Mod_A_A:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) % _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Mod_A_I:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) % _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Mod_A_V:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) % _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Mod_I_R:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) % _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Mod_I_A:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) % _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Mod_I_I:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) % _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Mod_I_V:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) % _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Lsh_R_R:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) << _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Lsh_R_A:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) << _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Lsh_R_I:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) << _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Lsh_R_V:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) << _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Lsh_A_R:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) << _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Lsh_A_A:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) << _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Lsh_A_I:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) << _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Lsh_A_V:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) << _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Lsh_I_R:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) << _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Lsh_I_A:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) << _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Lsh_I_I:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) << _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Lsh_I_V:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) << _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Rlsh_R_R:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _rotl(_forceRead(Register{ instr->arg1 }), _tryRead(Register{ instr->arg2 })));
				break;
			case Mnemonic::Rlsh_R_A:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _rotl(_forceRead(Register{ instr->arg1 }), _tryRead(Address{ instr->arg2 })));
				break;
			case Mnemonic::Rlsh_R_I:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _rotl(_forceRead(Register{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 })));
				break;
			case Mnemonic::Rlsh_R_V:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _rotl(_forceRead(Register{ instr->arg1 }), _tryRead(Value{ instr->arg2 })));
				break;
			case Mnemonic::Rlsh_A_R:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _rotl(_forceRead(Address{ instr->arg1 }), _tryRead(Register{ instr->arg2 })));
				break;
			case Mnemonic::Rlsh_A_A:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _rotl(_forceRead(Address{ instr->arg1 }), _tryRead(Address{ instr->arg2 })));
				break;
			case Mnemonic::Rlsh_A_I:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _rotl(_forceRead(Address{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 })));
				break;
			case Mnemonic::Rlsh_A_V:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _rotl(_forceRead(Address{ instr->arg1 }), _tryRead(Value{ instr->arg2 })));
				break;
			case Mnemonic::Rlsh_I_R:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _rotl(_forceRead(Indirect{ instr->arg1 }), _tryRead(Register{ instr->arg2 })));
				break;
			case Mnemonic::Rlsh_I_A:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _rotl(_forceRead(Indirect{ instr->arg1 }), _tryRead(Address{ instr->arg2 })));
				break;
			case Mnemonic::Rlsh_I_I:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _rotl(_forceRead(Indirect{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 })));
				break;
			case Mnemonic::Rlsh_I_V:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _rotl(_forceRead(Indirect{ instr->arg1 }), _tryRead(Value{ instr->arg2 })));
				break;
			case Mnemonic::Rsh_R_R:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) >> _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Rsh_R_A:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) >> _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Rsh_R_I:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) >> _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Rsh_R_V:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) >> _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Rsh_A_R:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) >> _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Rsh_A_A:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) >> _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Rsh_A_I:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) >> _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Rsh_A_V:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) >> _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Rsh_I_R:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) >> _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Rsh_I_A:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) >> _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Rsh_I_I:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) >> _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Rsh_I_V:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) >> _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Rrsh_R_R:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _rotr(_forceRead(Register{ instr->arg1 }), _tryRead(Register{ instr->arg2 })));
				break;
			case Mnemonic::Rrsh_R_A:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _rotr(_forceRead(Register{ instr->arg1 }), _tryRead(Address{ instr->arg2 })));
				break;
			case Mnemonic::Rrsh_R_I:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _rotr(_forceRead(Register{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 })));
				break;
			case Mnemonic::Rrsh_R_V:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _rotr(_forceRead(Register{ instr->arg1 }), _tryRead(Value{ instr->arg2 })));
				break;
			case Mnemonic::Rrsh_A_R:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _rotr(_forceRead(Address{ instr->arg1 }), _tryRead(Register{ instr->arg2 })));
				break;
			case Mnemonic::Rrsh_A_A:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _rotr(_forceRead(Address{ instr->arg1 }), _tryRead(Address{ instr->arg2 })));
				break;
			case Mnemonic::Rrsh_A_I:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _rotr(_forceRead(Address{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 })));
				break;
			case Mnemonic::Rrsh_A_V:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _rotr(_forceRead(Address{ instr->arg1 }), _tryRead(Value{ instr->arg2 })));
				break;
			case Mnemonic::Rrsh_I_R:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _rotr(_forceRead(Indirect{ instr->arg1 }), _tryRead(Register{ instr->arg2 })));
				break;
			case Mnemonic::Rrsh_I_A:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _rotr(_forceRead(Indirect{ instr->arg1 }), _tryRead(Address{ instr->arg2 })));
				break;
			case Mnemonic::Rrsh_I_I:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _rotr(_forceRead(Indirect{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 })));
				break;
			case Mnemonic::Rrsh_I_V:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _rotr(_forceRead(Indirect{ instr->arg1 }), _tryRead(Value{ instr->arg2 })));
				break;
		}

		return true;
	}

}	//sbl::vm