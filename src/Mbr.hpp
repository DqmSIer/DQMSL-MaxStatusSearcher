#ifndef MBR_CLASS
#define MBR_CLASS

#include <cstring>

template <int D>
struct Mbr {
public: /* function */
	Mbr(void) = default;
	~Mbr(void) = default;
	Mbr(const int *const lower, const int *const upper) { 
		this->setLower(lower);
		this->setUpper(upper);
	}

	void clear(void) {
		std::memset(this, 0, sizeof(*this));
	}

	Mbr *get(void) {
		return this;
	}
	const Mbr *get(void) const {
		return this;
	}
	int *getLower(void) {
		return &this->lowerPoint[0];
	}
	int *getUpper(void) {
		return &this->upperPoint[0];
	}
	const int *getLower(void) const {
		return &this->lowerPoint[0];
	}
	const int *getUpper(void) const {
		return &this->upperPoint[0];
	}

	void set(const int *const lower, const int *const upper) {
		this->setLower(lower);
		this->setUpper(upper);
	}
	void setLower(const int *const lower) {
		std::memcpy(this->lowerPoint, lower, sizeof(*lower) * D);
	}
	void setUpper(const int *const upper) {
		std::memcpy(this->upperPoint, upper, sizeof(*upper) * D);
	}

	void extend(const Mbr *const mbr) {
		this->extend(mbr->lowerPoint, mbr->upperPoint);
	}
	void extend(const int *const lower, const int *const upper) { 
		for (int i = 0; i < D; i++) {
			if (this->lowerPoint[i] > lower[i])
				this->lowerPoint[i] = lower[i];
			if (this->upperPoint[i] < upper[i])
				this->upperPoint[i] = upper[i];
		}
	}
	void extendLower(const int *const lower) {
		for (int i = 0; i < D; i++) {
			if (this->lowerPoint[i] > lower[i])
				this->lowerPoint[i] = lower[i];
		}
	}
	void extendUpper(const int *const upper) {
		for (int i = 0; i < D; i++) {
			if (this->upperPoint[i] > upper[i])
				this->upperPoint[i] = upper[i];
		}
	}

	bool overlap(const Mbr *const mbr) const {
		return this->overlap(mbr->lowerPoint, mbr->upperPoint);
	}
	bool overlap(const int *lower, const int *upper) const { 
		for (int i = 0; i < D; i++) {
			if (this->upperPoint[i] < lower[i] || upper[i] < this->lowerPoint[i])
				return false;
		}
		return true;
	}

	int getArea(void) {
		int area = 1;

		for (int i = 0; i < D; i++) {
			area *= this->upperPoint[i] - this->lowerPoint[i];
		}
		return area;
	}

private: /* member */
	int lowerPoint[D];
	int upperPoint[D];
};

#endif //MBR_CLASS
