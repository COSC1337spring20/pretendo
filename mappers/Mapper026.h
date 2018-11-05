
#ifndef MAPPER026_20080314_H_
#define MAPPER026_20080314_H_

#include "Mapper.h"
#include <memory>

class Mapper26 final : public Mapper {
public:
	Mapper26();

public:
	std::string name() const override;

public:
	void write_8(uint16_t address, uint8_t value) override;
	void write_9(uint16_t address, uint8_t value) override;
	void write_a(uint16_t address, uint8_t value) override;
	void write_b(uint16_t address, uint8_t value) override;
	void write_c(uint16_t address, uint8_t value) override;
	void write_d(uint16_t address, uint8_t value) override;
	void write_e(uint16_t address, uint8_t value) override;
	void write_f(uint16_t address, uint8_t value) override;

public:
	void cpu_sync() override;

private:
	std::unique_ptr<Mapper> mapper_;
};

#endif
