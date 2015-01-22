#ifndef BASIC_CIRCLE_H
#define BASIC_CIRCLE_H

class BasicCircle {
	public:
		float radius, r, g, b, x, y;
		BasicCircle() {
			this->radius = this->r = this->g = this->b = this->x = this->y = 0.0f;
		}
		void set_coordinates(float new_x, float new_y) {
			this->x = new_x;
			this->y = new_y;
		}
		void set_color(float new_r, float new_g, float new_b) {
			this->r = new_r;
			this->g = new_g;
			this->b = new_b;
		}
		void set_radius(float new_radius) {
			this->radius = new_radius;
		}
};

#endif
