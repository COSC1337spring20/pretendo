
#ifndef ZERO_PAGE_Y_H_
#define ZERO_PAGE_Y_H_

template <class Op>
class zero_page_y {
public:
	// dispatch to the appropriate version of the address mode	
	static void execute() {
		execute(typename Op::memory_access());
	}
	
private:
	static void execute(const operation_read &) {

		switch(cycle_) {
		case 1:
			// fetch address, increment PC
			effective_address16_.raw = PC.raw++;
			break;
		case 2:
			// read from address, add index register to it
			effective_address16_.raw = read_handler(effective_address16_.raw) + Y;
			break;
		case 3:
			LAST_CYCLE;
			// read from effective address
            Op::execute(read_handler(effective_address16_.lo));
			OPCODE_COMPLETE;
		default:
			abort();
		}
	}

	static void execute(const operation_write &) {

		switch(cycle_) {
		case 1:
			// fetch address, increment PC
			effective_address16_.raw = PC.raw++;
			break;
		case 2:
			// read from address, add index register to it
			effective_address16_.raw = read_handler(effective_address16_.raw) + Y;
			break;
		case 3:
			LAST_CYCLE;
			// write to effective address
			{
				const uint16_t address = effective_address16_.lo;
				const uint8_t  value   = Op::execute(address);
            	write_handler(address, value);
			}
			OPCODE_COMPLETE;
		default:
			abort();
		}
	}
};

#endif
