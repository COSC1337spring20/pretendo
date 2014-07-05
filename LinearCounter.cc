
#include "LinearCounter.h"

namespace nes {
namespace apu {

//------------------------------------------------------------------------------
// Name: LengthCounter
//------------------------------------------------------------------------------
LinearCounter::LinearCounter() : value_(0), control_(0), reload_(false)  {

}

//------------------------------------------------------------------------------
// Name: ~LengthCounter
//------------------------------------------------------------------------------
LinearCounter::~LinearCounter() {

}

//------------------------------------------------------------------------------
// Name: clock
//------------------------------------------------------------------------------
void LinearCounter::clock() {

	if(reload_) {
		value_ = (control_ & 0x7f);
	} else if(value_) {
		--value_;
	}

	if(!(control_ & 0x80)) {
		reload_ = false;
	}
}

//------------------------------------------------------------------------------
// Name: set_control
//------------------------------------------------------------------------------
void LinearCounter::set_control(uint8_t value) {
	control_ = value;
}

//------------------------------------------------------------------------------
// Name: reload
//------------------------------------------------------------------------------
void LinearCounter::reload() {
	reload_ = true;
}

//------------------------------------------------------------------------------
// Name: value
//------------------------------------------------------------------------------
uint8_t LinearCounter::value() const {
	return value_;
}

}
}
