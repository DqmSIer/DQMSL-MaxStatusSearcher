#ifndef STATUSINFO_CLASS
#define STATUSINFO_CLASS

#include <iostream>
#include <iomanip>
#include <cstring>

#include "Status.hpp"

struct StatusInfo {
public: /* function */
	void initialize(void) { std::memset(this, 0, sizeof(*this)); }
	void finalize(void) { this->initialize(); }

	void printStatus(void) const { this->status.printStatus(); }
	void printBonus(void) const { this->bonus.printStatus(); }
	void printStatusWithBonus(void) const {
		for (int i = 0; i < Status::STAT_NUM; i++) {
			std::cout << Status::STAT_STRING[i]
				  << std::setw(4) << this->status[i]
				  << " [" << std::setw(2) << this->bonus[i] << "]" << std::endl;
		}
	}

public: /* member */
	int star;
	int cost;
	Status status;
	Status bonus;
	const StatusInfo *basePtr;
	const StatusInfo *partnerPtr;
	std::string combi;
};

#endif // STATUSINFO_CLASS
