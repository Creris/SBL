
#include <iostream>
#include <chrono>
#include <string_view>
#include <functional>
#include <numeric>
#include <iomanip>

#include <intrin.h>

#include "interpreter/VM.hpp"

int nativeCallCount = 0;

void native_test(sbl::vm::State& state) {
	++nativeCallCount;
}

constexpr auto qq() {
	using namespace sbl::vm::literals;

	char c[] = {
		's', 'b', 'l', 'x',
		0, 0, 0, 0x80_c,
		0, 0x80_c, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
	};

	sbl::vm::CompiledHeader header{};
	header.fromString(std::string_view{ c, 32 });

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

int main() {
	static auto cast = [](auto m) constexpr { return static_cast<uint32_t>(m); };

	///*
	sbl::vm::VM vm;

	vm.addNativeFunction("func", native_test, false);
	vm.addNativeFunction("sqrt", native_test, false);
	vm.addNativeFunction("pow", native_test, false);
	vm.addNativeFunction("sin", native_test, false);

	vm.finalizeNatives();
	vm.removeNativeFunction("func");

	std::vector<uint32_t> instructionStream = {
	};

	instructionStream.resize(1000, 0);

	std::cout << sizeof(sbl::vm::VM) << "\n";

	std::vector<uint32_t> instrToInsert = {
		cast(sbl::vm::Mnemonic::Push_V), '\0nis', 0,
		cast(sbl::vm::Mnemonic::GetNtvId_R_I), 0, 63,
		cast(sbl::vm::Mnemonic::Add_R_V), 63, 1,
		cast(sbl::vm::Mnemonic::PrintS_R), 0, 0,

		cast(sbl::vm::Mnemonic::NtvCall_V), 0, 0,
		cast(sbl::vm::Mnemonic::RRegInt_V_A), 251, 6,
		cast(sbl::vm::Mnemonic::ICountInt_V), 1000000, 0,
		cast(sbl::vm::Mnemonic::RJmp_A), cast(-3), 0,

		cast(sbl::vm::Mnemonic::End), 0, 0,

		//cast(sbl::vm::Mnemonic::RICountInt_V), 3, 0,
		cast(sbl::vm::Mnemonic::Mov_R_V), 10, 524288,

		cast(sbl::vm::Mnemonic::Loop), 0, 0,
		cast(sbl::vm::Mnemonic::Push_V), 1234, 0,
		cast(sbl::vm::Mnemonic::Mov_R_V), 4, 256,
		cast(sbl::vm::Mnemonic::Push_R), 4, 0,
		cast(sbl::vm::Mnemonic::Pop_R), 5, 0,
		cast(sbl::vm::Mnemonic::Pop_R), 6, 0,
		cast(sbl::vm::Mnemonic::Mov_R_V), 11, 0,
		//cast(sbl::vm::Mnemonic::NtvCall_V), 1, 0,
		cast(sbl::vm::Mnemonic::Endloop), 0, 0,
	};

	instructionStream.insert(instructionStream.end(), instrToInsert.begin(), instrToInsert.end());
	instructionStream.push_back(cast(sbl::vm::Mnemonic::End));
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

		for (int i = ctr; i < s.size(); i += 3) {
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

		for (int i = 0; i < 100; ++i) {
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
		//break;
	}

	std::cin.get();
	exit(0);
	//*/

	/*std::vector<uint8_t> instrStream = /*{
		192, cast(sbl::vm::Mnemonic::Add_A_R), 0xFF, 0, 0, 0, 21,
		192, cast(sbl::vm::Mnemonic::Eq_R_R), 10, 11,
		cast(sbl::vm::Mnemonic::Loop),
		cast(sbl::vm::Mnemonic::Pop_A), 0xaa, 0xbb, 0xcc, 0xdd,
		cast(sbl::vm::Mnemonic::Pop_R), 14,
		cast(sbl::vm::Mnemonic::End)
	};
	///
	{
		192, cast(sbl::vm::Mnemonic::Add_A_R), 0, 0, 0xFF, 0, 0, 0, 21, 0, 0, 0,
		192, cast(sbl::vm::Mnemonic::Eq_R_R), 0, 0, 10, 11, 0, 0, 0, 0, 0, 0,
		cast(sbl::vm::Mnemonic::Loop), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		cast(sbl::vm::Mnemonic::Pop_A), 0, 0, 0, 0xaa, 0xbb, 0xcc, 0xdd, 0, 0, 0, 0,
		cast(sbl::vm::Mnemonic::Pop_R), 0, 0, 0, 14, 0, 0, 0, 0, 0, 0,
		cast(sbl::vm::Mnemonic::End), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};*/

	auto to32 = [](uint8_t u1, uint8_t u2, uint8_t u3, uint8_t u4) -> uint32_t {
		return (u1 << 24) | (u2 << 16) | (u3 << 8) | u4;
	};

	auto mnemoTo32 = [](sbl::vm::Mnemonic mn) {
		return (static_cast<uint16_t>(mn) << 16);
	};

	std::vector<uint32_t> instream = {
		cast(sbl::vm::Mnemonic::Add_A_R), 0xFF000000, 0x15000000,
		cast(sbl::vm::Mnemonic::Eq_R_R), 0xFF000000, 0x15000000,
		cast(sbl::vm::Mnemonic::Loop), 0, 0,
		cast(sbl::vm::Mnemonic::Pop_A), 0xddccbbaa, 0,
		cast(sbl::vm::Mnemonic::Pop_R), 0xE000000, 0,
		cast(sbl::vm::Mnemonic::Add_A_R), 0xFF000000, 0x15000000,
		cast(sbl::vm::Mnemonic::Lt_I_R), 0, 0,
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
	}*/

	sbl::vm::Instruction m;
	uint32_t instrOffset = 0;

	std::vector<uint64_t> nanosPerf;

	sbl::vm::Instruction* cached = nullptr;

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