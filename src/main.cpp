
#include <iostream>
#include <chrono>
#include <string_view>
#include <functional>
#include <numeric>
#include <iomanip>
#include <filesystem>

#include <intrin.h>

#include "interpreter/VM.hpp"

#include "interpreter/CompiledHeader.hpp"

int nativeCallCount = 0;

void native_test(sbl::vm::State& state) {
	//if (!nativeCallCount)
	//	std::cout << "Privilege: " << state.currentPrivilege() << "\n";

	std::cout << "Current privilege: " << state.currentPrivilege() << "\n";
	++nativeCallCount;
}

constexpr auto qq() {
	using namespace sbl::vm::literals;

	uint32_t c[] = {
		0,
		'sblx',
		0x80,
		0x800000,
		0x1000,
		0x00, 
		0x00,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};

	sbl::vm::CompiledHeader header{};
	//header.fromStream(c, 16 * 1024 * 1024);

	return header;
}

constexpr auto header = qq();
constexpr auto validate = header.validate();

constexpr auto qqw() {
	uint8_t u[] = { 0x00, 0x00, 0x01, 0x00 };
	uint32_t v = (u[0] << 24) | (u[1] << 16) | (u[2] << 8) | u[3];
	return v;
}

constexpr auto qqww = qqw();

#include <thread>

#include "common/FixedVector.hpp"

#include <random>

int distance(int x1, int y1, int x2, int y2) {
	int xDiff = x2 - x1;
	int yDiff = y2 - y1;
	return xDiff * xDiff + yDiff * yDiff;
}

#include <ctime>
#include <cmath>

void nearest() {
	struct Position {
		int x, y;
		bool operator==(const Position& p) {
			return x == p.x && y == p.y;
		}
	};

	std::srand(std::time(NULL));

	std::vector<Position> positions;
	std::mt19937 mt{ std::random_device{}() };
	std::uniform_int_distribution dist(-10000, 10000);

	/*
	positions.push_back({ 3, 2 });
	positions.push_back({ 0, 0 });
	positions.push_back({ 3, 1 });
	positions.push_back({ 4, 3 });
	positions.push_back({ 3, 3 });
	positions.push_back({ 2, 3 });
	//*/
	
	namespace ch = std::chrono;
	auto start = ch::high_resolution_clock::now().time_since_epoch();

	positions.reserve(1000000);
	
	///*
	for (int i = 0; i < 1000000; ++i) {
		positions.push_back({ dist(mt), dist(mt) });
		//positions.push_back({ (rand() % 20000) - 10000, (rand() % 20000) - 10000 });
	}
	//*/

	start = ch::high_resolution_clock::now().time_since_epoch() - start;
	std::cout << "Filling took " << ch::duration_cast<ch::microseconds>(start).count() / 1000.f << " ms.\n";

	//auto position = Position{ (rand() % 20000) - 10000, (rand() % 20000) - 10000 };
	auto position = Position{ dist(mt), dist(mt) };
	//position = { 4, 0 };
	auto nearestIdx = 0;
	auto nearestDistSq = distance(position.x, position.y, positions[0].x, positions[0].y);

	start = ch::high_resolution_clock::now().time_since_epoch();

	for (size_t i = 1; i < positions.size(); ++i) {
		auto dist = distance(position.x, position.y, positions[i].x, positions[i].y);
		if (dist < nearestDistSq) {
			nearestDistSq = dist;
			nearestIdx = i;
		}
	}

	start = ch::high_resolution_clock::now().time_since_epoch() - start;

	std::cout << "Searching nearest for [" << position.x << ", " << position.y << "]\n";
	std::cout << "Nearest is [" << positions[nearestIdx].x << ", "  << positions[nearestIdx].y << "]"
		<< " at index " << nearestIdx << " with distance squared: " << nearestDistSq << "\n";
	std::cout << "Search took " << ch::duration_cast<ch::microseconds>(start).count() / 1000.f << " ms.\n";
}

