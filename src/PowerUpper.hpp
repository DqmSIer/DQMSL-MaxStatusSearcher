#ifndef POWERUPPER_CLASS
#define POWERUPPER_CLASS

#include "Status.hpp"

class PowerUpper {
public: /* define */
	static const int PU_RATE = 2;

public: /* function */
	static inline
	int calcPowerUp(const int value) {
		return (value * PU_RATE + 100 - 1) / 100;
	}

	static inline
	Status getPowerUpValue(const Status &main) {
		Status result;
		for (int i = 0; i < Status::STAT_NUM; i++)
			result[i] = PowerUpper::calcPowerUp(main[i]);
		return result;
	}

	static inline
	Status getPowerUpValue(const Status &main, const Status &sub) {
		Status result;
		for (int i = 0; i < Status::STAT_NUM; i++)
			result[i] = PowerUpper::calcPowerUp(main[i]) + PowerUpper::calcPowerUp(sub[i]);
		return result;
	}

	static inline
	Status powerUp(const Status &main, const Status &sub) {
		Status result;
		for (int i = 0; i < Status::STAT_NUM; i++)
			result[i] = main[i] + PowerUpper::calcPowerUp(main[i]) + PowerUpper::calcPowerUp(sub[i]);
		return result;
	}
};

#endif //POWERUPPER_CLASS
