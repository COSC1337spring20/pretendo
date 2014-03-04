
#ifndef INPUT_20080314_H_
#define INPUT_20080314_H_

#include "Controller.h"
#include <boost/noncopyable.hpp>

class Input : public boost::noncopyable {
public:
	Input();

public:
	void reset();

public:
	void write4016(uint8_t value);
	uint8_t read4016();
	uint8_t read4017();

public:
	Controller &controller1();
	Controller &controller2();
	Controller &controller3();
	Controller &controller4();

private:
	uint8_t    strobe_;
	Controller controller1_;
	Controller controller2_;
	Controller controller3_;
	Controller controller4_;
};

#endif

