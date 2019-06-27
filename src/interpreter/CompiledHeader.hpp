#pragma once

#ifndef COMPILED_HEADER_HEADER_H_
#define COMPILED_HEADER_HEADER_H_

#include <string_view>
#include <cstdint>

namespace sbl::vm {

	struct CompiledHeader {
		uint32_t signature;
		uint32_t startAddress;
		uint32_t stackSize;
		uint32_t padding[5];

		/*
			Visual Studio 2017 IntelliSense has an issue with this being default,
			so we have to provide dummy body
		 */
		constexpr CompiledHeader() noexcept : signature(0), startAddress(0), stackSize(0), padding() {}

		constexpr CompiledHeader(const CompiledHeader&) noexcept = default;
		constexpr CompiledHeader(CompiledHeader&&) noexcept = default;

		constexpr CompiledHeader& operator=(const CompiledHeader& other) noexcept {
			signature = other.signature;
			startAddress = other.startAddress;
			stackSize = other.stackSize;
			for (int i = 0; i < 5; ++i)  padding[i] = other.padding[i];
			return *this;
		}

		constexpr CompiledHeader& operator=(CompiledHeader&& other) noexcept {
			signature = other.signature;
			startAddress = other.startAddress;
			stackSize = other.stackSize;
			for (int i = 0; i < 5; ++i)  padding[i] = other.padding[i];
			return *this;
		}

		~CompiledHeader() noexcept = default;

		constexpr CompiledHeader(uint32_t _sig,
			uint32_t startAddr,
			uint32_t stack) : signature(_sig), startAddress(startAddr),
			stackSize(stack), padding() {}

		constexpr bool validate() const {
			return ((signature >> 24) & 0xff) == 's' &&
				((signature >> 16) & 0xff) == 'b' &&
				((signature >> 8) & 0xff) == 'l' &&
				((signature) & 0xff) == 'x';
		}

		constexpr void fromString(std::string_view view) {
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
		}
	};
}

#endif	//COMPILED_HEADER_HEADER_H_