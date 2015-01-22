#ifndef COIN_H
#define COIN_H

float COIN_RADIUS = 0.03f;
class Coin: public BasicCircle {
	public:
		int angle;
		float velocity, last_x, last_y, mass;
		bool visible;
		Coin() {
			this->radius = COIN_RADIUS;
			this->visible = true;
			this->angle = 0;
			this->velocity = 0.0f;
			this->mass = 5;
		}
		void set_last_coordinates(float last_x, float last_y) {
			this->last_x = last_x;
			this->last_y = last_y;
		}
};

#endif
