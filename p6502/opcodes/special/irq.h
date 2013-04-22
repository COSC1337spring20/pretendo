#ifndef IRQ_20140417_H_
#define IRQ_20140417_H_

#define LAST_CYCLE_0                                 \
do {                                                 \
	rst_executing = false;                           \
	irq_executing = false;                           \
													 \
	if(rst_asserted) {                               \
		rst_executing = true;                        \
	} else if(irq_asserted && ((P & I_MASK) == 0)) { \
		irq_executing = true;                        \
	}                                                \
	rst_asserted = false;                            \
} while(0)

//------------------------------------------------------------------------------
// Name: opcode_irq
// Desc: Interrupt
//------------------------------------------------------------------------------
struct opcode_irq {

	void operator()(int cycle) {

		switch(cycle) {
		case 1:
			// read next instruction byte (and throw it away),
			// increment PC
			read_byte(PC);
			break;
		case 2:
			// push PCH on stack, decrement S
			write_byte(S-- + STACK_ADDRESS, pc_hi());
			break;
		case 3:
			// push PCL on stack, decrement S
			write_byte(S-- + STACK_ADDRESS, pc_lo());
			break;
		case 4:
			// push P on stack, decrement S
			write_byte(S-- + STACK_ADDRESS, P);
			if(nmi_asserted) {
				vector_ = NMI_VECTOR_ADDRESS;
				nmi_asserted = false;
			} else {
				vector_ = IRQ_VECTOR_ADDRESS;
			}
			break;
		case 5:
			set_flag<I_MASK>();
			// fetch PCL
			set_pc_lo(read_byte(vector_ + 0));
			break;
		case 6:
			LAST_CYCLE_0;
			// fetch PCH
			set_pc_hi(read_byte(vector_ + 1));
			OPCODE_COMPLETE;
		default:
			abort();
		}
	}

private:
	uint16_t vector_;
};

#undef LAST_CYCLE_0

#endif

