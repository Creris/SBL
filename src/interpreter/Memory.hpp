#pragma once

#ifndef INTERPRETER_MEMORY_HEADER_H_
#define INTERPRETER_MEMORY_HEADER_H_

#include <vector>

namespace sbl::vm {
	enum class SegmentAccessType : uint8_t {
		None = 0,
		Writable = 1,
		Readable = 1 << 1,
		Executable = 1 << 2,
	};

	inline SegmentAccessType operator|(SegmentAccessType left, SegmentAccessType right) {
		return static_cast<SegmentAccessType>(static_cast<uint8_t>(left) | static_cast<uint8_t>(right));
	}

	inline SegmentAccessType& operator|=(SegmentAccessType& left, SegmentAccessType right) {
		left = left | right;
		return left;
	}

	inline SegmentAccessType operator&(SegmentAccessType left, SegmentAccessType right) {
		return static_cast<SegmentAccessType>(static_cast<uint8_t>(left) & static_cast<uint8_t>(right));
	}

	inline SegmentAccessType& operator&=(SegmentAccessType& left, SegmentAccessType right) {
		left = left & right;
		return left;
	}

	inline SegmentAccessType operator^(SegmentAccessType left, SegmentAccessType right) {
		return static_cast<SegmentAccessType>(static_cast<uint8_t>(left) ^ static_cast<uint8_t>(right));
	}

	inline SegmentAccessType& operator^=(SegmentAccessType& left, SegmentAccessType right) {
		left = left ^ right;
		return left;
	}

	template <uint32_t SegmentSize = 4096>
	class Memory {
	public:
		template <class T>
		using Observer = T*;

		struct SegmentInformation {
			SegmentAccessType segmentAccessFlags = SegmentAccessType::Readable;
		};
	private:
		std::vector<uint32_t> memory;
		std::vector<SegmentInformation> segmentInfo;
		
		Observer<uint32_t> _getSegmentMemAddr(size_t segId) noexcept {
			if (segId > getSegmentCount())
				return nullptr;
			return &memory[0] + segId * SegmentSize;
		}

		Observer<const uint32_t> _getSegmentMemAddr(size_t segId) const noexcept {
			if (segId > getSegmentCount())
				return nullptr;
			return &memory[0] + segId * SegmentSize;
		}

		Observer<uint32_t> _tryAccess_nocheck(size_t atMemory, SegmentAccessType accessRequest) {
			//if (accessRequest == SegmentAccessType::None)
			//	return nullptr;
			auto segId = _segmentId_nocheck(atMemory);
			auto& access = segmentInfo[segId].segmentAccessFlags;
			if ((access & accessRequest) != accessRequest)
				return nullptr;

			return &memory[0] + atMemory;
		}

		Observer<const uint32_t> _tryAccess_nocheck(size_t atMemory, SegmentAccessType accessRequest) const {
			//if (accessRequest == SegmentAccessType::None)
			//	return nullptr;
			auto segId = _segmentId_nocheck(atMemory);
			auto& access = segmentInfo[segId].segmentAccessFlags;
			if ((access & accessRequest) != accessRequest)
				return nullptr;

			return &memory[0] + atMemory;
		}

		Observer<uint32_t> _tryAccessSegment_nocheck(size_t segmentId, SegmentAccessType accessRequest) {
			//if (accessRequest == SegmentAccessType::None)
			//	return nullptr;
			auto atMemory = _segmentOffset_nocheck(segmentId);
			auto& access = segmentInfo[segmentId].segmentAccessFlags;
			if ((access & accessRequest) != accessRequest)
				return nullptr;

			return &memory[0] + atMemory;
		}

		Observer<const uint32_t> _tryAccessSegment_nocheck(size_t segmentId, SegmentAccessType accessRequest) const {
			//if (accessRequest == SegmentAccessType::None)
			//	return nullptr;
			auto atMemory = _segmentOffset_nocheck(segmentId);
			auto& access = segmentInfo[segmentId].segmentAccessFlags;
			if ((access & accessRequest) != accessRequest)
				return nullptr;

			return &memory[0] + atMemory;
		}

		int32_t _segmentOffset_nocheck(size_t segmentId) const noexcept {
			return static_cast<int32_t>(segmentId * SegmentSize);
		}

		int32_t _segmentId_nocheck(size_t memOffset) const noexcept {
			return static_cast<int32_t>(memOffset / SegmentSize);
		}
	public:
		Memory() {}
		
