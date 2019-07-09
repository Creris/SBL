#pragma once

#ifndef COMMON_INSTRUCTION_HEADER_H_
#define COMMON_INSTRUCTION_HEADER_H_

#include <cstdint>
#include <cstdio>
#include <vector>

namespace sbl::cmn {
	/*
		Note:
			Mnemonics are split into groups according to what the instructions
			that these mnemonics encode do.

			Furthermore, the mnemonics are split into chunks of size 128(2^7)
			which creates a lot of artifical empty holes where mnemonic is
			smaller than some other mnemonic but does not encode anything.

			The space wasted with this structure is and given chunk size is: 431.

			Smaller chunk sizes can be used, but if we want to add more mnemonics
			into certain category in future, we will run out of space faster
			which will lead into the need to create new detached chunk of given
			category, which will slightly slower down the VM too since it will require
			to perform additional checks.

			Currently the spare slots per category is as follows:
				Basic:		 66
				Arithmetic:	 20
				Logical:	 29
				Allocation:	 68
				Interrupt:	 82
				Privilege:	 97
				Float:		 69
	*/
	enum class Mnemonic {
		/*
			Basic instructions
		*/
		Nop,
		Halt, End,
		Ret,
		Loop, Endloop,

		Push_R, Push_A, Push_I, Push_V, Push_All,
		Pop_R, Pop_A, Pop_I, Pop_All,

		Clear_All,

		Inc_R, Inc_A, Inc_I,
		Dec_R, Dec_A, Dec_I,

		Call_R, Call_A, Call_I,
		RCall_R, RCall_A, RCall_I,

		Read_R, Read_A, Read_I,
		Readstr_A, Readstr_I,

		Print_R, Print_A, Print_I, Print_V,
		PrintS_R, PrintS_A, PrintS_I, PrintS_V,
		PrintC_R, PrintC_A, PrintC_I, PrintC_V,
		Printstr_A, Printstr_I,

		Jmp_R, Jmp_A, Jmp_I,
		Jb_R, Jb_A, Jb_I,
		Jnb_R, Jnb_A, Jnb_I,
		Jbe_R, Jbe_A, Jbe_I,
		Jnbe_R, Jnbe_A, Jnbe_I,
		Jl_R, Jl_A, Jl_I,
		Jnl_R, Jnl_A, Jnl_I,
		Jle_R, Jle_A, Jle_I,
		Jnle_R, Jnle_A, Jnle_I,
		Jz_R, Jz_A, Jz_I,
		Jnz_R, Jnz_A, Jnz_I,
		Je_R, Je_A, Je_I,
		Jne_R, Jne_A, Jne_I,

		RJmp_R, RJmp_A, RJmp_I,
		RJb_R, RJb_A, RJb_I,
		RJnb_R, RJnb_A, RJnb_I,
		RJbe_R, RJbe_A, RJbe_I,
		RJnbe_R, RJnbe_A, RJnbe_I,
		RJl_R, RJl_A, RJl_I,
		RJnl_R, RJnl_A, RJnl_I,
		RJle_R, RJle_A, RJle_I,
		RJnle_R, RJnle_A, RJnle_I,
		RJz_R, RJz_A, RJz_I,
		RJnz_R, RJnz_A, RJnz_I,
		RJe_R, RJe_A, RJe_I,
		RJne_R, RJne_A, RJne_I,

		Time_R, Time_A, Time_I,
		Time64_R, Time64_A, Time64_I,

		ICount_R, ICount_A, ICount_I,
		ICount64_R, ICount64_A, ICount64_I,

		ExecInstr_A, ExecInstr_I,

		Mov_R_R, Mov_R_A, Mov_R_I, Mov_R_V,
		Mov_A_R, Mov_A_A, Mov_A_I, Mov_A_V,
		Mov_I_R, Mov_I_A, Mov_I_I, Mov_I_V,

		Move_R_R, Move_R_A, Move_R_I, Move_R_V,
		Move_A_R, Move_A_A, Move_A_I, Move_A_V,
		Move_I_R, Move_I_A, Move_I_I, Move_I_V,

