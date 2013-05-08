
#include "Cart.h"
#include "Mapper.h"
#include "NES.h"
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
void Cart::load(const std::string &s) {

	std::cout << "[Cart::load] loading '" << s << "'...";

	const UNIF_RETURN_CODE r = load_file_INES(s.c_str(), &cart_);
	if(r == UNIF_OK) {
		std::cout << " OK!" << std::endl;

		const size_t prg_size = prg_pages() * (16 * 1024);
		const size_t chr_size = chr_pages() * (8 * 1024);

		// get mask values
		prg_mask_ = create_mask(prg_size);
		chr_mask_ = create_mask(chr_size);

		switch(mirroring_INES(&cart_)) {
		case MIRR_HORIZONTAL: mirroring_ = MIR_HORIZONTAL; break;
		case MIRR_VERTICAL:   mirroring_ = MIR_VERTICAL;   break;
		case MIRR_4SCREEN:    mirroring_ = MIR_4SCREEN;    break;
		default:              mirroring_ = MIR_MAPPER;     break;
		}

		prg_hash_ = prg_hash_INES(&cart_);
		chr_hash_ = chr_hash_INES(&cart_);
		rom_hash_ = rom_hash_INES(&cart_);

		std::cout << "PRG HASH: " << std::hex << std::setw(8) << std::setfill('0') << prg_hash_ << std::dec << std::endl;
		std::cout << "CHR HASH: " << std::hex << std::setw(8) << std::setfill('0') << chr_hash_ << std::dec << std::endl;
		std::cout << "ROM HASH: " << std::hex << std::setw(8) << std::setfill('0') << rom_hash_ << std::dec << std::endl;

		if((prg_size & (prg_size - 1)) != 0) {
			std::cout << "WARNING: PRG size is not a power of 2, this is unusual" << std::endl;
		}

		if((chr_size & (chr_size - 1)) != 0) {
			std::cout << "WARNING: CHR size is not a power of 2, this is unusual" << std::endl;
		}

		mapper_ = Mapper::create_mapper(mapper_INES(&cart_));
	} else {
		std::cout << " ERROR! (" << r << ")" << std::endl;
		mapper_ = boost::shared_ptr<Mapper>();
		prg_hash_ = 0;
		chr_hash_ = 0;
		rom_hash_ = 0;
	}
}

//------------------------------------------------------------------------------
// Name: unload
//------------------------------------------------------------------------------
void Cart::unload() {
	free_file_INES(&cart_);
	memset(&cart_, 0, sizeof(cart_));
	mapper_.reset();
}

//------------------------------------------------------------------------------
// Name: prg_pages
//------------------------------------------------------------------------------
int Cart::prg_pages() const {
	return prg_size_INES(&cart_);
}

//------------------------------------------------------------------------------
// Name: chr_pages
//------------------------------------------------------------------------------
int Cart::chr_pages() const{
	return chr_size_INES(&cart_);
}

//------------------------------------------------------------------------------
// Name: has_chr_rom
//------------------------------------------------------------------------------
bool Cart::has_chr_rom() const {
	return chr_pages() != 0;
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
	return cart_.prg_rom;
}

//------------------------------------------------------------------------------
// Name: chr
//------------------------------------------------------------------------------
uint8_t *Cart::chr() const {
	return cart_.chr_rom;
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
const boost::shared_ptr<Mapper> &Cart::mapper() const {
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

	const uint8_t *const prg_rom  = prg();
	const uint8_t *const chr_rom  = chr();
	const size_t         prg_size = prg_pages() * (16 * 1024);

	// create a vector and copy the PRG into it
	std::vector<uint8_t> image(prg_rom, prg_rom + prg_size);

	// if there is CHR, insert it at the end of the vector
	if(chr_rom) {
		const size_t chr_size = chr_pages() * (8 * 1024);
		image.insert(image.end(), chr_rom, chr_rom + chr_size);
	}

 	return image;
}