		Memory(const Memory&) = delete;
		Memory& operator=(const Memory&) = delete;

		Memory(Memory&&) = delete;
		Memory& operator=(Memory&&) = delete;

		constexpr size_t getSegmentSize() const noexcept {
			return SegmentSize;
		}

		size_t getSegmentCount() const noexcept {
			return segmentInfo.size();
		}

		void clear() {
			memory.clear();
			segmentInfo.clear();
		}

		void clear(size_t newSegmentSize) {
			memory.clear();
			segmentInfo.clear();
		}

		bool addSegment(SegmentAccessType defaultAccess) {
			try {
				memory.insert(memory.end(), SegmentSize, 0);
				segmentInfo.push_back({ defaultAccess });
			} catch (...) {
				return false;
			}

			return true;
		}

		bool addSegment(SegmentAccessType defaultAccess, Observer<const uint32_t> initValueArray) {
			try {
				memory.insert(memory.end(), initValueArray, initValueArray + SegmentSize);
				segmentInfo.push_back({ defaultAccess });
			} catch (...) {
				return false;
			}
			
			return true;
		}

		bool addSegments(size_t count, SegmentAccessType defaultAccess) {
			try {
				memory.insert(memory.end(), SegmentSize * count, 0);
				segmentInfo.insert(segmentInfo.end(), count, { defaultAccess });
			} catch (...) {
				return false;
			}

			return true;
		}

		bool addSegments(size_t count, SegmentAccessType defaultAccess, Observer<const uint32_t> initValueArray) {
			for (size_t i = 0; i < count; ++i) {
				if (!addSegment(defaultAccess, initValueArray + SegmentSize * i))
					return false;
			}
			return true;
		}

		int32_t segmentIdx(size_t atMemOffset) const noexcept {
			if (atMemOffset > memory.size())
				return -1;
			return static_cast<int32_t>(atMemOffset / SegmentSize);
		}

		int32_t segmentOffset(size_t segmentId) const noexcept {
			if (segmentId > getSegmentCount())
				return -1;
			return static_cast<int32_t>(segmentId * SegmentSize);
		}


		Observer<uint32_t> tryAccess(size_t atMemory, SegmentAccessType accessRequest) {
			//if (accessRequest == SegmentAccessType::None)
			//	return nullptr;
			if (atMemory >= memory.size() || (segmentInfo[atMemory / SegmentSize].segmentAccessFlags & accessRequest) != accessRequest)
				return nullptr;
			/*
			auto& access = segmentInfo[atMemory / SegmentSize].segmentAccessFlags;
			if ((access & accessRequest) != accessRequest)
				return nullptr;
			*/
			return &memory[0] + atMemory;
		}

		Observer<const uint32_t> tryAccess(size_t atMemory, SegmentAccessType accessRequest) const {
			//if (accessRequest == SegmentAccessType::None)
			//	return nullptr;
			auto segId = segmentIdx(atMemory);
			if (segId == -1)
				return nullptr;
			auto& access = segmentInfo[segId].segmentAccessFlags;
			if ((access & accessRequest) != accessRequest)
				return nullptr;

			return &memory[0] + atMemory;
		}


		Observer<uint32_t> tryAccessRange(size_t memOffset, size_t length, SegmentAccessType accessRequest) {
			//if (accessRequest == SegmentAccessType::None)
			//	return nullptr;
			auto firstIdx = segmentIdx(memOffset);
			auto lastIdx = segmentIdx(memOffset + length - 1);
			if (firstIdx == -1 || lastIdx == -1)
				return nullptr;
			auto ptr = _tryAccess_nocheck(memOffset, accessRequest);
			if (!ptr)	return nullptr;

			//the access for first segment is already verified by the call above
			++firstIdx;

			for (; firstIdx <= lastIdx; ++firstIdx) {
				if (!_tryAccess_nocheck(_segmentOffset_nocheck(firstIdx), accessRequest))
					return nullptr;
			}

			return ptr;
		}