		Movne_R_R, Movne_R_A, Movne_R_I, Movne_R_V,
		Movne_A_R, Movne_A_A, Movne_A_I, Movne_A_V,
		Movne_I_R, Movne_I_A, Movne_I_I, Movne_I_V,

		Movz_R_R, Movz_R_A, Movz_R_I, Movz_R_V,
		Movz_A_R, Movz_A_A, Movz_A_I, Movz_A_V,
		Movz_I_R, Movz_I_A, Movz_I_I, Movz_I_V,

		Movnz_R_R, Movnz_R_A, Movnz_R_I, Movnz_R_V,
		Movnz_A_R, Movnz_A_A, Movnz_A_I, Movnz_A_V,
		Movnz_I_R, Movnz_I_A, Movnz_I_I, Movnz_I_V,

		Laddr_R_A, Laddr_R_I, Laddr_A_A, Laddr_A_I,
		Laddr_I_A, Laddr_I_I,

		Loadload_R_A, Loadload_R_I,
		Loadload_A_A, Loadload_A_I,
		Loadload_I_A, Loadload_I_I,

		Vcall_R_R, Vcall_R_A, Vcall_R_I, Vcall_R_V,
		Vcall_A_R, Vcall_A_A, Vcall_A_I, Vcall_A_V,
		Vcall_I_R, Vcall_I_A, Vcall_I_I, Vcall_I_V,
		Vcall_V_R, Vcall_V_A, Vcall_V_I, Vcall_V_V,

		RVcall_R_R, RVcall_R_A, RVcall_R_I, RVcall_R_V,
		RVcall_A_R, RVcall_A_A, RVcall_A_I, RVcall_A_V,
		RVcall_I_R, RVcall_I_A, RVcall_I_I, RVcall_I_V,
		RVcall_V_R, RVcall_V_A, RVcall_V_I, RVcall_V_V,

		NtvCall_R, NtvCall_A, NtvCall_I, NtvCall_V,

		GetNtvId_R_A, GetNtvId_R_I,
		GetNtvId_A_A, GetNtvId_A_I,
		GetNtvId_I_A, GetNtvId_I_I,

		Xchg_R_R, Xchg_R_A, Xchg_R_I,
		Xchg_A_R, Xchg_A_A, Xchg_A_I,
		Xchg_I_R, Xchg_I_A, Xchg_I_I,

		ClrCb,

		Hotpatch_A_A, Hotpatch_A_I,
		Hotpatch_I_A, Hotpatch_I_I,

		RHotpatch_A_A, RHotpatch_A_I,
		RHotpatch_I_A, RHotpatch_I_I,

		EnableExt_R, EnableExt_A, EnableExt_I, EnableExt_V,
		DisableExt_R, DisableExt_A, DisableExt_I, DisableExt_V,

		IsExtEnabled_R_R, IsExtEnabled_R_A, IsExtEnabled_R_I, IsExtEnabled_R_V,
		IsExtEnabled_A_R, IsExtEnabled_A_A, IsExtEnabled_A_I, IsExtEnabled_A_V,
		IsExtEnabled_I_R, IsExtEnabled_I_A, IsExtEnabled_I_I, IsExtEnabled_I_V,

		SetSegmntAccs_R_R, SetSegmntAccs_R_A, SetSegmntAccs_R_I, SetSegmntAccs_R_V,
		SetSegmntAccs_A_R, SetSegmntAccs_A_A, SetSegmntAccs_A_I, SetSegmntAccs_A_V,
		SetSegmntAccs_I_R, SetSegmntAccs_I_A, SetSegmntAccs_I_I, SetSegmntAccs_I_V,
		SetSegmntAccs_V_R, SetSegmntAccs_V_A, SetSegmntAccs_V_I, SetSegmntAccs_V_V,

		GetSegmntAccs_R_R, GetSegmntAccs_R_A, GetSegmntAccs_R_I, GetSegmntAccs_R_V,
		GetSegmntAccs_A_R, GetSegmntAccs_A_A, GetSegmntAccs_A_I, GetSegmntAccs_A_V,
		GetSegmntAccs_I_R, GetSegmntAccs_I_A, GetSegmntAccs_I_I, GetSegmntAccs_I_V,

