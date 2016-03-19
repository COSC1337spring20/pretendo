
#include "Cart.h"
#include "iNES/Error.h"
#include "Mapper.h"
#include <cstring>
#include <iostream>
#include <iomanip>

namespace {

//------------------------------------------------------------------------------
// Name: create_mask
//------------------------------------------------------------------------------
size_t create_mask(size_t size) {

	// returns 1 less than closest fitting power of 2
	// is this number not a power of two or 0?
	if((size & (size - 1)) != 0) {
		// yea, fix it!
		--size;
		size |= size >> 1;
		size |= size >> 2;
		size |= size >> 4;
		size |= size >> 8;
		size |= size >> 16;
		++size;
	} else if(size == 0) {
		++size;
	}

	return --size;
}

//------------------------------------------------------------------------------
// Name: is_power_of_2
//------------------------------------------------------------------------------
constexpr bool is_power_of_2(size_t size) {
	return (size & (size - 1)) == 0;
}

}

//------------------------------------------------------------------------------
// Name: Cart
//------------------------------------------------------------------------------
Cart::Cart() : prg_mask_(0), chr_mask_(0), prg_hash_(0), chr_hash_(0), rom_hash_(0), mirroring_(MIR_HORIZONTAL){
	memset(&cart_, 0, sizeof(cart_));
}

//------------------------------------------------------------------------------
// Name: ~Cart
//------------------------------------------------------------------------------
Cart::~Cart() {
	unload();
}

//------------------------------------------------------------------------------
// Name: load
//------------------------------------------------------------------------------
bool
Cart::load(const std::string &s) {

	std::cout << "[Cart::load] loading '" << s << "'...";
	
	try {	
		cart_ = new iNES::Rom(s.c_str());
		
		std::cout << " OK!" << std::endl;

		// get mask values
		prg_mask_ = create_mask(cart_->prg_size_);
		chr_mask_ = create_mask(cart_->chr_size_);

		switch(cart_->header_->mirroring()) {
		case iNES::Mirroring::HORIZONTAL:  mirroring_ = MIR_HORIZONTAL; break;
		case iNES::Mirroring::VERTICAL:    mirroring_ = MIR_VERTICAL;   break;
		case iNES::Mirroring::FOUR_SCREEN: mirroring_ = MIR_4SCREEN;    break;
		default:
			mirroring_ = MIR_MAPPER;
			break;
		}

		prg_hash_ = cart_->prg_hash();
		chr_hash_ = cart_->chr_hash();
		rom_hash_ = cart_->rom_hash();

		std::cout << "PRG HASH: " << std::hex << std::setw(8) << std::setfill('0') << prg_hash_ << std::dec << std::endl;
		std::cout << "CHR HASH: " << std::hex << std::setw(8) << std::setfill('0') << chr_hash_ << std::dec << std::endl;
		std::cout << "ROM HASH: " << std::hex << std::setw(8) << std::setfill('0') << rom_hash_ << std::dec << std::endl;

		if(!is_power_of_2(cart_->prg_size_)) {
			std::cout << "WARNING: PRG size is not a power of 2, this is unusual" << std::endl;
		}

		if(!is_power_of_2(cart_->chr_size_)) {
			std::cout << "WARNING: CHR size is not a power of 2, this is unusual" << std::endl;
		}

		mapper_ = Mapper::create_mapper(cart_->header()->mapper());
		return true;
	} catch (const iNES::ines_error &e) {
		std::cout << " ERROR Loading ROM File!" << std::endl;
		cart_     = nullptr;
		mapper_   = std::shared_ptr<Mapper>();
		prg_hash_ = 0;
		chr_hash_ = 0;
		rom_hash_ = 0;
	}

	return false;
}

//------------------------------------------------------------------------------
// Name: unload
//------------------------------------------------------------------------------
void Cart::unload() {
	delete cart_;
	cart_ = nullptr;
	mapper_.reset();
}

//------------------------------------------------------------------------------
// Name: has_chr_rom
//------------------------------------------------------------------------------
bool Cart::has_chr_rom() const {
	return cart_->chr_rom_;
}

//------------------------------------------------------------------------------
// Name: prg_mask
//------------------------------------------------------------------------------
uint32_t Cart::prg_mask() const {
	return prg_mask_;
}

//------------------------------------------------------------------------------
// Name: chr_mask
//------------------------------------------------------------------------------
uint32_t Cart::chr_mask() const {
	return chr_mask_;
}

//------------------------------------------------------------------------------
// Name: prg
//------------------------------------------------------------------------------
uint8_t *Cart::prg() const {
	return cart_->prg_rom_;
}

//------------------------------------------------------------------------------
// Name: chr
//------------------------------------------------------------------------------
uint8_t *Cart::chr() const {
	return cart_->chr_rom_;
}

//------------------------------------------------------------------------------
// Name: mirroring
//------------------------------------------------------------------------------
Cart::MIRRORING Cart::mirroring() const {
	return mirroring_;
}

//------------------------------------------------------------------------------
// Name: mapper
//------------------------------------------------------------------------------
const std::shared_ptr<Mapper> &Cart::mapper() const {
	return mapper_;
}

//------------------------------------------------------------------------------
// Name: prg_hash
//------------------------------------------------------------------------------
uint32_t Cart::prg_hash() const {
	return prg_hash_;
}

//------------------------------------------------------------------------------
// Name: chr_hash
//------------------------------------------------------------------------------
uint32_t Cart::chr_hash() const {
	return chr_hash_;
}

//------------------------------------------------------------------------------
// Name: rom_hash
//------------------------------------------------------------------------------
uint32_t Cart::rom_hash() const {
	return rom_hash_;
}

//------------------------------------------------------------------------------
// Name: raw_image
//------------------------------------------------------------------------------
std::vector<uint8_t> Cart::raw_image() const {

	const uint8_t *const prg_rom = prg();
	const uint8_t *const chr_rom = chr();

	// create a vector and copy the PRG into it
	std::vector<uint8_t> image(prg_rom, prg_rom + cart_->prg_size_);

	// if there is CHR, insert it at the end of the vector
	if(chr_rom) {
		image.insert(image.end(), chr_rom, chr_rom + cart_->chr_size_);
	}

	return image;
}
