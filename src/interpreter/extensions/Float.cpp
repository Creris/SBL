#include <cmath>
#include "../VM.hpp"
#include "../../common/Instruction.hpp"

namespace sbl::vm {
	bool VM::_performFloat(Instruction* instr) {
		switch (instr->mnemonic) {
			case Mnemonic::FpMov_R_R:
				_tryWrite(FpRegister{ instr->arg1 }, _tryRead(FpRegister{ instr->arg2 }));
				break;
			case Mnemonic::FpMov_R_V:
				_tryWrite(FpRegister{ instr->arg1 }, _tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) }));
				break;
			case Mnemonic::FpAdd_R_R:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, _forceRead(FpRegister{ instr->arg1 }) + _tryRead(FpRegister{ instr->arg2 }));
				break;
			case Mnemonic::FpAdd_R_V:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, _forceRead(FpRegister{ instr->arg1 }) + _tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) }));
				break;
			case Mnemonic::FpSub_R_R:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, _forceRead(FpRegister{ instr->arg1 }) - _tryRead(FpRegister{ instr->arg2 }));
				break;
			case Mnemonic::FpSub_R_V:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, _forceRead(FpRegister{ instr->arg1 }) - _tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) }));
				break;
			case Mnemonic::FpMul_R_R:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, _forceRead(FpRegister{ instr->arg1 }) * _tryRead(FpRegister{ instr->arg2 }));
				break;
			case Mnemonic::FpMul_R_V:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, _forceRead(FpRegister{ instr->arg1 }) * _tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) }));
				break;
			case Mnemonic::FpDiv_R_R:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, _forceRead(FpRegister{ instr->arg1 }) / _tryRead(FpRegister{ instr->arg2 }));
				break;
			case Mnemonic::FpDiv_R_V:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, _forceRead(FpRegister{ instr->arg1 }) / _tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) }));
				break;
			case Mnemonic::FpSign_R:
			{
				auto f = _tryRead(FpRegister{ instr->arg1 });
				if (f < 0) {
					controlByte |= TestFloatNegative;
				}
				else if (f > 0) {
					controlByte |= TestFloatPositive;
				}
				else {
					controlByte |= TestFloatZero;
				}
			}
			break;
			case Mnemonic::FpSign_V:
			{
				auto f = _tryRead(FpValue{ reinterpret_cast<float&>(instr->arg1) });
				if (f < 0) {
					controlByte |= TestFloatNegative;
				}
				else if (f > 0) {
					controlByte |= TestFloatPositive;
				}
				else {
					controlByte |= TestFloatZero;
				}
			}
			break;
			case Mnemonic::FpRound_R:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, std::round(_forceRead(FpRegister{ instr->arg1 })));
				break;
			case Mnemonic::FpFloor_R:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, std::floor(_forceRead(FpRegister{ instr->arg1 })));
				break;
			case Mnemonic::FpCeil_R:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, std::ceil(_forceRead(FpRegister{ instr->arg1 })));
				break;
			case Mnemonic::FpTest_R_R:
				setFloatControl(_tryRead(FpRegister{ instr->arg1 }), _tryRead(FpRegister{ instr->arg2 }));
				break;
			case Mnemonic::FpTest_R_V:
				setFloatControl(_tryRead(FpRegister{ instr->arg1 }), _tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) }));
				break;
			case Mnemonic::FpTest_V_R:
				setFloatControl(_tryRead(FpValue{ reinterpret_cast<float&>(instr->arg1) }), _tryRead(FpRegister{ instr->arg2 }));
				break;
			case Mnemonic::FpTest_V_V:
				setFloatControl(_tryRead(FpValue{ reinterpret_cast<float&>(instr->arg1) }), _tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) }));
				break;
			case Mnemonic::FpMod_R_R:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_tryWrite(FpRegister{ instr->arg1 }, std::fmod(_forceRead(FpRegister{ instr->arg1 }), _tryRead(FpRegister{ instr->arg2 })));
				break;
			case Mnemonic::FpMod_R_V:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_tryWrite(FpRegister{ instr->arg1 }, std::fmod(_forceRead(FpRegister{ instr->arg1 }), _tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) })));
				break;
			case Mnemonic::FpNan_R:
				if (std::isnan(_tryRead(FpRegister{ instr->arg1 }))) {
					controlByte |= TestFloatNan;
				}
				else {
					controlByte &= ~TestFloatNan;
				}
				break;
			case Mnemonic::FpInf_R:
				if (std::isinf(_tryRead(FpRegister{ instr->arg1 }))) {
					controlByte |= TestFloatInf;
				}
				else {
					controlByte &= ~TestFloatInf;
				}
				break;
			case Mnemonic::FpPi_R:
				_tryWrite(FpRegister{ instr->arg1 }, 3.1415926536f);
				break;
			case Mnemonic::FpE_R:
				_tryWrite(FpRegister{ instr->arg1 }, 2.7182818285f);
				break;
			case Mnemonic::FpCstLn2_R:
				_tryWrite(FpRegister{ instr->arg1 }, 0.6931471806f);
				break;
			case Mnemonic::FpCstLn10_R:
				_tryWrite(FpRegister{ instr->arg1 }, 2.302585093f);
				break;
			case Mnemonic::FpCstLog10_R:
				_tryWrite(FpRegister{ instr->arg1 }, 0.3010299957f);
				break;
			case Mnemonic::FpSin_R_R:
				_tryWrite(FpRegister{ instr->arg1 }, std::sin(_tryRead(FpRegister{ instr->arg2 })));
				break;
			case Mnemonic::FpSin_R_V:
				_tryWrite(FpRegister{ instr->arg1 }, std::sin(_tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) })));
				break;
			case Mnemonic::FpCos_R_R:
				_tryWrite(FpRegister{ instr->arg1 }, std::cos(_tryRead(FpRegister{ instr->arg2 })));
				break;
			case Mnemonic::FpCos_R_V:
				_tryWrite(FpRegister{ instr->arg1 }, std::cos(_tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) })));
				break;
			case Mnemonic::FpTan_R_R:
				_tryWrite(FpRegister{ instr->arg1 }, std::tan(_tryRead(FpRegister{ instr->arg2 })));
				break;
			case Mnemonic::FpTan_R_V:
				_tryWrite(FpRegister{ instr->arg1 }, std::tan(_tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) })));
				break;
			case Mnemonic::FpLn_R_R:
				_tryWrite(FpRegister{ instr->arg1 }, std::log(_tryRead(FpRegister{ instr->arg2 })));
				break;
			case Mnemonic::FpLn_R_V:
				_tryWrite(FpRegister{ instr->arg1 }, std::log(_tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) })));
				break;
			case Mnemonic::FpLog2_R_R:
				_tryWrite(FpRegister{ instr->arg1 }, std::log2(_tryRead(FpRegister{ instr->arg2 })));
				break;
			case Mnemonic::FpLog2_R_V:
				_tryWrite(FpRegister{ instr->arg1 }, std::log2(_tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) })));
				break;
			case Mnemonic::FpLog10_R_R:
				_tryWrite(FpRegister{ instr->arg1 }, std::log10(_tryRead(FpRegister{ instr->arg2 })));
				break;
			case Mnemonic::FpLog10_R_V:
				_tryWrite(FpRegister{ instr->arg1 }, std::log10(_tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) })));
				break;
			case Mnemonic::FpRoot_R:
				_tryWrite(FpRegister{ instr->arg1 }, std::sqrt(_tryRead(FpRegister{ instr->arg1 })));
				break;
			case Mnemonic::FpCsqrt_R:
				_assertReadWrite(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, std::cbrt(_forceRead(FpRegister{ instr->arg1 })));
				break;
			case Mnemonic::FpSq_R:
			{
				_assertReadWrite(FpRegister{ instr->arg1 });
				auto v = _forceRead(FpRegister{ instr->arg1 });
				_forceWrite(FpRegister{ instr->arg1 }, v * v);
			}
			break;
			case Mnemonic::FpPow_R_R:
			case Mnemonic::FpRoot_R_R:
				_tryWrite(FpRegister{ instr->arg1 }, std::pow(_tryRead(FpRegister{ instr->arg1 }), _tryRead(FpRegister{ instr->arg2 })));
				break;
			case Mnemonic::FpPow_R_V:
			case Mnemonic::FpRoot_R_V:
				_tryWrite(FpRegister{ instr->arg1 }, std::pow(_tryRead(FpRegister{ instr->arg1 }), _tryRead(FpValue{ reinterpret_cast<float&>(instr->arg2) })));
				break;
			case Mnemonic::FpExp_R:
				_tryWrite(FpRegister{ instr->arg1 }, std::exp(_tryRead(FpRegister{ instr->arg1 })));
				break;
			case Mnemonic::FpPrint_R:
				std::cout << _tryRead(FpRegister{ instr->arg1 });
				break;
			case Mnemonic::FpPrint_V:
				std::cout << _tryRead(FpValue{ reinterpret_cast<float&>(instr->arg1) });
				break;
			case Mnemonic::MvSignFlag_R:
				_tryWrite(Register{ instr->arg1 }, (controlByte & (TestFloatPositive | TestFloatNegative)) >> 6);
				break;
			case Mnemonic::MvSignFlag_A:
				_tryWrite(Address{ instr->arg1 }, (controlByte & (TestFloatPositive | TestFloatNegative)) >> 6);
				break;
			case Mnemonic::MvSignFlag_I:
				_tryWrite(Indirect{ instr->arg1 }, (controlByte & (TestFloatPositive | TestFloatNegative)) >> 6);
				break;
			case Mnemonic::MvInfFlag_R:
				_tryWrite(Register{ instr->arg1 }, (controlByte & (TestFloatPositive | TestFloatNegative)) >> 9);
				break;
			case Mnemonic::MvInfFlag_A:
				_tryWrite(Address{ instr->arg1 }, (controlByte & (TestFloatPositive | TestFloatNegative)) >> 9);
				break;
			case Mnemonic::MvInfFlag_I:
				_tryWrite(Indirect{ instr->arg1 }, (controlByte & (TestFloatPositive | TestFloatNegative)) >> 9);
				break;
			case Mnemonic::MvNanFlag_R:
				_tryWrite(Register{ instr->arg1 }, (controlByte & (TestFloatPositive | TestFloatNegative)) >> 10);
				break;
			case Mnemonic::MvNanFlag_A:
				_tryWrite(Address{ instr->arg1 }, (controlByte & (TestFloatPositive | TestFloatNegative)) >> 10);
				break;
			case Mnemonic::MvNanFlag_I:
				_tryWrite(Indirect{ instr->arg1 }, (controlByte & (TestFloatPositive | TestFloatNegative)) >> 10);
				break;
		}

		return true;
	}

}	//sbl::vm