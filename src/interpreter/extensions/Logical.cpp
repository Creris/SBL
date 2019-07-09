
#include "../VM.hpp"
#include "../../common/Instruction.hpp"

namespace sbl::vm {
	bool VM::_performLogical(Instruction* instr) {
		switch (instr->mnemonic) {
			case Mnemonic::And_R_R:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) & _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::And_R_A:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) & _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::And_R_I:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) & _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::And_R_V:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) & _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::And_A_R:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) & _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::And_A_A:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) & _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::And_A_I:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) & _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::And_A_V:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) & _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::And_I_R:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) & _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::And_I_A:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) & _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::And_I_I:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) & _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::And_I_V:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) & _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Or_R_R:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) | _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Or_R_A:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) | _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Or_R_I:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) | _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Or_R_V:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) | _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Or_A_R:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) | _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Or_A_A:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) | _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Or_A_I:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) | _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Or_A_V:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) | _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Or_I_R:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) | _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Or_I_A:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) | _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Or_I_I:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) | _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Or_I_V:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) | _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Xor_R_R:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) ^ _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Xor_R_A:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) ^ _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Xor_R_I:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) ^ _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Xor_R_V:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, _forceRead(Register{ instr->arg1 }) ^ _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Xor_A_R:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) ^ _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Xor_A_A:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) ^ _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Xor_A_I:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) ^ _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Xor_A_V:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, _forceRead(Address{ instr->arg1 }) ^ _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Xor_I_R:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) ^ _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Xor_I_A:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) ^ _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Xor_I_I:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) ^ _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Xor_I_V:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, _forceRead(Indirect{ instr->arg1 }) ^ _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Eq_R_R:
				setControl(_tryRead(Register{ instr->arg1 }) == _tryRead(Register{ instr->arg2 }), TestEqual | TestBiggerEqual | TestSmallerEqual);
				break;
			case Mnemonic::Eq_R_A:
				setControl(_tryRead(Register{ instr->arg1 }) == _tryRead(Address{ instr->arg2 }), TestEqual | TestBiggerEqual | TestSmallerEqual);
				break;
			case Mnemonic::Eq_R_I:
				setControl(_tryRead(Register{ instr->arg1 }) == _tryRead(Indirect{ instr->arg2 }), TestEqual | TestBiggerEqual | TestSmallerEqual);
				break;
			case Mnemonic::Eq_R_V:
				setControl(_tryRead(Register{ instr->arg1 }) == _tryRead(Value{ instr->arg2 }), TestEqual | TestBiggerEqual | TestSmallerEqual);
				break;
			case Mnemonic::Eq_A_R:
				setControl(_tryRead(Address{ instr->arg1 }) == _tryRead(Register{ instr->arg2 }), TestEqual | TestBiggerEqual | TestSmallerEqual);
				break;
			case Mnemonic::Eq_A_A:
				setControl(_tryRead(Address{ instr->arg1 }) == _tryRead(Address{ instr->arg2 }), TestEqual | TestBiggerEqual | TestSmallerEqual);
				break;
			case Mnemonic::Eq_A_I:
				setControl(_tryRead(Address{ instr->arg1 }) == _tryRead(Indirect{ instr->arg2 }), TestEqual | TestBiggerEqual | TestSmallerEqual);
				break;
			case Mnemonic::Eq_A_V:
				setControl(_tryRead(Address{ instr->arg1 }) == _tryRead(Value{ instr->arg2 }), TestEqual | TestBiggerEqual | TestSmallerEqual);
				break;
			case Mnemonic::Eq_I_R:
				setControl(_tryRead(Indirect{ instr->arg1 }) == _tryRead(Register{ instr->arg2 }), TestEqual | TestBiggerEqual | TestSmallerEqual);
				break;
			case Mnemonic::Eq_I_A:
				setControl(_tryRead(Indirect{ instr->arg1 }) == _tryRead(Address{ instr->arg2 }), TestEqual | TestBiggerEqual | TestSmallerEqual);
				break;
			case Mnemonic::Eq_I_I:
				setControl(_tryRead(Indirect{ instr->arg1 }) == _tryRead(Indirect{ instr->arg2 }), TestEqual | TestBiggerEqual | TestSmallerEqual);
				break;
			case Mnemonic::Eq_I_V:
				setControl(_tryRead(Indirect{ instr->arg1 }) == _tryRead(Value{ instr->arg2 }), TestEqual | TestBiggerEqual | TestSmallerEqual);
				break;
			case Mnemonic::Neq_R_R:
				setControl(_tryRead(Register{ instr->arg1 }) != _tryRead(Register{ instr->arg2 }), TestUnequal);
				break;
			case Mnemonic::Neq_R_A:
				setControl(_tryRead(Register{ instr->arg1 }) != _tryRead(Address{ instr->arg2 }), TestUnequal);
				break;
			case Mnemonic::Neq_R_I:
				setControl(_tryRead(Register{ instr->arg1 }) != _tryRead(Indirect{ instr->arg2 }), TestUnequal);
				break;
			case Mnemonic::Neq_R_V:
				setControl(_tryRead(Register{ instr->arg1 }) != _tryRead(Value{ instr->arg2 }), TestUnequal);
				break;
			case Mnemonic::Neq_A_R:
				setControl(_tryRead(Address{ instr->arg1 }) != _tryRead(Register{ instr->arg2 }), TestUnequal);
				break;
			case Mnemonic::Neq_A_A:
				setControl(_tryRead(Address{ instr->arg1 }) != _tryRead(Address{ instr->arg2 }), TestUnequal);
				break;
			case Mnemonic::Neq_A_I:
				setControl(_tryRead(Address{ instr->arg1 }) != _tryRead(Indirect{ instr->arg2 }), TestUnequal);
				break;
			case Mnemonic::Neq_A_V:
				setControl(_tryRead(Address{ instr->arg1 }) != _tryRead(Value{ instr->arg2 }), TestUnequal);
				break;
			case Mnemonic::Neq_I_R:
				setControl(_tryRead(Indirect{ instr->arg1 }) != _tryRead(Register{ instr->arg2 }), TestUnequal);
				break;
			case Mnemonic::Neq_I_A:
				setControl(_tryRead(Indirect{ instr->arg1 }) != _tryRead(Address{ instr->arg2 }), TestUnequal);
				break;
			case Mnemonic::Neq_I_I:
				setControl(_tryRead(Indirect{ instr->arg1 }) != _tryRead(Indirect{ instr->arg2 }), TestUnequal);
				break;
			case Mnemonic::Neq_I_V:
				setControl(_tryRead(Indirect{ instr->arg1 }) != _tryRead(Value{ instr->arg2 }), TestUnequal);
				break;
			case Mnemonic::Lt_R_R:
				setControl(_tryRead(Register{ instr->arg1 }) < _tryRead(Register{ instr->arg2 }), TestSmaller | TestUnequal | TestSmallerEqual);
				break;
			case Mnemonic::Lt_R_A:
				setControl(_tryRead(Register{ instr->arg1 }) < _tryRead(Address{ instr->arg2 }), TestSmaller | TestUnequal | TestSmallerEqual);
				break;
			case Mnemonic::Lt_R_I:
				setControl(_tryRead(Register{ instr->arg1 }) < _tryRead(Indirect{ instr->arg2 }), TestSmaller | TestUnequal | TestSmallerEqual);
				break;
			case Mnemonic::Lt_R_V:
				setControl(_tryRead(Register{ instr->arg1 }) < _tryRead(Value{ instr->arg2 }), TestSmaller | TestUnequal | TestSmallerEqual);
				break;
			case Mnemonic::Lt_A_R:
				setControl(_tryRead(Address{ instr->arg1 }) < _tryRead(Register{ instr->arg2 }), TestSmaller | TestUnequal | TestSmallerEqual);
				break;
			case Mnemonic::Lt_A_A:
				setControl(_tryRead(Address{ instr->arg1 }) < _tryRead(Address{ instr->arg2 }), TestSmaller | TestUnequal | TestSmallerEqual);
				break;
			case Mnemonic::Lt_A_I:
				setControl(_tryRead(Address{ instr->arg1 }) < _tryRead(Indirect{ instr->arg2 }), TestSmaller | TestUnequal | TestSmallerEqual);
				break;
			case Mnemonic::Lt_A_V:
				setControl(_tryRead(Address{ instr->arg1 }) < _tryRead(Value{ instr->arg2 }), TestSmaller | TestUnequal | TestSmallerEqual);
				break;
			case Mnemonic::Lt_I_R:
				setControl(_tryRead(Indirect{ instr->arg1 }) < _tryRead(Register{ instr->arg2 }), TestSmaller | TestUnequal | TestSmallerEqual);
				break;
			case Mnemonic::Lt_I_A:
				setControl(_tryRead(Indirect{ instr->arg1 }) < _tryRead(Address{ instr->arg2 }), TestSmaller | TestUnequal | TestSmallerEqual);
				break;
			case Mnemonic::Lt_I_I:
				setControl(_tryRead(Indirect{ instr->arg1 }) < _tryRead(Indirect{ instr->arg2 }), TestSmaller | TestUnequal | TestSmallerEqual);
				break;
			case Mnemonic::Lt_I_V:
				setControl(_tryRead(Indirect{ instr->arg1 }) < _tryRead(Value{ instr->arg2 }), TestSmaller | TestUnequal | TestSmallerEqual);
				break;
			case Mnemonic::Bt_R_R:
				setControl(_tryRead(Register{ instr->arg1 }) > _tryRead(Register{ instr->arg2 }), TestBigger | TestUnequal | TestBiggerEqual);
				break;
			case Mnemonic::Bt_R_A:
				setControl(_tryRead(Register{ instr->arg1 }) > _tryRead(Address{ instr->arg2 }), TestBigger | TestUnequal | TestBiggerEqual);
				break;
			case Mnemonic::Bt_R_I:
				setControl(_tryRead(Register{ instr->arg1 }) > _tryRead(Indirect{ instr->arg2 }), TestBigger | TestUnequal | TestBiggerEqual);
				break;
			case Mnemonic::Bt_R_V:
				setControl(_tryRead(Register{ instr->arg1 }) > _tryRead(Value{ instr->arg2 }), TestBigger | TestUnequal | TestBiggerEqual);
				break;
			case Mnemonic::Bt_A_R:
				setControl(_tryRead(Address{ instr->arg1 }) > _tryRead(Register{ instr->arg2 }), TestBigger | TestUnequal | TestBiggerEqual);
				break;
			case Mnemonic::Bt_A_A:
				setControl(_tryRead(Address{ instr->arg1 }) > _tryRead(Address{ instr->arg2 }), TestBigger | TestUnequal | TestBiggerEqual);
				break;
			case Mnemonic::Bt_A_I:
				setControl(_tryRead(Address{ instr->arg1 }) > _tryRead(Indirect{ instr->arg2 }), TestBigger | TestUnequal | TestBiggerEqual);
				break;
			case Mnemonic::Bt_A_V:
				setControl(_tryRead(Address{ instr->arg1 }) > _tryRead(Value{ instr->arg2 }), TestBigger | TestUnequal | TestBiggerEqual);
				break;
			case Mnemonic::Bt_I_R:
				setControl(_tryRead(Indirect{ instr->arg1 }) > _tryRead(Register{ instr->arg2 }), TestBigger | TestUnequal | TestBiggerEqual);
				break;
			case Mnemonic::Bt_I_A:
				setControl(_tryRead(Indirect{ instr->arg1 }) > _tryRead(Address{ instr->arg2 }), TestBigger | TestUnequal | TestBiggerEqual);
				break;
			case Mnemonic::Bt_I_I:
				setControl(_tryRead(Indirect{ instr->arg1 }) > _tryRead(Indirect{ instr->arg2 }), TestBigger | TestUnequal | TestBiggerEqual);
				break;
			case Mnemonic::Bt_I_V:
				setControl(_tryRead(Indirect{ instr->arg1 }) > _tryRead(Value{ instr->arg2 }), TestBigger | TestUnequal | TestBiggerEqual);
				break;
			case Mnemonic::Not_R:
				_assertReadWrite(Register{ instr->arg1 });
				_forceWrite(Register{ instr->arg1 }, ~_forceRead(Register{ instr->arg1 }));
				break;
			case Mnemonic::Not_A:
				_assertReadWrite(Address{ instr->arg1 });
				_forceWrite(Address{ instr->arg1 }, ~_forceRead(Address{ instr->arg1 }));
				break;
			case Mnemonic::Not_I:
				_assertReadWrite(Indirect{ instr->arg1 });
				_forceWrite(Indirect{ instr->arg1 }, ~_forceRead(Indirect{ instr->arg1 }));
				break;
			case Mnemonic::Test_R_R:
				setControl(_tryRead(Register{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Test_R_A:
				setControl(_tryRead(Register{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Test_R_I:
				setControl(_tryRead(Register{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Test_R_V:
				setControl(_tryRead(Register{ instr->arg1 }), _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Test_A_R:
				setControl(_tryRead(Address{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Test_A_A:
				setControl(_tryRead(Address{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Test_A_I:
				setControl(_tryRead(Address{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Test_A_V:
				setControl(_tryRead(Address{ instr->arg1 }), _tryRead(Value{ instr->arg2 }));
				break;
			case Mnemonic::Test_I_R:
				setControl(_tryRead(Indirect{ instr->arg1 }), _tryRead(Register{ instr->arg2 }));
				break;
			case Mnemonic::Test_I_A:
				setControl(_tryRead(Indirect{ instr->arg1 }), _tryRead(Address{ instr->arg2 }));
				break;
			case Mnemonic::Test_I_I:
				setControl(_tryRead(Indirect{ instr->arg1 }), _tryRead(Indirect{ instr->arg2 }));
				break;
			case Mnemonic::Test_I_V:
				setControl(_tryRead(Indirect{ instr->arg1 }), _tryRead(Value{ instr->arg2 }));
				break;
		}

		return true;
	}

}	//sbl::vm