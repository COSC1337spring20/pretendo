
#ifndef BITFIELD_H__20140702_H_
#define BITFIELD_H__20140702_H_

#include <cstdint>
#include <cstddef>
#include <type_traits>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

namespace {

template <size_t LastBit>
struct bitfield_helper {
    using type =
		typename std::conditional<LastBit == 0 , void,
		typename std::conditional<LastBit <= 8 , uint8_t,
		typename std::conditional<LastBit <= 16, uint16_t,
		typename std::conditional<LastBit <= 32, uint32_t,
		typename std::conditional<LastBit <= 64, uint64_t,
        void>::type>::type>::type>::type>::type;
};

}

template <size_t Index, size_t Bits = 1>
class bitfield {
private:
	enum {
		Mask = (1u << Bits) - 1u
	};

    using T = typename bitfield_helper<Index + Bits>::type;
public:
	template <class T2>
	bitfield &operator=(T2 value) {
		value_ = (value_ & ~(Mask << Index)) | ((value & Mask) << Index);
		return *this;
	}

	operator T() const             { return (value_ >> Index) & Mask; }
	explicit operator bool() const { return (value_ & (Mask << Index)) != 0; }
	bitfield &operator++()         { return *this = *this + 1; }
	T operator++(int)              { T r = *this; ++*this; return r; }
	bitfield &operator--()         { return *this = *this - 1; }
	T operator--(int)              { T r = *this; ++*this; return r; }

private:
	T value_;
};


template <size_t Index>
class bitfield<Index, 1> {
private:
	enum {
		Bits = 1,
		Mask = 0x01
	};

    using T = typename bitfield_helper<Index + Bits>::type;


public:
	bitfield &operator=(bool value) {
		value_ = (value_ & ~(Mask << Index)) | (value << Index);
		return *this;
	}

	operator bool() const  { return (value_ & (Mask << Index)) != 0; }
	bool operator!() const { return (value_ & (Mask << Index)) == 0; }
	
private:
	T value_;
};

#endif
