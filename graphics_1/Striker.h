#ifndef STRIKER_H
#define STRIKER_H

float STRIKER_RADIUS = 0.04f;
class Striker: public BasicCircle {
	public:
		float velocity, last_x, last_y, mass;
		Striker() {
			this->radius = STRIKER_RADIUS;
			this->set_color(0.3f, 0.5f, 0.7f);
			this->velocity = 0.0f;
			this->mass = 7.5;
		}
		void set_last_coordinates(float last_x, float last_y) {
			this->last_x = last_x;
			this->last_y = last_y;
		}
};

#endif
