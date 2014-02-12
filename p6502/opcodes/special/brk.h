
#ifndef BRK_20140417_H_
#define BRK_20140417_H_

//------------------------------------------------------------------------------
// Name: opcode_brk
// Desc: Software Interrupt
//------------------------------------------------------------------------------
class opcode_brk {
public:
	opcode_brk() {
	}

public:
	void operator()(Context &ctx) {
		execute(ctx);
	}

private:
	void execute(Context &ctx) {
		switch(ctx.cycle) {
		case 1:
			// read next instruction byte (and throw it away),
			// increment PC
			read_byte(ctx, ctx.PC++);
			break;
		case 2:
			// push PCH on stack, decrement S
			write_byte(ctx, ctx.S-- + STACK_ADDRESS, pc_hi(ctx));
			break;
		case 3:
			// push PCL on stack, decrement S
			write_byte(ctx, ctx.S-- + STACK_ADDRESS, pc_lo(ctx));
			break;
		case 4:
			// push P on stack, decrement S
			write_byte(ctx, ctx.S-- + STACK_ADDRESS, ctx.P | B_MASK);
			if(ctx.nmi_asserted) {
				vector_ = NMI_VECTOR_ADDRESS;
				ctx.nmi_asserted = false;
			} else {
				vector_ = IRQ_VECTOR_ADDRESS;
			}
			break;
		case 5:
			set_flag<I_MASK>(ctx);
			// fetch PCL
			set_pc_lo(ctx, read_byte(ctx, vector_ + 0));
			break;
		case 6:
			// NOTE: are we supposed to check for interrupts here?
			//       I only pass the tests if I don't.
			//LAST_CYCLE;

			// fetch PCH
			set_pc_hi(ctx, read_byte(ctx, vector_ + 1));
			OPCODE_COMPLETE;
		default:
			abort();
		}
	}

private:
	uint16_t vector_;
};

#endif

