
#ifndef TRIANGLE_20130206_H_
#define TRIANGLE_20130206_H_

#include "LengthCounter.h"
#include "LinearCounter.h"
#include "Timer.h"
#include <cstddef>
#include <cstdint>

namespace nes {
namespace apu {

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

class Triangle {
public:
    Triangle()                            = default;
    ~Triangle()                           = default;
    Triangle(const Triangle &)            = delete;
	Triangle &operator=(const Triangle &) = delete;

public:
	void enable();
	void disable();
	void set_enabled(bool value);

public:
	void write_reg0(uint8_t value);
	void write_reg2(uint8_t value);
	void write_reg3(uint8_t value);

public:
	bool enabled() const;

public:
	LengthCounter length_counter;
	LinearCounter linear_counter;

public:
	void tick();
	uint8_t output() const;

private:
    bool          enabled_        = false;
    uint16_t      timer_load_     = 0;
    size_t        sequence_index_ = 0;
	Timer         timer_;
};

}
}

#endif
