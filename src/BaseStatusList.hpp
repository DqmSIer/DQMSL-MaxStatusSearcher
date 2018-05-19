#ifndef BASESTATUSLIST_CLASS
#define BASESTATUSLIST_CLASS

#include <iostream>
#include <vector>

#include "Status.hpp"

struct BaseStatusList {
public: /* function */
	void initialize(const int transCnt) { this->list.resize(transCnt); }
	void finalize(void) { this->list.clear(); this->list.shrink_to_fit(); }
	void input(void) {
		for (int i = 0; i < this->list.size(); i++) { 
			Status &status = this->list[i];
			std::cout << "Input " << i << "-th base status" << std::endl;
			status.scanStatus();
		}
	}
	size_t size(void) const { return this->list.size(); }
	Status &operator[](const int i) { return this->list[i]; }
	const Status &operator[](const int i) const { return this->list[i]; }

public: /* member */
	std::vector<Status> list;
};

#endif // BASESTATUSLIST_CLASS


