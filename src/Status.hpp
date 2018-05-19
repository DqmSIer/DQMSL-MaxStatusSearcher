#ifndef STATUS_CLASS
#define STATUS_CLASS

#include <iostream>
#include <cstring>

struct Status {
public: /* define */
	static constexpr int STAT_NUM = 6;
	static const char *STAT_STRING[];

public: /* function */
	void initialize(void) { std::memset(this, 0, sizeof(*this)); }
	void finalize(void) { this->initialize(); }

	void scanStatus(void) {
		for (int i = 0; i < Status::STAT_NUM; i++) {
			std::string str;

			do {
				std::cout << Status::STAT_STRING[i];
				std::cin >> str;
			} while (str[0] == '-');

			(*this)[i] = std::stoi(str);
		}
	}

	void printStatus(void) const {
		for (int i = 0; i < Status::STAT_NUM; i++) {
			std::cout << Status::STAT_STRING[i] << (*this)[i] << std::endl;
		}
	}

	Status operator+(const Status &stat) const {
		Status result;
		for (int i = 0; i < Status::STAT_NUM; i++)
			result[i] = (*this)[i] + stat[i];
		return result;
	}

	bool operator>(const Status &stat) const {
		bool hit = true;
		for (int i = 0; i < Status::STAT_NUM; i++) {
			if ((*this)[i] < stat[i]) {
				hit = false;
				break;
			}
		}
		return hit;
	}
	bool operator<(const Status &stat) const {
		bool hit = true;
		for (int i = 0; i < Status::STAT_NUM; i++) {
			if ((*this)[i] > stat[i]) {
				hit = false;
				break;
			}
		}
		return hit;
	}
	bool operator>=(const Status &stat) const { return !(*this < stat); }
	bool operator<=(const Status &stat) const { return !(*this > stat); }
	bool operator==(const Status &stat) const {
		bool hit = true;
		for (int i = 0; i < Status::STAT_NUM; i++) {
			if ((*this)[i] != stat[i]) {
				hit = false;
				break;
			}
		}
		return hit;
	}
	bool operator!=(const Status &stat) const { return !(*this == stat); }

	int &operator[](const int idx) { return this->stat[idx]; }
	const int &operator[](const int idx) const { return this->stat[idx]; }

private: /* member */
	int stat[Status::STAT_NUM];
};

const char *Status::STAT_STRING[] = {
	"HP : ",
	"MP : ",
	"ATK: ",
	"DEF: ",
	"SPD: ",
	"INT: "
};

#endif // STATUS_CLASS
