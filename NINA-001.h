
#ifndef NINA_001_20110314_H_
#define NINA_001_20110314_H_

#include "Mapper.h"

class NINA_001 : public Mapper {
public:
	NINA_001();

public:
	virtual std::string name() const;

public:
	virtual void write_7(uint16_t address, uint8_t value);
};

#endif
