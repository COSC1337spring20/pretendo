#ifndef SXA_20121206_H_
#define SXA_20121206_H_

//------------------------------------------------------------------------------
// Name: opcode_sxa
// Desc: AND X register with the high byte of the target address of the
//       argument + 1. Store the result in memory.
//------------------------------------------------------------------------------
struct opcode_sxa {

	typedef operation_write memory_access;

	uint8_t operator()(Context &ctx, uint16_t &address) const {
	
		const uint8_t high_byte = ((address >> 8) & 0xff) + 1;
		const uint8_t value     = (ctx.X & high_byte);
		address = (address & 0x00ff) | (value << 8);
		return ctx.X & high_byte;
	}
};


#endif