		/*
			End of Basic instructions
			Beginning of Arithmetic instructions
		*/
		Add_R_R = 128 * 3, Add_R_I, Add_R_A, Add_R_V,
		Add_I_R, Add_I_I, Add_I_A, Add_I_V,
		Add_A_R, Add_A_I, Add_A_A, Add_A_V,

		Sub_R_R, Sub_R_A, Sub_R_I, Sub_R_V,
		Sub_A_R, Sub_A_A, Sub_A_I, Sub_A_V,
		Sub_I_R, Sub_I_A, Sub_I_I, Sub_I_V,

		Mul_R_R, Mul_R_A, Mul_R_I, Mul_R_V,
		Mul_A_R, Mul_A_A, Mul_A_I, Mul_A_V,
		Mul_I_R, Mul_I_A, Mul_I_I, Mul_I_V,

		Div_R_R, Div_R_A, Div_R_I, Div_R_V,
		Div_A_R, Div_A_A, Div_A_I, Div_A_V,
		Div_I_R, Div_I_A, Div_I_I, Div_I_V,

		Mod_R_R, Mod_R_A, Mod_R_I, Mod_R_V,
		Mod_A_R, Mod_A_A, Mod_A_I, Mod_A_V,
		Mod_I_R, Mod_I_A, Mod_I_I, Mod_I_V,

		Lsh_R_R, Lsh_R_A, Lsh_R_I, Lsh_R_V,
		Lsh_A_R, Lsh_A_A, Lsh_A_I, Lsh_A_V,
		Lsh_I_R, Lsh_I_A, Lsh_I_I, Lsh_I_V,

		Rlsh_R_R, Rlsh_R_A, Rlsh_R_I, Rlsh_R_V,
		Rlsh_A_R, Rlsh_A_A, Rlsh_A_I, Rlsh_A_V,
		Rlsh_I_R, Rlsh_I_A, Rlsh_I_I, Rlsh_I_V,

		Rsh_R_R, Rsh_R_A, Rsh_R_I, Rsh_R_V,
		Rsh_A_R, Rsh_A_A, Rsh_A_I, Rsh_A_V,
		Rsh_I_R, Rsh_I_A, Rsh_I_I, Rsh_I_V,

		Rrsh_R_R, Rrsh_R_A, Rrsh_R_I, Rrsh_R_V,
		Rrsh_A_R, Rrsh_A_A, Rrsh_A_I, Rrsh_A_V,
		Rrsh_I_R, Rrsh_I_A, Rrsh_I_I, Rrsh_I_V,

		/*
			End of Arithmetic instructions
			Beginning of Logical instructions
		*/
		And_R_R = 128 * 4, And_R_A, And_R_I, And_R_V,
		And_A_R, And_A_A, And_A_I, And_A_V,
		And_I_R, And_I_A, And_I_I, And_I_V,

		Or_R_R, Or_R_A, Or_R_I, Or_R_V,
		Or_A_R, Or_A_A, Or_A_I, Or_A_V,
		Or_I_R, Or_I_A, Or_I_I, Or_I_V,

		Xor_R_R, Xor_R_A, Xor_R_I, Xor_R_V,
		Xor_A_R, Xor_A_A, Xor_A_I, Xor_A_V,
		Xor_I_R, Xor_I_A, Xor_I_I, Xor_I_V,

		Eq_R_R, Eq_R_A, Eq_R_I, Eq_R_V,
		Eq_A_R, Eq_A_A, Eq_A_I, Eq_A_V,
		Eq_I_R, Eq_I_A, Eq_I_I, Eq_I_V,

		Neq_R_R, Neq_R_A, Neq_R_I, Neq_R_V,
		Neq_A_R, Neq_A_A, Neq_A_I, Neq_A_V,
		Neq_I_R, Neq_I_A, Neq_I_I, Neq_I_V,

