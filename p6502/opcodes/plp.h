#ifndef PLP_20121206_H_
#define PLP_20121206_H_

//------------------------------------------------------------------------------
// Name: opcode_plp
// Desc: Pull Processor Status
//------------------------------------------------------------------------------
struct opcode_plp {

	typedef operation_stack_read memory_access;

	void operator()(Context &ctx, uint8_t data) const {
		ctx.P = ((data & ~B_MASK) | R_MASK);
	}
};

#endif

