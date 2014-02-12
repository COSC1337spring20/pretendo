#ifndef ROR_20121206_H_
#define ROR_20121206_H_

//------------------------------------------------------------------------------
// Name: opcode_ror
// Desc: Rotate Right
//------------------------------------------------------------------------------
struct opcode_ror {

	typedef operation_modify memory_access;
	
	void operator()(Context &ctx, uint8_t &data) const {
		// only correct because C_MASK is 0x01, we basically want to shift in an 0x80
		// if C_MASK is set, and a 0 if it is not
		const uint8_t bit = (ctx.P & C_MASK) << 7;
		set_flag_condition<C_MASK>(ctx, data & 0x01);
		data = ((data >> 1) & 0x7f) | bit;
		update_nz_flags(ctx, data);
	}
};

#endif

