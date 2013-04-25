
#ifndef NOISE_20130206_H_
#define NOISE_20130206_H_

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include "LengthCounter.h"
#include "Timer.h"

using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;

class Noise {
public:
	Noise();
	~Noise();

public:
	void enable();
	void disable();

public:
	void write_reg0(uint8_t value);
	void write_reg2(uint8_t value);
	void write_reg3(uint8_t value);

public:
	bool enabled() const;
public:
	void tick();

public:
	LengthCounter &length_counter();

private:
	LengthCounter length_counter_;
	bool          enabled_;
	uint8_t       mode_;
	Timer         timer_;
};

#endif
