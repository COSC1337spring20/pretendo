#ifndef LSR_20121206_H_
#define LSR_20121206_H_

//------------------------------------------------------------------------------
// Name: opcode_lsr
// Desc: Logical Shift Right
//------------------------------------------------------------------------------
struct opcode_lsr {

	typedef operation_modify memory_access;
	
	void operator()(uint8_t &data) const {
		set_flag_condition<C_MASK>(data & 0x01);
		data >>= 1;
		update_nz_flags(data);
	}
};

#endif