		Observer<const uint32_t> tryAccessRange(size_t memOffset, size_t length, SegmentAccessType accessRequest) const {
			//if (accessRequest == SegmentAccessType::None)
			//	return nullptr;
			auto firstIdx = segmentIdx(memOffset);
			auto lastIdx = segmentIdx(memOffset + length - 1);
			if (firstIdx == -1 || lastIdx == -1)
				return nullptr;
			auto ptr = _tryAccess_nocheck(memOffset, accessRequest);
			if (!ptr)	return nullptr;

			//the access for first segment is already verified by the call above
			++firstIdx;

			for (; firstIdx <= lastIdx; ++firstIdx) {
				if (!_tryAccess_nocheck(firstIdx, accessRequest))
					return nullptr;
			}

			return ptr;
		}


		Observer<uint32_t> tryAccessSegment(size_t segmentId, SegmentAccessType accessRequest) {
			if (segmentId > getSegmentCount())
				return false;
			return _tryAccessSegment_nocheck(segmentId, accessRequest);
		}

		Observer<uint32_t> tryAccessSegmentRange(size_t segmentId, size_t length, SegmentAccessType accessRequest) {
			//if (accessRequest == SegmentAccessType::None)
			//	return nullptr;
			if (segmentId > getSegmentCount())	return nullptr;

			auto memOffset = _segmentOffset_nocheck(segmentId);
			auto lastIdx = segmentIdx(memOffset + length - 1);
			if (lastIdx == -1)
				return nullptr;
			auto ptr = _tryAccess_nocheck(memOffset, accessRequest);
			if (!ptr)	return nullptr;

			//the access for first segment is already verified by the call above
			++segmentId;

			for (; segmentId <= lastIdx; ++segmentId) {
				if (!_tryAccessSegment_nocheck(segmentId, accessRequest))
					return nullptr;
			}

			return ptr;
		}

		Observer<const uint32_t> tryAccessSegmentRange(size_t segmentId, size_t length, SegmentAccessType accessRequest) const {
			//if (accessRequest == SegmentAccessType::None)
			//	return nullptr;
			if (segmentId > getSegmentCount())	return nullptr;

			auto memOffset = _segmentOffset_nocheck(segmentId);
			auto lastIdx = segmentIdx(memOffset + length - 1);
			if (lastIdx == -1)
				return nullptr;
			auto ptr = _tryAccess_nocheck(memOffset, accessRequest);
			if (!ptr)	return nullptr;

			//the access for first segment is already verified by the call above
			++segmentId;

			for (; segmentId <= lastIdx; ++segmentId) {
				if (!_tryAccessSegment_nocheck(segmentId, accessRequest))
					return nullptr;
			}

			return ptr;
		}

		bool tryWrite(size_t memOffset, uint32_t value) {
			auto ptr = tryAccess(memOffset, SegmentAccessType::Writable);
			if (!ptr)	return false;
			*ptr = value;
			return true;
		}

		bool tryWrite(size_t memOffset, uint32_t count, uint32_t* sequence) {
			auto ptr = tryAccessRange(memOffset, count, SegmentAccessType::Writable);
			if (!ptr)	return false;
			auto _writeN = [](uint32_t* to, uint32_t count, uint32_t* from) {
				while (count--)
					*(to++) = *(from)++;
			};
			_writeN(ptr, count, sequence);
			return true;
		}

		std::pair<bool, uint32_t> tryRead(size_t memOffset) {
			auto ptr = tryAccess(memOffset, SegmentAccessType::Readable);
			if (!ptr)	return { false, 0 };
			return { true, *ptr };
		}

		Observer<uint32_t> tryExecute(size_t memOffset) {
			return tryAccess(memOffset, SegmentAccessType::Executable);
		}

		size_t roundToSegmentBase(size_t size) const {
			return (size + (SegmentSize - 1)) / SegmentSize;
		}

		SegmentAccessType getSegmentAccess(uint32_t segmentId) const {
			if (segmentId > getSegmentCount())
				return SegmentAccessType::None;
			return segmentInfo[segmentId].segmentAccessFlags;
		}

		void setSegmentAccess(uint32_t segmentId, SegmentAccessType newAccess) {
			if (segmentId > getSegmentCount())
				return;
			segmentInfo[segmentId].segmentAccessFlags = newAccess;
		}

		const Observer<const uint32_t> baseAddress() const {
			return &memory[0];
		}

		uint32_t& _getNocheck(size_t addr) {
			return memory[addr];
		}

		const uint32_t& _getNocheck(size_t addr) const {
			return memory[addr];
		}
	};
}

#endif	//INTERPRETER_MEMORY_HEADER_H_
