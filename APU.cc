
#include "APU.h"
#include "NES.h"
#include <iostream>
#include <cassert>

namespace {

const int frame_irq_delay = 29832;

enum {
	STATUS_DMC_IRQ  	   = 0x80,
	STATUS_FRAME_IRQ	   = 0x40,
	STATUS_ENABLE_DMC	   = 0x10,
	STATUS_ENABLE_NOISE    = 0x08,
	STATUS_ENABLE_TRIANGLE = 0x04,
	STATUS_ENABLE_SQUARE_2 = 0x02,
	STATUS_ENABLE_SQUARE_1 = 0x01,
	STATUS_ENABLE_ALL	   = STATUS_ENABLE_SQUARE_1 | STATUS_ENABLE_SQUARE_2 | STATUS_ENABLE_TRIANGLE | STATUS_ENABLE_NOISE | STATUS_ENABLE_DMC
};

const int FRAME_MODE		 = 0x80;
const int FRAME_INHIBIT_IRQ  = 0x40;

}

//------------------------------------------------------------------------------
// Name: APU
//------------------------------------------------------------------------------
APU::APU() : apu_cycles_(0), next_clock_(0), clock_step_(0), status_(0x00), frame_counter_(0x00), last_frame_counter_(0x00) {

}

//------------------------------------------------------------------------------
// Name: ~APU
//------------------------------------------------------------------------------
APU::~APU() {

}

//------------------------------------------------------------------------------
// Name: reset
//------------------------------------------------------------------------------
void APU::reset(nes::RESET reset_type) {

	status_        = 0;
	frame_counter_ = 0;
	apu_cycles_    = 0;
	next_clock_    = 0;
	clock_step_    = 0;

	if(reset_type == nes::HARD_RESET) {
		last_frame_counter_ = 0;
	}

	square_1.reset();
	square_2.reset();
	triangle_.reset();
	noise_.reset();
	dmc_.reset();

	write4017(last_frame_counter_);
	write4015(0x00);

	write4000(10);
	write4001(00);
	write4002(00);
	write4003(00);
	write4004(10);
	write4005(00);
	write4006(00);
	write4007(00);
	write4008(10);
//	write4009(00);
	write400A(00);
	write400B(00);
	write400C(10);
//	write400D(00);
	write400E(00);
	write400F(00);
	write4010(10);

	// TODO: why doesn't this make it pass? If I put 5 here, it does, but that
	//       doesn't seem correct. Blargg says it is as if this happens
	//
	//       lda   #$00
	//       sta   $4017       ; 1
	//       lda   <0          ; 9 delay
	//       nop
	//       nop
	//       nop
	//     reset:

	run(10);

	std::cout << "[APU::reset] reset complete" << std::endl;
}

//------------------------------------------------------------------------------
// Name: write4000
//------------------------------------------------------------------------------
void APU::write4000(uint8_t value) {
	square_1.write_reg0(value);
}

//------------------------------------------------------------------------------
// Name: write4001
//------------------------------------------------------------------------------
void APU::write4001(uint8_t value) {
	square_1.write_reg1(value);
}

//------------------------------------------------------------------------------
// Name: write4002
//------------------------------------------------------------------------------
void APU::write4002(uint8_t value) {
	square_1.write_reg2(value);
}

//------------------------------------------------------------------------------
// Name: write4003
//------------------------------------------------------------------------------
void APU::write4003(uint8_t value) {
	square_1.write_reg3(value);
}

//------------------------------------------------------------------------------
// Name: write4004
//------------------------------------------------------------------------------
void APU::write4004(uint8_t value) {
	square_2.write_reg0(value);
}

//------------------------------------------------------------------------------
// Name: write4005
//------------------------------------------------------------------------------
void APU::write4005(uint8_t value) {
	square_2.write_reg1(value);
}

//------------------------------------------------------------------------------
// Name: write4006
//------------------------------------------------------------------------------
void APU::write4006(uint8_t value) {
	square_2.write_reg2(value);
}