		Lt_R_R, Lt_R_A, Lt_R_I, Lt_R_V,
		Lt_A_R, Lt_A_A, Lt_A_I, Lt_A_V,
		Lt_I_R, Lt_I_A, Lt_I_I, Lt_I_V,

		Bt_R_R, Bt_R_A, Bt_R_I, Bt_R_V,
		Bt_A_R, Bt_A_A, Bt_A_I, Bt_A_V,
		Bt_I_R, Bt_I_A, Bt_I_I, Bt_I_V,

		Not_R, Not_A, Not_I,

		Test_R_R, Test_R_A, Test_R_I, Test_R_V,
		Test_A_R, Test_A_A, Test_A_I, Test_A_V,
		Test_I_R, Test_I_A, Test_I_I, Test_I_V,

		/*
			End of Logical instructions
			Beginning of Allocation instructions
		*/
		Alloc_R_R = 128 * 5, Alloc_R_A, Alloc_R_I, Alloc_R_V,
		Alloc_A_R, Alloc_A_A, Alloc_A_I, Alloc_A_V,
		Alloc_I_R, Alloc_I_A, Alloc_I_I, Alloc_I_V,

		Dealloc_R, Dealloc_A, Dealloc_I,

		DynOffset_R, DynOffset_A, DynOffset_I, DynOffset_V,
		ClrDynOffset,

		LoadDyn_R_R, LoadDyn_R_A, LoadDyn_R_I, LoadDyn_R_V,
		LoadDyn_A_R, LoadDyn_A_A, LoadDyn_A_I, LoadDyn_A_V,
		LoadDyn_I_R, LoadDyn_I_A, LoadDyn_I_I, LoadDyn_I_V,

		WriteDyn_R_R, WriteDyn_R_A, WriteDyn_R_I, WriteDyn_R_V,
		WriteDyn_A_R, WriteDyn_A_A, WriteDyn_A_I, WriteDyn_A_V,
		WriteDyn_I_R, WriteDyn_I_A, WriteDyn_I_I, WriteDyn_I_V,
		WriteDyn_V_R, WriteDyn_V_A, WriteDyn_V_I, WriteDyn_V_V,

		GetDynSize_R_R, GetDynSize_R_A, GetDynSize_R_I, GetDynSize_R_V,
		GetDynSize_A_R, GetDynSize_A_A, GetDynSize_A_I, GetDynSize_A_V,
		GetDynSize_I_R, GetDynSize_I_A, GetDynSize_I_I, GetDynSize_I_V,

		/*
			End of Allocation instructions
			Beginning of Interrupt instructions
		*/
		Raise_R = 128 * 6, Raise_A, Raise_I, Raise_V,
		DisableInt_R, DisableInt_V,
		EnableInt_R, EnableInt_V,
		ICountInt_R, ICountInt_V,

		RegInt_R_R, Regint_R_A, Regint_R_I,
		RegInt_A_R, Regint_A_A, Regint_A_I,
		RegInt_I_R, Regint_I_A, Regint_I_I,
		RegInt_V_R, Regint_V_A, Regint_V_I,

		RRegInt_R_R, RRegInt_R_A, RRegInt_R_I,
		RRegInt_A_R, RRegInt_A_A, RRegInt_A_I,
		RRegInt_I_R, RRegInt_I_A, RRegInt_I_I,
		RRegInt_V_R, RRegInt_V_A, RRegInt_V_I,

		IRet,

		RICountInt_R, RICountInt_V,

		DisableAllInts,
		RestoreInts,
		EnableAllInts,

		ICountInt64_R, ICountInt64_A, ICountInt64_I,
		RICountInt64_R, RICountInt64_A, RICountInt64_I,

		/*
			End of Interrupt instructions
			Beginning of Privilege instructions
		*/
		GetPrivlg_R = 128 * 7, GetPrivlg_A, GetPrivlg_I,

