#pragma once

#ifndef INTERPRETER_COMPILED_HEADER_HEADER_H_
#define INTERPRETER_COMPILED_HEADER_HEADER_H_

#include <string_view>
#include <cstdint>
#include <cmath>

namespace sbl::vm {
	constexpr uint32_t minOf(uint32_t one, uint32_t last) {
		return one < last ? one : last;
	}

	template <class... T>
	constexpr uint32_t minOf(uint32_t first, T&&... tail) {
		static_assert(std::conjunction_v<std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, uint32_t>...>, "All parameters must be uint32_t.");

		uint32_t rightMin = minOf(tail...);
		return first < rightMin ? first : rightMin;
	}

	struct CompiledHeader {
		uint32_t version;			//Version of the VM, currently required to be set to 0
		uint32_t signature;			//Fixed signature identifying this to be SBL bytecode file
		uint32_t startAddress;		//The address to start executing at, relative to the start of first Program Segment
									//For segmenting, see Memory.hpp
		uint32_t stackSize;			//Number of bytes that the stack will take
		uint32_t heapPtrCount;		//Number of bytes that the heap block will take
		uint32_t staticBlockSize;	//Number of bytes the globals that are not executable but still part of the
									//program image will take
		uint32_t programSize;		//Number of bytes the executable portion of the program image will take

		std::array<uint32_t, 64 - 7> padding;
									//Padding for up to 64 uint32_ts. Reserved for future use
									//All of these have to be set to 0

		/*
			Visual Studio 2017 IntelliSense has an issue with this being default,
			so we have to provide dummy body
		 */
		constexpr CompiledHeader() noexcept : version(0), signature(0), startAddress(0), stackSize(0),
												heapPtrCount(0), staticBlockSize(0), 
												programSize(0), padding() {}

		constexpr CompiledHeader(const CompiledHeader&) noexcept = default;
		constexpr CompiledHeader(CompiledHeader&&) noexcept = default;

		constexpr CompiledHeader& operator=(const CompiledHeader& other) noexcept = default;
		constexpr CompiledHeader& operator=(CompiledHeader&& other) noexcept = default;

		~CompiledHeader() noexcept = default;

		constexpr CompiledHeader(uint32_t ver, uint32_t _sig,
			uint32_t startAddr,
			uint32_t stack, uint32_t globals, uint32_t heap,
			uint32_t blockSize, uint32_t program_size)
				: version(ver), signature(_sig), startAddress(startAddr),
					stackSize(stack), heapPtrCount(heap),
					staticBlockSize(blockSize), programSize(program_size), padding() {}

		constexpr bool _allPadding(uint32_t initValue) const {
			for (auto& x : padding)
				if (x != initValue)	return false;
			return true;
		}

		constexpr bool validate() const {
			return signature == 'sblx' && version == 0 &&
				_allPadding(0);
		}

		void fromStream(const std::vector<uint32_t>& stream) {
			if (stream.size() < sizeof(*this))
				return;

			version = stream[0];
			signature = stream[1];
			startAddress = stream[2];
			stackSize = stream[3];
			heapPtrCount = stream[4];
			staticBlockSize = stream[5];
			programSize = stream[6];

			for (int i = 0; i < padding.size(); ++i) {
				padding[i] = stream[i + 7];
			}


			//If the total size of the staticm memory block and program block is bigger
			//than the filesize, invalidate header
			if (static_cast<uint64_t>(programSize + staticBlockSize) > stream.size()) {
				signature = 0;
				return;
			}

			//Globals storage cant be bigger than 16MB
			staticBlockSize = std::min(staticBlockSize, 16ui32 * 1024ui32 * 1024ui32);

			//Program cant be bigger than 16MB
			programSize = std::min(programSize, std::min(16ui32 * 1024ui32 * 1024ui32, (uint32_t)stream.size() - 64));

			//Stack size cannot be bigger than 15MB
			stackSize = std::min(stackSize, 15ui32 * 1024ui32 * 1024ui32);

			//Heap cant store more than 1MB worth of pointers to dynamic memory
			heapPtrCount = std::min(1024ui32 * 1024ui32, heapPtrCount);

			//The combined maximum size of heap and stack is 16MB

			//Combined total memory of the entire program is therefore <= 48MB


			//If the address is out of range of the program, early quit
			//even tho this would've been caught by the interpreter when
			//you try to execute read/write memory with no exec access right
			if (startAddress > programSize)
				signature = 0;
		}

		constexpr void fromString(std::string_view view, uint32_t fileSize) {
			if (view.size() < sizeof(*this))
				return;

			auto unsign = [](const char v) -> uint8_t { return static_cast<uint8_t>(v); };
			auto oneByte = [unsign](const auto v, const auto shift) -> uint32_t {
				return (unsign(v) & 0xff) << shift;
			};

			signature = oneByte(view[0], 24) | oneByte(view[1], 16) | oneByte(view[2], 8) | oneByte(view[3], 0);
			view.remove_prefix(4);
			startAddress = oneByte(view[0], 24) | oneByte(view[1], 16) | oneByte(view[2], 8) | oneByte(view[3], 0);
			view.remove_prefix(4);
			stackSize = oneByte(view[0], 24) | oneByte(view[1], 16) | oneByte(view[2], 8) | oneByte(view[3], 0);
			view.remove_prefix(4);
			heapPtrCount = oneByte(view[0], 24) | oneByte(view[1], 16) | oneByte(view[2], 8) | oneByte(view[3], 0);
			view.remove_prefix(4);
			staticBlockSize = oneByte(view[0], 24) | oneByte(view[1], 16) | oneByte(view[2], 8) | oneByte(view[3], 0);
			view.remove_prefix(4);
			programSize = oneByte(view[0], 24) | oneByte(view[1], 16) | oneByte(view[2], 8) | oneByte(view[3], 0);


			//If the total size of the staticm memory block and program block is bigger
			//than the filesize, invalidate header
			if (static_cast<uint64_t>(programSize + staticBlockSize) > static_cast<uint64_t>(fileSize)) {
				signature = 0;
				return;
			}

			//Globals storage cant be bigger than 16MB
			staticBlockSize = std::min(staticBlockSize, 16ui32 * 1024ui32 * 1024ui32);

			//Program cant be bigger than 16MB
			programSize = std::min(programSize, std::min(16ui32 * 1024ui32 * 1024ui32, fileSize));

			//Stack size cannot be bigger than 15MB
			stackSize = std::min(stackSize, 15ui32 * 1024ui32 * 1024ui32);

			//Heap cant store more than 1MB worth of pointers to dynamic memory
			heapPtrCount = std::min(1024ui32 * 1024ui32, heapPtrCount);

			//The combined maximum size of heap and stack is 16MB

			//Combined total memory of the entire program is therefore <= 48MB


			//If the address is out of range of the program, early quit
			//even tho this would've been caught by the interpreter when
			//you try to execute read/write memory with no exec access right
			if (startAddress > programSize)
				signature = 0;
		}
	};
}

#endif	//INTERPRETER_COMPILED_HEADER_HEADER_H_4