//------------------------------------------------------------------------------
// Name: write4007
//------------------------------------------------------------------------------
void APU::write4007(uint8_t value) {
	square_2.write_reg3(value);
}

//------------------------------------------------------------------------------
// Name: write4008
//------------------------------------------------------------------------------
void APU::write4008(uint8_t value) {
	triangle_.write_reg0(value);
}

//------------------------------------------------------------------------------
// Name: write400A
//------------------------------------------------------------------------------
void APU::write400A(uint8_t value) {
	triangle_.write_reg2(value);
}

//------------------------------------------------------------------------------
// Name: write400B
//------------------------------------------------------------------------------
void APU::write400B(uint8_t value) {
	triangle_.write_reg3(value);
}

//------------------------------------------------------------------------------
// Name: write400C
//------------------------------------------------------------------------------
void APU::write400C(uint8_t value) {
	noise_.write_reg0(value);
}

//------------------------------------------------------------------------------
// Name: write400E
//------------------------------------------------------------------------------
void APU::write400E(uint8_t value) {
	noise_.write_reg2(value);
}

//------------------------------------------------------------------------------
// Name: write400F
//------------------------------------------------------------------------------
void APU::write400F(uint8_t value) {
	noise_.write_reg3(value);
}

//------------------------------------------------------------------------------
// Name: write4010
//------------------------------------------------------------------------------
void APU::write4010(uint8_t value) {
	dmc_.write_reg0(value);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void APU::write4011(uint8_t value) {
	dmc_.write_reg1(value);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void APU::write4012(uint8_t value) {
	dmc_.write_reg2(value);
}

//------------------------------------------------------------------------------
// Name: write4013
//------------------------------------------------------------------------------
void APU::write4013(uint8_t value) {
	dmc_.write_reg3(value);
}

//------------------------------------------------------------------------------
// Name: write4015
//------------------------------------------------------------------------------
void APU::write4015(uint8_t value) {

	// Writing to this register clears the DMC interrupt flag.
	status_ &= ~STATUS_DMC_IRQ;

	if(value & STATUS_ENABLE_SQUARE_1) {
		square_1.enable();
	} else {
		square_1.disable();
	}

	if(value & STATUS_ENABLE_SQUARE_2) {
		square_2.enable();
	} else {
		square_2.disable();
	}

	if(value & STATUS_ENABLE_TRIANGLE) {
		triangle_.enable();
	} else {
		triangle_.disable();
	}

	if(value & STATUS_ENABLE_NOISE) {
		noise_.enable();
	} else {
		noise_.disable();
	}

	if(value & STATUS_ENABLE_DMC) {
		dmc_.enable();
	} else {
		dmc_.disable();
	}

	if(!(status_ & (STATUS_DMC_IRQ | STATUS_FRAME_IRQ))) {
		nes::cpu.clear_irq(CPU::APU_IRQ);
	}
}

//------------------------------------------------------------------------------
// Name: read4015
//------------------------------------------------------------------------------
uint8_t APU::read4015() {
	uint8_t ret = status_ & (STATUS_DMC_IRQ | STATUS_FRAME_IRQ);

	if(square_1.length_counter().value() > 0) {
		ret |= STATUS_ENABLE_SQUARE_1;
	}

	if(square_2.length_counter().value() > 0) {
		ret |= STATUS_ENABLE_SQUARE_2;
	}

	if(triangle_.length_counter().value() > 0) {
		ret |= STATUS_ENABLE_TRIANGLE;
	}

	if(noise_.length_counter().value() > 0) {
		ret |= STATUS_ENABLE_NOISE;
	}

	if(dmc_.sample_length() > 0) {
		ret |= STATUS_ENABLE_DMC;
	}

	// clear frame IRQ flag
	status_ &= ~STATUS_FRAME_IRQ;
	if(!(status_ & (STATUS_DMC_IRQ | STATUS_FRAME_IRQ))) {
		nes::cpu.clear_irq(CPU::APU_IRQ);
	}

	return ret;
}

//------------------------------------------------------------------------------
// Name: write4017
//------------------------------------------------------------------------------
void APU::write4017(uint8_t value) {

	frame_counter_      = value;
	last_frame_counter_ = value;

	if(frame_counter_ & FRAME_INHIBIT_IRQ) {
		status_ &= ~STATUS_FRAME_IRQ;
		if(!(status_ & (STATUS_DMC_IRQ | STATUS_FRAME_IRQ))) {
			nes::cpu.clear_irq(CPU::APU_IRQ);
		}
	}

	next_clock_ = apu_cycles_ + (apu_cycles_ & 1) + 1;
	clock_step_ = 0;

	if(!(frame_counter_ & FRAME_MODE)) {
		next_clock_ += 7458;
	}
}

//------------------------------------------------------------------------------
// Name: clock_length
//------------------------------------------------------------------------------
void APU::clock_length() {
	square_1.length_counter().clock();
	square_2.length_counter().clock();
	triangle_.length_counter().clock();
	noise_.length_counter().clock();
}

//------------------------------------------------------------------------------
// Name: clock_linear
//------------------------------------------------------------------------------
void APU::clock_linear() {

}

//------------------------------------------------------------------------------
// Name: clock_frame_mode_0
//------------------------------------------------------------------------------
void APU::clock_frame_mode_0() {

	// 4 step sequence
	switch(clock_step_) {
	case 0:
		clock_linear();
		next_clock_ += 7456;
		break;

	case 1:
		clock_linear();
		clock_length();
		next_clock_ += 7458;
		break;

	case 2:
		clock_linear();
		clock_linear();
		next_clock_ += 7457;
		break;

	case 3:
		if(!(frame_counter_ & FRAME_INHIBIT_IRQ)) {
			status_ |= STATUS_FRAME_IRQ;
		}

		++next_clock_;
		break;

	case 4:
		clock_linear();
		clock_length();
		if(!(frame_counter_ & FRAME_INHIBIT_IRQ)) {
			status_ |= STATUS_FRAME_IRQ;
		}

		++next_clock_;
		break;

	case 5:
		if(!(frame_counter_ & FRAME_INHIBIT_IRQ)) {
			status_ |= STATUS_FRAME_IRQ;
		}

		next_clock_ += 7457;
		break;
	}

	clock_step_ = (clock_step_ + 1) % 6;
}

//------------------------------------------------------------------------------
// Name: clock_frame_mode_1
//------------------------------------------------------------------------------
void APU::clock_frame_mode_1() {

	// 5 step sequence
	switch(clock_step_) {
	case 0:
		clock_length();
		next_clock_ += 7458;
		break;

	case 1:
		next_clock_ += 7456;
		break;

	case 2:
		clock_length();
		next_clock_ += 7458;
		break;

	case 3:
		next_clock_ += 7456;
		break;

	case 4:
		next_clock_ += 7454;
		break;
	}

	clock_step_ = (clock_step_ + 1) % 5;
}

//------------------------------------------------------------------------------
// Name: run
//------------------------------------------------------------------------------
void APU::run(int cycles) {

	while(cycles-- > 0) {

		if(!(frame_counter_ & FRAME_INHIBIT_IRQ) && (status_ & STATUS_FRAME_IRQ)) {
			nes::cpu.irq(CPU::APU_IRQ);
		}

		if(apu_cycles_ == next_clock_) {
			if(frame_counter_ & FRAME_MODE) {
				clock_frame_mode_1();
			} else {
				clock_frame_mode_0();
			}
		}


		triangle_.tick();

		if((apu_cycles_ % 2) == 0) {
			square_1.tick();
			square_2.tick();
			noise_.tick();
			dmc_.tick();
		}

		++apu_cycles_;
	}
}
