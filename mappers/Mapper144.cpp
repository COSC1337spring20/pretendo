
#include "Mapper144.h"
#include "Bus.h"

SETUP_STATIC_INES_MAPPER_REGISTRAR(144)

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
Mapper144::Mapper144() {

	set_prg_89abcdef(0);
	set_chr_0000_1fff(0);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
std::string Mapper144::name() const {
	return "Color Dreams (50282)";
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper144::write_8(uint16_t address, uint8_t value) {
	write_handler(address, value);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper144::write_9(uint16_t address, uint8_t value) {
	write_handler(address, value);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper144::write_a(uint16_t address, uint8_t value) {
	write_handler(address, value);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper144::write_b(uint16_t address, uint8_t value) {
	write_handler(address, value);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper144::write_c(uint16_t address, uint8_t value) {
	write_handler(address, value);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper144::write_d(uint16_t address, uint8_t value) {
	write_handler(address, value);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper144::write_e(uint16_t address, uint8_t value) {
	write_handler(address, value);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper144::write_f(uint16_t address, uint8_t value) {
	write_handler(address, value);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void Mapper144::write_handler(uint16_t address, uint8_t value) {

	const uint8_t effective_value = (nes::bus::read_memory(address) & (value | 1));

	set_prg_89abcdef(effective_value & 0x3);
	set_chr_0000_1fff((effective_value >> 4) & 0x0f);
}

