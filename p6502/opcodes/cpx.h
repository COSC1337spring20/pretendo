#ifndef CPX_20121206_H_
#define CPX_20121206_H_

#include "compare.h"

//------------------------------------------------------------------------------
// Name: opcode_cpx
// Desc: Compare X Register
//------------------------------------------------------------------------------
struct opcode_cpx : compare {

	void operator()(Context &ctx, uint8_t data) const {
		do_compare(ctx, ctx.X, data);
	}
};

#endif

