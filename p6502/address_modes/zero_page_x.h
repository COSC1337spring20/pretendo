
#ifndef ZERO_PAGE_X_H_
#define ZERO_PAGE_X_H_

class zero_page_x {
public:
	zero_page_x() {
	}
	
public:
	// dispatch to the appropriate version of the address mode
	template <class Op>
	void operator()(Context &ctx, Op op) {
		execute(ctx, op, typename Op::memory_access());
	}
	
private:
	template <class Op>
	void execute(Context &ctx, Op op, const operation_read &) {

		switch(ctx.cycle) {
		case 1:
			// fetch address, increment ctx.PC
			effective_address_ = ctx.PC++;
			break;
		case 2:
			// read from address, add index register to it
			effective_address_ = read_byte(ctx, effective_address_) + ctx.X;
			break;
		case 3:
			LAST_CYCLE;
			// read from effective address
            op(ctx, read_byte_zp(ctx, effective_address_ & 0xff));
			OPCODE_COMPLETE;
		default:
			abort();
		}
	}

	template <class Op>
	void execute(Context &ctx, Op op, const operation_modify &) {

		switch(ctx.cycle) {
		case 1:
			// fetch address, increment ctx.PC
			effective_address_ = ctx.PC++;
			break;
		case 2:
			// read from address, add index register ctx.X to it
			effective_address_ = read_byte(ctx, effective_address_) + ctx.X;
			break;
		case 3:
			// read from effective address
            data_ = read_byte_zp(ctx, effective_address_ & 0xff);
			break;
		case 4:
			// write the value back to effective address,
			// and do the operation on it
            write_byte_zp(ctx, effective_address_ & 0xff, data_);
			op(ctx, data_);
			break;
		case 5:
			LAST_CYCLE;
			// write the new value to effective address
            write_byte_zp(ctx, effective_address_ & 0xff, data_);
			OPCODE_COMPLETE;
		default:
			abort();
		}
	}

	template <class Op>
	void execute(Context &ctx, Op op, const operation_write &) {

		switch(ctx.cycle) {
		case 1:
			// fetch address, increment ctx.PC
			effective_address_ = ctx.PC++;
			break;
		case 2:
			// read from address, add index register to it
			effective_address_ = read_byte(ctx, effective_address_) + ctx.X;
			break;
		case 3:
			LAST_CYCLE;
			// write to effective address
			{
				uint16_t address = effective_address_ & 0xff;
				uint8_t  value   = op(ctx, address);
            	write_byte_zp(ctx, address, value);
			}
			OPCODE_COMPLETE;
		default:
			abort();
		}
	}

private:
	uint16_t effective_address_;
	uint8_t  data_;
	
};

#endif
