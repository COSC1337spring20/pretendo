
#include "Mapper010.h"

SETUP_STATIC_INES_MAPPER_REGISTRAR(10);

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
Mapper10::Mapper10() : latch1_(true), latch2_(true), latch1_lo_(0), latch1_hi_(0), latch2_lo_(0), latch2_hi_(0) {

	set_prg_89ab(0);
	set_prg_cdef(-1);

	assert(nes::cart.has_chr_rom());
	set_chr_0000_1fff(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
std::string Mapper10::name() const {
	return "FxROM (Nintendo MMC4)";
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper10::write_a(uint16_t address, uint8_t value) {
	(void)address;
	set_prg_89ab(value & 0x0f);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper10::write_b(uint16_t address, uint8_t value) {
	(void)address;
	latch1_lo_ = value;

	if(!latch1_) {
		set_chr_0000_0fff(value & 0x1f);
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper10::write_c(uint16_t address, uint8_t value) {
	(void)address;
	latch1_hi_ = value;

	if(latch1_) {
		set_chr_0000_0fff(value & 0x1f);
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper10::write_d(uint16_t address, uint8_t value) {
	(void)address;
	latch2_lo_ = value;

	if(!latch2_) {
		set_chr_1000_1fff(value & 0x1f);
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper10::write_e(uint16_t address, uint8_t value) {
	(void)address;
	latch2_hi_ = value;

	if(latch2_) {
		set_chr_1000_1fff(value & 0x1f);
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper10::write_f(uint16_t address, uint8_t value) {
	(void)address;
	if(value & 0x01) {
		nes::ppu.set_mirroring(PPU::mirror_horizontal);
	} else {
		nes::ppu.set_mirroring(PPU::mirror_vertical);
	}
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
uint8_t Mapper10::read_vram(uint16_t address) {

	const uint8_t ret = Mapper::read_vram(address);

	switch(address & 0xfff0) {
	case 0x0fd0:
		set_chr_0000_0fff(latch1_lo_);
		latch1_ = false;
		break;
	case 0x1fd0:
		set_chr_1000_1fff(latch2_lo_);
		latch2_ = false;
		break;
	case 0x0fe0:
		set_chr_0000_0fff(latch1_hi_);
		latch1_ = true;
		break;
	case 0x1fe0:
		set_chr_1000_1fff(latch2_hi_);
		latch2_ = true;
		break;
	}

	return ret;
}