auto beautify = [](const std::string& s) {
	std::string r;
	r.reserve(s.size() + s.size() / 3);

	auto over = s.size() % 3;
	auto ctr = 0;

	for (; over > 0; --over) {
		r += s[ctr++];
	}

	if (ctr)
		r += ',';

	for (size_t i = ctr; i < s.size(); i += 3) {
		r += s[i];
		r += s[i + 1];
		r += s[i + 2];
		r += ',';
	}

	if (r.back() == ',')
		r.pop_back();

	return r;
};

std::vector<uint32_t> getGenerated(uint32_t numberElems) {
	std::vector<uint32_t> positions;
	std::mt19937 mt{ std::random_device{}() };
	std::uniform_int_distribution dist(10, 20000);

	positions.reserve(numberElems * 2 + 6);

	for (int i = 0; i < numberElems * 2; ++i) {
		positions.push_back(dist(mt));
	}

	positions.resize(4096 * ((positions.size() + 4095) / 4096));

	positions.push_back(static_cast<uint32_t>(sbl::vm::Mnemonic::Mov_R_V));
	positions.push_back(20);
	positions.push_back(dist(mt));

	positions.push_back(static_cast<uint32_t>(sbl::vm::Mnemonic::Mov_R_V));
	positions.push_back(21);
	positions.push_back(dist(mt));

	return positions;
}

void computeCpp(const std::vector<uint32_t>& v, uint32_t generated) {
	auto py = v.back();
	auto px = v[v.size() - 4];

	uint32_t dist = -1;
	auto distance = [](uint32_t x, uint32_t y, uint32_t ux, uint32_t uy) {
		auto xD = x - ux;
		auto yD = y - uy;
		return xD * xD + yD * yD;
	};

	for (int i = 0; i < std::min((uint32_t)v.size(), 2 * generated); i += 2) {
		auto d = distance(px, py, v[i], v[i + 1]);
		if (d < dist) {
			dist = d;
		}
	}

	std::cout << "Smallest dist is: " << dist << "\n\n";
}

#include <fstream>

