
#ifndef INPUT_20080314_H_
#define INPUT_20080314_H_

#include "Controller.h"

namespace nes {
namespace input {

void reset();

void write4016(uint8_t value);
uint8_t read4016();
uint8_t read4017();

Controller &controller1();
Controller &controller2();
Controller &controller3();
Controller &controller4();

}
}

#endif

