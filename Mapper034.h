
#ifndef MAPPER034_20080314_H_
#define MAPPER034_20080314_H_

#include "Mapper.h"


class Mapper34 : public Mapper {
public:
	Mapper34();
	virtual ~Mapper34();

public:
	virtual std::string name() const;

	virtual void write_7(uint16_t address, uint8_t value);
	virtual void write_8(uint16_t address, uint8_t value);
	virtual void write_9(uint16_t address, uint8_t value);
	virtual void write_a(uint16_t address, uint8_t value);
	virtual void write_b(uint16_t address, uint8_t value);
	virtual void write_c(uint16_t address, uint8_t value);
	virtual void write_d(uint16_t address, uint8_t value);
	virtual void write_e(uint16_t address, uint8_t value);
	virtual void write_f(uint16_t address, uint8_t value);

private:
	Mapper *mapper_;
};

#endif

