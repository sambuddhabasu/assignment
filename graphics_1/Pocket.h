#ifndef POCKET_H
#define POCKET_H

float POCKET_RADIUS = 0.05f;
class Pocket: public BasicCircle {
	public:
		Pocket() {
			this->radius = POCKET_RADIUS;
		}
};

#endif
