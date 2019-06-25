#pragma once

#ifndef HEADER_INSTRUCTION_H_
#define HEADER_INSTRUCTION_H_

#include <cstdint>
#include <cstdio>
#include <vector>

namespace sbl::vm {
	enum class Mnemonic : uint32_t {
		/* 0 Argument mnemonics */
		Nop,

		Halt,
		End,

		Ret,

		Loop,
		Endloop,

		/* 1 Argument mnemonics */
		Push_R, Push_A, Push_I, Push_V,

		Pop_R, Pop_A, Pop_I,

		Inc_R, Inc_A, Inc_I,
		Dec_R, Dec_A, Dec_I,

		Call_R, Call_A, Call_I,
		RCall_R, RCall_A, RCall_I,

		Read_R, Read_A, Read_I,
		Readstr_A, Readstr_I,

		Print_R, Print_A, Print_I, Print_V,
		PrintS_R, PrintS_A, PrintS_I, PrintS_V,
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

		Not_R,

		Time_R,
		ICount_R,

		Dealloc_R, Dealloc_A, Dealloc_I,

		Raise_R, Raise_A, Raise_I, Raise_V,
		DisableInt_R, DisableInt_V,
		EnableInt_R, EnableInt_V,
		ICountInt_R, ICountInt_V,

		/* 2 Argument mnemonics */

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

		Add_R_R, Add_R_I, Add_R_A, Add_R_V,
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

		Test_R_R, Test_R_A, Test_R_I, Test_R_V,
		Test_A_R, Test_A_A, Test_A_I, Test_A_V,
		Test_I_R, Test_I_A, Test_I_I, Test_I_V,


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


		And_R_R, And_R_A, And_R_I, And_R_V,
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


		Loadload_R_A, Loadload_R_I,
		Loadload_A_A, Loadload_A_I,
		Loadload_I_A, Loadload_I_I,

		Alloc_R_R, Alloc_R_A, Alloc_R_I, Alloc_R_V,
		Alloc_A_R, Alloc_A_A, Alloc_A_I, Alloc_A_V,
		Alloc_I_R, Alloc_I_A, Alloc_I_I, Alloc_I_V,

		Vcall_R_R, Vcall_R_A, Vcall_R_I, Vcall_R_V,
		Vcall_A_R, Vcall_A_A, Vcall_A_I, Vcall_A_V,
		Vcall_I_R, Vcall_I_A, Vcall_I_I, Vcall_I_V,
		Vcall_V_R, Vcall_V_A, Vcall_V_I, Vcall_V_V,

		RegInt_R_R, Regint_R_I, Regint_R_A,
		RegInt_A_R, Regint_A_I, Regint_A_A,
		RegInt_I_R, Regint_I_I, Regint_I_A,
		RegInt_V_R, Regint_V_I, Regint_V_A,

		RRegInt_R_R, RRegInt_R_I, RRegInt_R_A,
		RRegInt_A_R, RRegInt_A_I, RRegInt_A_A,
		RRegInt_I_R, RRegInt_I_I, RRegInt_I_A,
		RRegInt_V_R, RRegInt_V_I, RRegInt_V_A,

		Time_A, Time_I,
		Time64_R, Time64_A, Time64_I,
		ICount_A, ICount_I,
		ICount64_R, ICount64_A, ICount64_I,

		NtvCall_R, NtvCall_A, NtvCall_I, NtvCall_V,

		IRet,

		RICountInt_R, RICountInt_V,

		Push_All, Pop_All,

		Clear_All,

		GetNtvId_R_A, GetNtvId_R_I,
		GetNtvId_A_A, GetNtvId_A_I,
		GetNtvId_I_A, GetNtvId_I_I,

		Xchg_R_R, Xchg_R_A, Xchg_R_I,
		Xchg_A_R, Xchg_A_A, Xchg_A_I,
		Xchg_I_R, Xchg_I_A, Xchg_I_I,

		DisableAllInts,
		RestoreInts,
		EnableAllInts,

		ClrCb,

		ICountInt64_R, ICountInt64_A, ICountInt64_I,
		RICountInt64_R, RICountInt64_A, RICountInt64_I,

		//This variable is important, it marks the total count of instructions
		//It will stop working when we introduce 2byte instructions(>=192 on first byte)
		TotalCount,

		Invalid,
	};

	class Instruction {
		static Instruction& _constructInvalid() {
			static Instruction _invalid = Instruction{ Mnemonic::Invalid, {}, {} };
			return _invalid;
		}
	public:
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
	};
}

#endif //HEADER_INSTRUCTION_H_