int main() {
	static auto cast = [](auto m) constexpr { return static_cast<uint32_t>(m); };

	namespace fs = std::filesystem;

	using namespace sbl::vm;

	std::cout << sizeof(VM) << "\n";

	/*std::vector<uint32_t> program = {
		//SBL header
		0,
		('sblx'),
		0,
		(2048),
		0,
		0,
		(4096),
		0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,

		//Program image

		cast(Mnemonic::Mov_R_V), 0, 10,
		cast(Mnemonic::Mul_R_R), 0, 0,
		cast(Mnemonic::Print_R), 0, 0,
		//cast(Mnemonic::Mov_R_V), 65, 0,

		cast(Mnemonic::Mov_R_V), 10, 2200000,
		cast(Mnemonic::PNtvCall_V_V), 0, 10,

		cast(Mnemonic::Loop), 0, 0,
		//cast(Mnemonic::RCall_A), 3 * 2, 0,
		cast(Mnemonic::RCall_A), 3 * 6, 0,
		//cast(Mnemonic::NtvCall_V), 0, 0,
		cast(Mnemonic::Endloop), 0, 0,

		cast(Mnemonic::FpPrint_R), 0, 0,
		cast(Mnemonic::PrintC_V), '\n', 0,
		cast(Mnemonic::Print_R), 16, 0,
		cast(Mnemonic::PrintC_V), '\n', 0,
		cast(Mnemonic::End), 0, 0,

		//cast(Mnemonic::PNtvCall_V_V), 0, 10,
		//cast(Mnemonic::Ret), 0, 0,
		cast(Mnemonic::FpPi_R), 0, 0,
		cast(Mnemonic::FpE_R), 1, 0,
		cast(Mnemonic::FpMul_R_R), 0, 1,
		cast(Mnemonic::RCall_A), 3 * 1, 0,
		cast(Mnemonic::Ret), 0, 0,

		cast(Mnemonic::Add_R_V), 16, 104123,
		cast(Mnemonic::Xor_R_V), 16, 55472,
		cast(Mnemonic::Ret), 0, 0,

		cast(Mnemonic::RRegInt_V_A), cast(InterruptType::InstrCount), 9,
		cast(Mnemonic::ICountInt_V), 100000000, 0,

		//cast(Mnemonic::Nop), 0, 0,
		cast(Mnemonic::Mov_R_V), 11, 0,
		cast(Mnemonic::RJmp_A), cast(-3), 0,

		cast(Mnemonic::End), 0, 0,
	};
	*/
	
	///*
	constexpr uint32_t ToGenerate = 1000000;
	std::vector<uint32_t> programData = getGenerated(ToGenerate);
	computeCpp(programData, ToGenerate);
	std::cout << "-----------------\n";
	std::array<uint32_t, 64> header;
	header.fill(0);
	header[0] = 0;
	header[1] = 'sblx';
	header[2] = 0;	//Start addr, undecided
	header[3] = 4096;
	header[4] = 0;
	header[5] = (programData.size() - 6);
	header[6] = 4096;

	std::vector<uint32_t> program = {
		cast(Mnemonic::Mov_R_V), 16, 0,
		cast(Mnemonic::Mov_R_V), 22, cast(-1),
		cast(Mnemonic::Mov_R_V), 23, 0,

		cast(Mnemonic::Mov_R_V), 10, ToGenerate,
		cast(Mnemonic::Loop), 0, 0,

		cast(Mnemonic::Mov_R_I), 0, 16,
		cast(Mnemonic::Inc_R), 16, 0,
		cast(Mnemonic::Mov_R_I), 1, 16,
		cast(Mnemonic::Inc_R), 16, 0,

		cast(Mnemonic::RCall_A), 3 * 11, 0,
		cast(Mnemonic::Test_R_R), 0, 22,
		cast(Mnemonic::RJb_A), 6, 0,

		cast(Mnemonic::Mov_R_R), 22, 0,
		cast(Mnemonic::Mov_R_R), 23, 16,

		cast(Mnemonic::Endloop), 0, 0,
		cast(Mnemonic::Div_R_V), 23, 2,
		cast(Mnemonic::Dec_R), 23, 0,
		cast(Mnemonic::Print_R), 22, 0,
		cast(Mnemonic::PrintC_V), '\n', 0,
		cast(Mnemonic::Print_R), 23, 0,
		cast(Mnemonic::End), 0, 0,

		cast(Mnemonic::Sub_R_R), 0, 20,
		cast(Mnemonic::Sub_R_R), 1, 21,
		cast(Mnemonic::Mul_R_R), 0, 0,
		cast(Mnemonic::Mul_R_R), 1, 1,
		cast(Mnemonic::Add_R_R), 0, 1,
		cast(Mnemonic::Ret), 0, 0,
	};

	programData.insert(programData.begin(), header.begin(), header.end());
	programData.insert(programData.end(), program.begin(), program.end());
	programData.resize(4096 * ((programData.size() + 4095) / 4096));

	std::cin.get();

	while (1) {
		sbl::vm::VM vm;
		vm.run(programData);

		std::cout << "\nExecution total time: " << ch::duration_cast<ch::microseconds>(vm.getEndingTime() - vm.getStartingTime()).count() / 1000.f << "ms.\n";
		std::cout << "instructions executed: " << vm.totalExecuted() << "\n";
	}

	std::cin.get();
	exit(0);
	/*
	program.resize(sizeof(CompiledHeader) / sizeof(uint32_t) + 4096 - (program.size() % 4096) + program.size());

	while (1) {
		VM vm;
		vm.addNativeFunction("func", native_test, true);
		vm.run(program);

		auto execTimeNs = ch::duration_cast<ch::nanoseconds>(vm.getEndingTime() - vm.getStartingTime()).count();
		auto execInstrCount = vm.totalExecuted();

		if (vm.getError().code != sbl::vm::ErrorCode::None) {
			std::cout << vm.formatErrorCode() << ", last instruction pointer: " << vm.getError().instrPtr << "\n";
			std::cin.get();
			continue;
		}

		std::cout << "Execution took " << execTimeNs / 1000000.f << "ms.\n";
		continue;

		double mhz = 1000. / (execTimeNs * 1. / execInstrCount);
		int aboveDot = static_cast<int>(std::floor(mhz));
		int belowDot = int((mhz - aboveDot) * 1000);
		std::string belowStr = std::to_string(belowDot);
		switch (belowStr.size()) {
			case 0:
				belowStr += "0";
			case 1:
				belowStr += "0";
			case 2:
				belowStr += "0";
				break;
		}

		std::cout << "Executed " << beautify(std::to_string(execInstrCount))
			<< " instructions. Average speed of 100 runs: " << aboveDot << '.' << belowStr << " Mhz\n";
		std::cout << "On average it takes " << (4000.f / (aboveDot + belowDot / 1000.f)) << " cycles per operation.\n";

		std::cout << "Native calls: " << beautify(std::to_string(nativeCallCount)) << "\n";
		nativeCallCount = 0;
	}

	/*
	nearest();

	srand(time(nullptr));
	auto rnd = rand();
	rnd = rand();
	rnd = rand();
	rnd = rand();
	rnd = rand();
	rnd = rand();
	rnd = rand();
	rnd = rand();
	rnd = rand();
	rnd = rand();
	std::cout << std::hex << rnd << "\n";

	DummyJump jmp;
	jmp.exec(rnd);
	rnd = rand();
	jmp.exec(rnd);
	rnd = rand();
	jmp.exec(rnd);
	rnd = rand();
	jmp.exec(rnd);
	rnd = rand();
	jmp.exec(rnd);
	*/

	std::cin.get();
	return 0;

	/*
	VM vm;

	vm.addNativeFunction("func", native_test, false);
	vm.addNativeFunction("sqrt", native_test, false);
	vm.addNativeFunction("pow", native_test, false);
	vm.addNativeFunction("sin", native_test, false);

	vm.finalizeNatives();
	vm.removeNativeFunction("func");

	std::vector<uint32_t> instructionStream = {
	};


	instructionStream.resize(1000, 0);

	std::cout << "VM is of size " << sizeof(VM) << " bytes\n";

	std::vector<uint32_t> instrToInsert = {
		/*
		cast(Mnemonic::Push_V), 0, 0,
		cast(Mnemonic::Push_V), 0, 0,
		cast(Mnemonic::Push_V), cast(Mnemonic::IRet), 0,
		cast(Mnemonic::Mov_R_R), 0, 63,
		cast(Mnemonic::RHotpatch_A_I), 4 * 3, 0,
		cast(Mnemonic::Add_R_V), 63, 3,
		cast(Mnemonic::Mov_R_V), 0, 0,
		cast(Mnemonic::RJmp_A), 2 * 3, 0,

		cast(Mnemonic::SetPrivlg_V), 255, 0,
		cast(Mnemonic::Nop), 0, 0,

		cast(Mnemonic::RRegInt_V_A), 10, cast(-3 * 3),
		cast(Mnemonic::SetPrivlg_V), 1, 0,
		cast(Mnemonic::NtvCall_V), 0, 0,
		cast(Mnemonic::Raise_V), 10, 0,
		cast(Mnemonic::NtvCall_V), 0, 0,
		cast(Mnemonic::End), 0, 0,
		//*

		cast(Mnemonic::RRegInt_V_A), cast(InterruptType::InstrCount), 9,
		cast(Mnemonic::ICountInt_V), 500000000, 0,

		cast(Mnemonic::Mov_R_V), 11, 0,
		cast(Mnemonic::RJmp_A), cast(-6), 0,

		cast(Mnemonic::End), 0, 0,

		cast(Mnemonic::SetPrivlg_V), 255, 0,
		cast(Mnemonic::Raise_V), cast(InterruptType::InstrCount), 0,
		//cast(Mnemonic::Mov_R_V), 500, 1,
		cast(Mnemonic::NtvCall_V), 0, 0,
		//cast(Mnemonic::SetExtPrivlg_V_V), 0, 255,
		cast(Mnemonic::SetPrivlg_V), 100, 0,
		//cast(Mnemonic::RRegInt_V_A), 251, 3 * 8,
		//cast(Mnemonic::ICountInt_V), 1000000, 0,
		
		//cast(Mnemonic::FpPi_R), 15, 0,
		//cast(Mnemonic::FpPrint_R), 150, 0,
		//cast(Mnemonic::PrintC_V), '\n', 0,
		cast(Mnemonic::RPCall_A_V), 3, 10,
		cast(Mnemonic::End), 0, 0,

		cast(Mnemonic::Mov_R_V), 10, 500000,

		cast(Mnemonic::Loop), 0, 0,
		cast(Mnemonic::RCall_A), 3 * 4, 0,
		cast(Mnemonic::NtvCall_V), 0, 0,
		cast(Mnemonic::Endloop), 0, 0,
		cast(Mnemonic::Ret), 0, 0,

		cast(Mnemonic::End), 0, 0,

		cast(Mnemonic::Add_R_V), 1, 1,
		cast(Mnemonic::Ret), 0, 0,
		
		//cast(Mnemonic::RJmp_A), cast(-3), 0,

		cast(Mnemonic::End), 0, 0,


		cast(Mnemonic::SetInstrPrivlg_V_V), cast(Mnemonic::Mod_R_V), 1,
		cast(Mnemonic::RRegInt_V_A), cast(InterruptType::InsufficientPrivilege), 3,
		cast(Mnemonic::RJmp_A), 4 * 3, 0,

		cast(Mnemonic::IRet), 0, 0,

		//Unprivileged function
		cast(Mnemonic::Mov_R_V), 2, 10,
		cast(Mnemonic::Mod_R_V), 2, 3,
		//cast(Mnemonic::NtvCall_R), 2, 0,
		cast(Mnemonic::Ret), 0, 0,

		cast(Mnemonic::SetPrivlg_V), 200, 0,
		cast(Mnemonic::Mov_R_V), 10, 524288,
		cast(Mnemonic::Loop), 0, 0,
		cast(Mnemonic::RCall_A), cast(-7 * 3), 0,
		cast(Mnemonic::NtvCall_V), 1, 0,
		//cast(Mnemonic::RPCall_A_V), cast(-6 * 3), 100,
		cast(Mnemonic::Endloop), 0, 0,

		//cast(Mnemonic::RJmp_A), cast(-3), 0,

		cast(Mnemonic::End), 0, 0,

		//cast(Mnemonic::RICountInt_V), 3, 0,
		cast(Mnemonic::Mov_R_V), 10, 524288,

		cast(Mnemonic::Loop), 0, 0,
		cast(Mnemonic::Push_V), 1234, 0,
		cast(Mnemonic::Mov_R_V), 4, 256,
		cast(Mnemonic::Push_R), 4, 0,
		cast(Mnemonic::Pop_R), 5, 0,
		cast(Mnemonic::Pop_R), 6, 0,
		cast(Mnemonic::Mov_R_V), 11, 0,
		//cast(Mnemonic::NtvCall_V), 1, 0,
		cast(Mnemonic::Endloop), 0, 0,
	};

	instructionStream.insert(instructionStream.end(), instrToInsert.begin(), instrToInsert.end());
	instructionStream.push_back(cast(Mnemonic::End));
	instructionStream.push_back(0);
	instructionStream.push_back(0);

	auto beautify = [](const std::string& s) {
		std::string r;
		r.reserve(s.size() + s.size() / 3);

		auto over = s.size() % 3;
		auto ctr = 0;

		for (; over > 0; --over) {
			r += s[ctr++];
		}

		if (ctr)
			r += ',';

		for (size_t i = ctr; i < s.size(); i += 3) {
			r += s[i];
			r += s[i + 1];
			r += s[i + 2];
			r += ',';
		}

		if (r.back() == ',')
			r.pop_back();

		return r;
	};

	std::cout << "Length: " << beautify(std::to_string(instrToInsert.size())) << "\n";

	while (true) {
		std::vector<uint64_t> times;

		for (int i = 0; i < 1; ++i) {
			if (!vm.run(1000, instructionStream)) {
				std::cin.get();
				exit(0);
			}

			times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(vm.getEndingTime() - vm.getStartingTime()).count());
		}

		std::cout << "Native calls: " << beautify(std::to_string(nativeCallCount)) << "\n";
		nativeCallCount = 0;
			 
		auto execInstrCount = vm.totalExecuted();

		auto _avg = [](const std::vector<uint64_t>& v) {
			return std::accumulate(v.begin(), v.end(), 0ull) * 1. / v.size();
		};

		auto execTimeNs = _avg(times);

		//std::cout << beautify(std::to_string(execInstrCount)) << " instructions in " << (execTimeNs / 1000 * 1.) / 1000 << " ms.\n";
		//std::cout << "Average per instruction: " << (execTimeNs * 1. / execInstrCount) << "ns.\n";
		double mhz = 1000. / (execTimeNs * 1. / execInstrCount);
		int aboveDot = static_cast<int>(std::floor(mhz));
		int belowDot = int((mhz - aboveDot) * 1000);
		std::string belowStr = std::to_string(belowDot);
		switch (belowStr.size()) {
			case 0:
				belowStr += "0";
			case 1:
				belowStr += "0";
			case 2:
				belowStr += "0";
				break;
		}

		std::cout << "Executed " << beautify(std::to_string(execInstrCount))
			<< " instructions. Average speed of 100 runs: " << aboveDot << '.' << belowStr << " Mhz\n";
		std::cout << "On average it takes " << (4000.f / (aboveDot + belowDot / 1000.f)) << " cycles per operation.\n";
		//break;
	}

	std::cin.get();
	exit(0);
	//*/

	/*std::vector<uint8_t> instrStream = /*{
		192, cast(Mnemonic::Add_A_R), 0xFF, 0, 0, 0, 21,
		192, cast(Mnemonic::Eq_R_R), 10, 11,
		cast(Mnemonic::Loop),
		cast(Mnemonic::Pop_A), 0xaa, 0xbb, 0xcc, 0xdd,
		cast(Mnemonic::Pop_R), 14,
		cast(Mnemonic::End)
	};
	///
	{
		192, cast(Mnemonic::Add_A_R), 0, 0, 0xFF, 0, 0, 0, 21, 0, 0, 0,
		192, cast(Mnemonic::Eq_R_R), 0, 0, 10, 11, 0, 0, 0, 0, 0, 0,
		cast(Mnemonic::Loop), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		cast(Mnemonic::Pop_A), 0, 0, 0, 0xaa, 0xbb, 0xcc, 0xdd, 0, 0, 0, 0,
		cast(Mnemonic::Pop_R), 0, 0, 0, 14, 0, 0, 0, 0, 0, 0,
		cast(Mnemonic::End), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};//*

	auto to32 = [](uint8_t u1, uint8_t u2, uint8_t u3, uint8_t u4) -> uint32_t {
		return (u1 << 24) | (u2 << 16) | (u3 << 8) | u4;
	};

	auto mnemoTo32 = [](Mnemonic mn) {
		return (static_cast<uint16_t>(mn) << 16);
	};

	std::vector<uint32_t> instream = {
		cast(Mnemonic::Add_A_R), 0xFF000000, 0x15000000,
		cast(Mnemonic::Eq_R_R), 0xFF000000, 0x15000000,
		cast(Mnemonic::Loop), 0, 0,
		cast(Mnemonic::Pop_A), 0xddccbbaa, 0,
		cast(Mnemonic::Pop_R), 0xE000000, 0,
		cast(Mnemonic::Add_A_R), 0xFF000000, 0x15000000,
		cast(Mnemonic::Lt_I_R), 0, 0,
	};

	
	while (instream.size() < 2'000'000) {
		auto cpy = instream;
		instream.insert(instream.end(), cpy.begin(), cpy.end());
	}

	long long counter = 0;
	long long counterMax = 0;

	/*while (instrStream.size() < 20000) {
		auto cpy = instrStream;
		instrStream.insert(instrStream.end(), cpy.begin(), cpy.end());
	}//*

	Instruction m;
	uint32_t instrOffset = 0;

	std::vector<uint64_t> nanosPerf;

	Instruction* cached = nullptr;

	for (int i = 0; i < 1000; ++i) {
		auto start = std::chrono::high_resolution_clock::now().time_since_epoch();

		instrOffset = 0;
		counter = 0;

		while (instrOffset < instream.size()) {
			cached = m.fromStream(instream, instrOffset);
			//cached = m.fromStream(&instrStream[instrOffset]);
			instrOffset += 3;
			++counter;
		}

		counterMax += counter;

		auto end = std::chrono::high_resolution_clock::now().time_since_epoch() - start;
		auto ttl = std::chrono::duration_cast<std::chrono::nanoseconds>(end).count();
		nanosPerf.push_back(ttl);
	}

	std::sort(nanosPerf.begin(), nanosPerf.end());
	//nanosPerf.erase(nanosPerf.begin(), nanosPerf.begin() + 20);
	//nanosPerf.erase(nanosPerf.end() - 20, nanosPerf.end());

	auto ttl = nanosPerf[nanosPerf.size() / 2];
	auto ttlMin = nanosPerf[0];
	auto ttlMax = nanosPerf[nanosPerf.size() - 1];
	auto ttlCombined = std::accumulate(nanosPerf.begin(), nanosPerf.end(), 0ull);

	std::cout << (int)cast(cached->mnemonic) << "\n";

	std::cout << "Minimum:\n";
	std::cout << ttlMin / 1'000'000.f << "ms total time for " << counter << " instructions.\n";
	std::cout << "Time per instruction: " << ttlMin * 1.f / counter << "ns.\n";
	std::cout << "Decoding speed in Hz: " << 1000.f / (ttlMin * 1.f / counter) << "Mhz\n";

	std::cout << "Average:\n";
	std::cout << ttl / 1'000'000.f << "ms total time for " << counter << " instructions.\n";
	std::cout << "Time per instruction: " << ttl * 1.f / counter << "ns.\n";
	std::cout << "Decoding speed in Hz: " << 1000.f / (ttl * 1.f / counter) << "Mhz\n";

	std::cout << "Maximum:\n";
	std::cout << ttlMax / 1'000'000.f << "ms total time for " << counter << " instructions.\n";
	std::cout << "Time per instruction: " << ttlMax * 1.f / counter << "ns.\n";
	std::cout << "Decoding speed in Hz: " << 1000.f / (ttlMax * 1.f / counter) << "Mhz\n";

	std::cout << "\n\nTotal sum:\n";
	std::cout << ttlCombined / 1'000'000.f << "ms total time for " << counterMax << " instructions.\n";
	std::cout << "Time per instruction: " << ttlCombined * 1.f / counterMax << "ns.\n";
	std::cout << "Decoding speed in Hz: " << 1000.f / (ttlCombined * 1.f / counterMax) << "Mhz\n";

	std::cin.get();
	exit(0);

	/*__asm {
		push eax
		push ebx
		push ecx
		push edx

		push [eax]
		push [ebx]
		push [ecx]
		push [edx]

		push 45678901h
		push dword ptr ds:[0a1234567h]

		mov eax, dword ptr ds:[11223344h]
		add eax, 256
		mov dword ptr ds:[11223344h], eax
		pop eax
	}*/
	
}