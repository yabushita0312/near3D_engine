#pragma once
#ifndef INCLUDED_thd_h_
#define INCLUDED_thd_h_

#include "sub.hpp"
#include "useful.hpp"
#include <thread> 
#include <chrono>
#include <array> 
//üÍ
struct input {
	std::array<bool, 13> get; /*L[p(êÄ)*/
};
struct output {
	bool ends{ false }; /*I¹tO*/
	float x,y;
};
//60fpsðÛµÂÂìðZ(box2DÝ)
class ThreadClass {
private:
	std::thread thread_1;
	void calc(input& p_in, output& p_out);
public:
	ThreadClass();
	~ThreadClass();
	void thread_start(input& p_in, output& p_out);
	void thead_stop();
};

#endif 