		SetInstrPrivlg_R_R, SetInstrPrivlg_R_A, SetInstrPrivlg_R_I, SetInstrPrivlg_R_V,
		SetInstrPrivlg_A_R, SetInstrPrivlg_A_A, SetInstrPrivlg_A_I, SetInstrPrivlg_A_V,
		SetInstrPrivlg_I_R, SetInstrPrivlg_I_A, SetInstrPrivlg_I_I, SetInstrPrivlg_I_V,
		SetInstrPrivlg_V_R, SetInstrPrivlg_V_A, SetInstrPrivlg_V_I, SetInstrPrivlg_V_V,

		SetPrivlg_R, SetPrivlg_A, SetPrivlg_I, SetPrivlg_V,

		GetInstrPrivlg_R_R, GetInstrPrivlg_R_A, GetInstrPrivlg_R_I, GetInstrPrivlg_R_V,
		GetInstrPrivlg_A_R, GetInstrPrivlg_A_A, GetInstrPrivlg_A_I, GetInstrPrivlg_A_V,
		GetInstrPrivlg_I_R, GetInstrPrivlg_I_A, GetInstrPrivlg_I_I, GetInstrPrivlg_I_V,

		SetIntPrivlg_R_R, SetIntPrivlg_R_A, SetIntPrivlg_R_I, SetIntPrivlg_R_V,
		SetIntPrivlg_A_R, SetIntPrivlg_A_A, SetIntPrivlg_A_I, SetIntPrivlg_A_V,
		SetIntPrivlg_I_R, SetIntPrivlg_I_A, SetIntPrivlg_I_I, SetIntPrivlg_I_V,
		SetIntPrivlg_V_R, SetIntPrivlg_V_A, SetIntPrivlg_V_I, SetIntPrivlg_V_V,

		GetIntPrivlg_R_R, GetIntPrivlg_R_A, GetIntPrivlg_R_I, GetIntPrivlg_R_V,
		GetIntPrivlg_A_R, GetIntPrivlg_A_A, GetIntPrivlg_A_I, GetIntPrivlg_A_V,
		GetIntPrivlg_I_R, GetIntPrivlg_I_A, GetIntPrivlg_I_I, GetIntPrivlg_I_V,

		SetIntExecPrivlg_R_R, SetIntExecPrivlg_R_A, SetIntExecPrivlg_R_I, SetIntExecPrivlg_R_V,
		SetIntExecPrivlg_A_R, SetIntExecPrivlg_A_A, SetIntExecPrivlg_A_I, SetIntExecPrivlg_A_V,
		SetIntExecPrivlg_I_R, SetIntExecPrivlg_I_A, SetIntExecPrivlg_I_I, SetIntExecPrivlg_I_V,
		SetIntExecPrivlg_V_R, SetIntExecPrivlg_V_A, SetIntExecPrivlg_V_I, SetIntExecPrivlg_V_V,

		GetIntExecPrivlg_R_R, GetIntExecPrivlg_R_A, GetIntExecPrivlg_R_I, GetIntExecPrivlg_R_V,
		GetIntExecPrivlg_A_R, GetIntExecPrivlg_A_A, GetIntExecPrivlg_A_I, GetIntExecPrivlg_A_V,
		GetIntExecPrivlg_I_R, GetIntExecPrivlg_I_A, GetIntExecPrivlg_I_I, GetIntExecPrivlg_I_V,

		SetExtPrivlg_R_R, SetExtPrivlg_R_A, SetExtPrivlg_R_I, SetExtPrivlg_R_V,
		SetExtPrivlg_A_R, SetExtPrivlg_A_A, SetExtPrivlg_A_I, SetExtPrivlg_A_V,
		SetExtPrivlg_I_R, SetExtPrivlg_I_A, SetExtPrivlg_I_I, SetExtPrivlg_I_V,
		SetExtPrivlg_V_R, SetExtPrivlg_V_A, SetExtPrivlg_V_I, SetExtPrivlg_V_V,

		GetExtPrivlg_R_R, GetExtPrivlg_R_A, GetExtPrivlg_R_I, GetExtPrivlg_R_V,
		GetExtPrivlg_A_R, GetExtPrivlg_A_A, GetExtPrivlg_A_I, GetExtPrivlg_A_V,
		GetExtPrivlg_I_R, GetExtPrivlg_I_A, GetExtPrivlg_I_I, GetExtPrivlg_I_V,
        
