#pragma once

#ifndef COMMON_FIXED_VECTOR_HEADER_H_
#define COMMON_FIXED_VECTOR_HEADER_H_

#include <memory>
#include <array>

namespace sbl::cmn {
	template <class T, size_t Size>
	class FixedVector {
		std::unique_ptr<std::array<T, Size>> arrPtr;
	public:
		~FixedVector() noexcept = default;

		FixedVector() {
			arrPtr = std::make_unique<std::array<T, Size>>();
		}
		
		FixedVector(const FixedVector& v) {
			arrPtr = std::make_unique<std::array<T, Size>>();
			for (size_t i = 0; i < v.size(); ++i) {
				(*arrPtr)[i] = v[i];
			}
			return *this;
		}

		FixedVector& operator=(const FixedVector& v) {
			arrPtr = std::make_unique<std::array<T, Size>>();
			for (size_t i = 0; i < v.size(); ++i) {
				(*arrPtr)[i] = v[i];
			}
			return *this;
		}

		FixedVector(FixedVector&& v) noexcept = default;
		FixedVector& operator=(FixedVector&& v) noexcept = default;

		size_t size() const {
			return arrPtr->size();
		}

		const T& operator[](size_t idx) const {
			return (*arrPtr)[idx];
		}

		T& operator[](size_t idx) {
			return (*arrPtr)[idx];
		}

		const T& at(size_t idx) const {
			return arrPtr.get()->at(idx);
		}

		T& at(size_t idx) {
			return arrPtr.get()->at(idx);
		}

		auto begin() {
			return arrPtr->begin();
		}

		auto end() {
			return arrPtr->end();
		}

		auto cbegin() const {
			return arrPtr->cbegin();
		}

		auto cend() const {
			return arrPtr->cend();
		}

		auto rbegin() {
			return arrPtr->rbegin();
		}

		auto rend() {
			return arrPtr->rend();
		}

		auto crbegin() const {
			return arrPtr->crbegin();
		}

		auto crend() const {
			return arrPtr->crend();
		}

		const T* data() const {
			return arrPtr->data();
		}

		T* data() {
			return arrPtr->data();
		}

		void fill(const T& v) {
			for (auto& x : *this) {
				x = v;
			}
		}
	};
}

#endif	//COMMON_FIXED_VECTOR_HEADER_H_