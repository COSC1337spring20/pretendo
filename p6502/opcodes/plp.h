#ifndef PLP_20121206_H_
#define PLP_20121206_H_

//------------------------------------------------------------------------------
// Name: opcode_plp
// Desc: Pull Processor Status
//------------------------------------------------------------------------------
struct opcode_plp {

	typedef operation_stack_read memory_access;

	static void execute(uint8_t data) {
		P = ((data & ~B_MASK) | R_MASK);
	}
};

#endif