		PCall_R_R, PCall_R_A, PCall_R_I, PCall_R_V,
		PCall_A_R, PCall_A_A, PCall_A_I, PCall_A_V,
		PCall_I_R, PCall_I_A, PCall_I_I, PCall_I_V,

		RPCall_R_R, RPCall_R_A, RPCall_R_I, RPCall_R_V,
		RPCall_A_R, RPCall_A_A, RPCall_A_I, RPCall_A_V,
		RPCall_I_R, RPCall_I_A, RPCall_I_I, RPCall_I_V,

		PNtvCall_R_R, PNtvCall_R_A, PNtvCall_R_I, PNtvCall_R_V,
		PNtvCall_A_R, PNtvCall_A_A, PNtvCall_A_I, PNtvCall_A_V,
		PNtvCall_I_R, PNtvCall_I_A, PNtvCall_I_I, PNtvCall_I_V,
		PNtvCall_V_R, PNtvCall_V_A, PNtvCall_V_I, PNtvCall_V_V,

		/*
			End of Privilege instructions
			Beginning of Floating point instructions
		*/
		FpMov_R_R = 128 * 9, FpMov_R_V,

		FpAdd_R_R, FpAdd_R_V,
		FpSub_R_R, FpSub_R_V,
		FpMul_R_R, FpMul_R_V,
		FpDiv_R_R, FpDiv_R_V,

		FpSign_R, FpSign_V,

		FpRound_R,
		FpFloor_R,
		FpCeil_R,

		FpTest_R_R, FpTest_R_V,
		FpTest_V_R, FpTest_V_V,

		FpMod_R_R, FpMod_R_V,

		FpNan_R,
		FpInf_R,

		FpPi_R,
		FpE_R,
		FpCstLn2_R,
		FpCstLn10_R,
		FpCstLog10_R,

		FpSin_R_R, FpSin_R_V,
		FpCos_R_R, FpCos_R_V,
		FpTan_R_R, FpTan_R_V,

		FpLn_R_R, FpLn_R_V,
		FpLog2_R_R, FpLog2_R_V,
		FpLog10_R_R, FpLog10_R_V,

		FpRoot_R,
		FpCsqrt_R,

		FpSq_R,
		FpPow_R_R, FpPow_R_V,

		FpExp_R,

		FpRoot_R_R, FpRoot_R_V,

		FpPrint_R, FpPrint_V,

		MvSignFlag_R, MvSignFlag_A, MvSignFlag_I,
		MvInfFlag_R, MvInfFlag_A, MvInfFlag_I,
		MvNanFlag_R, MvNanFlag_A, MvNanFlag_I,
		
		/*
			End of Floating point instructions
			Not valid instructions, only tags remaining:
		*/

		TotalCount = 128 * 10,
		Invalid,

		NativeToCodeCall,
	};

	inline bool operator>=(uint32_t v, Mnemonic m) {
		return v >= static_cast<uint32_t>(m);
	}

	struct Instruction {
		Mnemonic mnemonic;
		uint32_t arg1;
		uint32_t arg2;

		static Instruction* fromStream(std::vector<uint32_t>& source, uint32_t at) {
			return reinterpret_cast<Instruction*>(&source[at]);
		}

		static Instruction* fromStream(uint32_t* source, uint32_t at) {
			return reinterpret_cast<Instruction*>(source + at);
		}

		template <class T>
		static Instruction* fromStream(T& source, uint32_t at) {
			return reinterpret_cast<Instruction*>(&source[0] + at);
		}


		static Instruction* fromAddress(uint32_t* addr) {
			return reinterpret_cast<Instruction*>(addr);
		}

		static Instruction* fromAddress(uint32_t* addr, uint32_t offset) {
			return reinterpret_cast<Instruction*>(addr + offset);
		}
	};
}

#endif //COMMON_INSTRUCTION_HEADER_H_