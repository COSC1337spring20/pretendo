#ifndef TAX_20121206_H_
#define TAX_20121206_H_

//------------------------------------------------------------------------------
// Name: opcode_tax
// Desc: Transfer A to X
//------------------------------------------------------------------------------
struct opcode_tax {
	typedef operation_none memory_access;
	
	static void execute() {
		X = A;
		update_nz_flags(X);
	}
};

#endif

