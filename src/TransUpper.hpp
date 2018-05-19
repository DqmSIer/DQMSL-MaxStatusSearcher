#ifndef TRANSUPPER_CLASS
#define TRANSUPPER_CLASS

#include "Status.hpp"

class TransUpper {
public: /* define */
	static const int TR_RATE = 20;

public: /* function */
	static inline
	int calcTransUp(const int value) {
		return (value * TR_RATE + 100 - 1) / 100;
	}

	static inline
	Status getTransUpValue(const Status &main, const Status &base) {
		Status result;
		for (int i = 0; i < Status::STAT_NUM; i++) {
			const int diff = main[i] - base[i];
			result[i] = TransUpper::calcTransUp(diff);
		}
		return result;
	}

	static inline
	Status transUp(const Status &main, const Status &base, const Status &next_base) {
		Status result;
		for (int i = 0; i < Status::STAT_NUM; i++) {
			const int diff = main[i] - base[i];
			result[i] = next_base[i] + TransUpper::calcTransUp(diff);
		}
		return result;
	}
};

#endif //TRANSUPPER_CLASS
