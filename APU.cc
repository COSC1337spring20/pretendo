
#include "APU.h"
#include "CPU.h"
#include "DMC.h"
#include "Noise.h"
#include "Square.h"
#include "Triangle.h"
#include <algorithm>
#include <iostream>

namespace nes {
namespace apu {
namespace {

enum {
	STATUS_DMC_IRQ  	   = 0x80,
	STATUS_FRAME_IRQ	   = 0x40,
	STATUS_ENABLE_DMC	   = 0x10,
	STATUS_ENABLE_NOISE    = 0x08,
	STATUS_ENABLE_TRIANGLE = 0x04,
	STATUS_ENABLE_SQUARE_2 = 0x02,
	STATUS_ENABLE_SQUARE_1 = 0x01,
};

union APUFrameCounter {
	uint8_t raw;
	BitField<6> inihibit_frame_irq;
	BitField<7> mode;

};

//------------------------------------------------------------------------------
// Name: bound
//------------------------------------------------------------------------------
template<class T>
const T &bound(const T &lower, const T &value, const T &upper) {
	using std::min;
	using std::max;

	return max(lower, min(value, upper));
}


const double CPUFrequency  = 1789772.67; // 1.78977267Mhz
// CPUFrequency / 44100Hz  = 40.5844142857 clocks per sample
// CPUFrequency / 48000Hz  = 37.286930625  clocks per sample
// CPUFrequency / 192000Hz = 9.32173265625 clocks per sample
const int ClocksPerSample = CPUFrequency / frequency;

}

Square<0> square_0;
Square<1> square_1;
Triangle  triangle;
Noise     noise;
DMC       dmc;

uint64_t        apu_cycles_         = -1;
uint64_t        next_clock_         = -1;
uint8_t         clock_step_         = 0;
APUStatus       status              = {0};
APUFrameCounter frame_counter_      = {0};
uint8_t         last_frame_counter_ = 0;
size_t          sample_index_       = 0;
uint8_t         sample_buffer_[buffer_size];

void clock_frame_mode_0();
void clock_frame_mode_1();
void clock_length();
void clock_linear();
uint8_t mix_channels();

//------------------------------------------------------------------------------
// Name: reset
//------------------------------------------------------------------------------
void reset(RESET reset_type) {

	status.raw     = 0;
	frame_counter_ = {0};
	apu_cycles_    = 0;
	next_clock_    = 0;
	clock_step_    = 0;

	if(reset_type == HARD_RESET) {
		last_frame_counter_ = 0;
	}

	write4017(last_frame_counter_);
	write4015(0x00);

	// square 1
	write4000(10);
	write4001(00);
	write4002(00);
	write4003(00);

	// square 2
	write4004(10);
	write4005(00);
	write4006(00);
	write4007(00);

	// triangle
//	write4008(10); // triangle is unaffected..
	write400A(00);
	write400B(00);

	// noise
	write400C(10);
	write400E(00);
	write400F(00);

	// dmc
	write4010(10);

	// OK, the APU is supposed to act as if it has run for approximately 9
	// cycles by the time the reset is complete. I beleive that the first 7
	// of these cycles are the 7 cycles of the reset itself. So we run the
	// APU manually for an extra 2 ticks.
	//
	// Blargg says it is as if this happens
	//
	//       lda   #$00
	//       sta   $4017       ; 1
	//       lda   <0          ; 9 delay
	//       nop
	//       nop
	//       nop
	//     reset:
	if(reset_type == HARD_RESET) {
		run(2);
	}

	std::cout << "APU reset complete" << std::endl;
}

//------------------------------------------------------------------------------
// Name: write4000
//------------------------------------------------------------------------------
void write4000(uint8_t value) {
	square_0.write_reg0(value);
}

//------------------------------------------------------------------------------
// Name: write4001
//------------------------------------------------------------------------------
void write4001(uint8_t value) {
	square_0.write_reg1(value);
}

//------------------------------------------------------------------------------
// Name: write4002
//------------------------------------------------------------------------------
void write4002(uint8_t value) {
	square_0.write_reg2(value);
}

//------------------------------------------------------------------------------
// Name: write4003
//------------------------------------------------------------------------------
void write4003(uint8_t value) {
	square_0.write_reg3(value);
}

//------------------------------------------------------------------------------
// Name: write4004
//------------------------------------------------------------------------------
void write4004(uint8_t value) {
	square_1.write_reg0(value);
}

//------------------------------------------------------------------------------
// Name: write4005
//------------------------------------------------------------------------------
void write4005(uint8_t value) {
	square_1.write_reg1(value);
}

//------------------------------------------------------------------------------
// Name: write4006
//------------------------------------------------------------------------------
void write4006(uint8_t value) {
	square_1.write_reg2(value);
}

//------------------------------------------------------------------------------
// Name: write4007
//------------------------------------------------------------------------------
void write4007(uint8_t value) {
	square_1.write_reg3(value);
}

//------------------------------------------------------------------------------
// Name: write4008
//------------------------------------------------------------------------------
void write4008(uint8_t value) {
	triangle.write_reg0(value);
}

//------------------------------------------------------------------------------
// Name: write400A
//------------------------------------------------------------------------------
void write400A(uint8_t value) {
	triangle.write_reg2(value);
}

//------------------------------------------------------------------------------
// Name: write400B
//------------------------------------------------------------------------------
void write400B(uint8_t value) {
	triangle.write_reg3(value);
}

//------------------------------------------------------------------------------
// Name: write400C
//------------------------------------------------------------------------------
void write400C(uint8_t value) {
	noise.write_reg0(value);
}

//------------------------------------------------------------------------------
// Name: write400E
//------------------------------------------------------------------------------
void write400E(uint8_t value) {
	noise.write_reg2(value);
}

//------------------------------------------------------------------------------
// Name: write400F
//------------------------------------------------------------------------------
void write400F(uint8_t value) {
	noise.write_reg3(value);
}

//------------------------------------------------------------------------------
// Name: write4010
//------------------------------------------------------------------------------
void write4010(uint8_t value) {
	dmc.write_reg0(value);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void write4011(uint8_t value) {
	dmc.write_reg1(value);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void write4012(uint8_t value) {
	dmc.write_reg2(value);
}

//------------------------------------------------------------------------------
// Name: write4013
//------------------------------------------------------------------------------
void write4013(uint8_t value) {
	dmc.write_reg3(value);
}

//------------------------------------------------------------------------------
// Name: write4015
//------------------------------------------------------------------------------
void write4015(uint8_t value) {

	// Writing to this register clears the DMC interrupt flag.
	status.dmc_irq = false;

	if(value & STATUS_ENABLE_SQUARE_1) {
		square_0.enable();
	} else {
		square_0.disable();
	}

	if(value & STATUS_ENABLE_SQUARE_2) {
		square_1.enable();
	} else {
		square_1.disable();
	}

	if(value & STATUS_ENABLE_TRIANGLE) {
		triangle.enable();
	} else {
		triangle.disable();
	}

	if(value & STATUS_ENABLE_NOISE) {
		noise.enable();
	} else {
		noise.disable();
	}

	if(value & STATUS_ENABLE_DMC) {
		dmc.enable();
	} else {
		dmc.disable();
	}

	if(!status.irq_firing) {
		cpu::clear_irq(cpu::APU_IRQ);
	}
}

//------------------------------------------------------------------------------
// Name: read4015
//------------------------------------------------------------------------------
uint8_t read4015() {
	uint8_t ret = status.raw & (STATUS_DMC_IRQ | STATUS_FRAME_IRQ);

	// reading this register clears the Frame interrupt flag.
	status.frame_irq = false;

	if(square_0.length_counter.value() > 0) {
		ret |= STATUS_ENABLE_SQUARE_1;
	}

	if(square_1.length_counter.value() > 0) {
		ret |= STATUS_ENABLE_SQUARE_2;
	}

	if(triangle.length_counter.value() > 0) {
		ret |= STATUS_ENABLE_TRIANGLE;
	}

	if(noise.length_counter.value() > 0) {
		ret |= STATUS_ENABLE_NOISE;
	}

	if(dmc.bytes_remaining() > 0) {
		ret |= STATUS_ENABLE_DMC;
	}

	if(!status.irq_firing) {
		cpu::clear_irq(cpu::APU_IRQ);
	}

	return ret;
}

//------------------------------------------------------------------------------
// Name: write4017
//------------------------------------------------------------------------------
void write4017(uint8_t value) {

	frame_counter_.raw  = value;
	last_frame_counter_ = value;

	if(frame_counter_.inihibit_frame_irq) {
		status.frame_irq = false;
		if(!status.irq_firing) {
			cpu::clear_irq(cpu::APU_IRQ);
		}
	}

	next_clock_ = apu_cycles_ + (apu_cycles_ & 1) + 1;

	clock_step_ = 0;

	if(!frame_counter_.mode) {
		next_clock_ += 7458;
	}
}

//------------------------------------------------------------------------------
// Name: clock_length
//------------------------------------------------------------------------------
void clock_length() {
	square_0.length_counter.clock();
	square_1.length_counter.clock();
	triangle.length_counter.clock();
	noise.length_counter.clock();

	square_0.sweep.clock();
	square_1.sweep.clock();
}

//------------------------------------------------------------------------------
// Name: clock_linear
//------------------------------------------------------------------------------
void clock_linear() {
	triangle.linear_counter.clock();

	square_0.envelope.clock();
	square_1.envelope.clock();
	noise.envelope.clock();
}

//------------------------------------------------------------------------------
// Name: clock_frame_mode_0
//------------------------------------------------------------------------------
void clock_frame_mode_0() {

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
		next_clock_ += 7457;
		break;

	case 3:
		if(!(frame_counter_.inihibit_frame_irq)) {
			status.frame_irq = true;
		}

		++next_clock_;
		break;

	case 4:
		clock_linear();
		clock_length();
		if(!(frame_counter_.inihibit_frame_irq)) {
			status.frame_irq = true;
		}

		++next_clock_;
		break;

	case 5:
		if(!(frame_counter_.inihibit_frame_irq)) {
			status.frame_irq = true;
		}

		next_clock_ += 7457;
		break;
	}

	clock_step_ = (clock_step_ + 1) % 6;
}

//------------------------------------------------------------------------------
// Name: clock_frame_mode_1
//------------------------------------------------------------------------------
void clock_frame_mode_1() {

	// 5 step sequence
	switch(clock_step_) {
	case 0:
		clock_linear();
		clock_length();
		next_clock_ += 7458;
		break;

	case 1:
		clock_linear();
		next_clock_ += 7456;
		break;

	case 2:
		clock_linear();
		clock_length();
		next_clock_ += 7458;
		break;

	case 3:
		clock_linear();
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
void run(int cycles) {

	while(cycles-- > 0) {

		if(!(frame_counter_.inihibit_frame_irq) && (status.frame_irq)) {
			cpu::irq(cpu::APU_IRQ);
		}

		if(apu_cycles_ == next_clock_) {
			if(frame_counter_.mode) {
				clock_frame_mode_1();
			} else {
				clock_frame_mode_0();
			}
		}

		if(sample_index_ != sizeof(sample_buffer_)) {
			if((apu_cycles_ % ClocksPerSample) == 0) {
				sample_buffer_[sample_index_++] = mix_channels();
			}
		}

		dmc.tick();
		noise.tick();
		triangle.tick();
		square_0.tick();
		square_1.tick();

		++apu_cycles_;
	}
}

//------------------------------------------------------------------------------
// Name: buffer
//------------------------------------------------------------------------------
const uint8_t *buffer() {
	sample_index_ = 0;
	return sample_buffer_;
}

//------------------------------------------------------------------------------
// Name: mix_channels
//------------------------------------------------------------------------------
uint8_t mix_channels() {

	const uint8_t pulse1_out   = square_0.output();
	const uint8_t pulse2_out   = square_1.output();
	const uint8_t triangle_out = triangle.output();
	const uint8_t noise_out    = noise.output();
	const uint8_t dmc_out      = dmc.output();

#if 0
	float pulse_volume = 0;
	float tnd_volume   = 0;

	if(pulse1_out || pulse2_out) {
		pulse_volume = 95.88 / ((8128.0 / (pulse1_out + pulse2_out)) + 100.0);
	}

	if(triangle_out || noise_out || dmc_out) {
		tnd_volume = 159.79 / ((1 / ((triangle_out / 8227.0) + (noise_out / 12241.0) + (dmc_out / 22638.0))) + 100.0);
	}

	int result = (pulse_volume + tnd_volume) * 0x100;
#else
	int result = (
		pulse1_out   +
		pulse2_out   +
		triangle_out +
		noise_out    +
		dmc_out      +
		0);
#endif

	return bound(0, result, 255);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
uint64_t cycle_count() {
	return apu_cycles_;
}

}
}
