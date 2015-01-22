/*
	Author - Sambuddha Basu
	Roll Number - 201301141
	Graphics Assignment 1
	Carrom game implemented using OpenGL.
 */
#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdio>
#ifdef __APPLE__
	#include <GLUT/glut.h>
	#include <CoreFoundation/CoreFoundation.h>
#else
	#include <GL/glut.h>
	#include <irrKlang.h>
#endif
#include "BasicCircle.h"
#include "Pocket.h"
#include "Striker.h"
#include "Coin.h"
#include "AudioPlayer.h"
using namespace std;
#ifndef __APPLE__
	using namespace irrklang;
#endif

#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define RAD2DEG(rad) (rad * 180 / PI)

// Global variables.
int GAME_WINDOW_HEIGHT = 600;
int GAME_WINDOW_WIDTH = 600;
int STRIKER_ANGLE = 90;
int PLAYER_1_SCORE = 30;
int PLAYER_2_SCORE = 30;
float STRIKER_MOVE_AMOUNT = 0.01f;
float STRIKER_POWER_AMOUNT = 0.01f;
#ifdef __APPLE__
	float STRIKER_HIT_VELOCITY = 0.03f;
	float FRICTION = 0.0001f;
#else
	float STRIKER_HIT_VELOCITY = 0.01f;
	float FRICTION = 0.00001f;
#endif
float PLAYER_1_STRIKER[] = {0.0f, -0.6f + STRIKER_RADIUS};
float PLAYER_2_STRIKER[] = {0.0f, 0.6f - STRIKER_RADIUS};
float STRIKER_POWER_X = 0.0f;
float STRIKER_POWER_Y = 0.0f;
float STRIKER_POWER = 0.75f;
bool PLAYER_TURN = true;
bool MOVEMENT = false;
bool MOUSE_PRESS = false;
time_t PLAYER_1_TIME = 0, PLAYER_2_TIME = 0, PREV_TIME, CURR_TIME;
GLuint texture;

// Function definitions.
void init_game_window(void);
void init_coins(void);
void render_game(void);
void handle_key_press(unsigned char key, int x, int y);
void handle_special_key_press(int key, int x, int y);
void handle_mouse_press(int button, int state, int x, int y);
void draw_board_boundary(void);
void draw_board_pockets(void);
void draw_board_coins(void);
void draw_board_striker(void);
void draw_board_text(void);
void check_pockets(void);
void next_turn(void);
GLuint LoadBMP(const char *fileName);

Pocket pockets[4];
Striker striker;
Coin coins[7];
bool coin_collision[7];

int main(int argc, char *argv[]) {
	// Play the music file.
	#ifdef __APPLE__
		const char *fn = "music.wav";
		AudioPlayer* ap = AudioPlayer::file(fn);
		if(!ap) {
			cerr<<"Error loading audio"<<endl;
			return 1;
		}
		ap->play();
	#else
		ISoundEngine* engine = createIrrKlangDevice();
		if(!engine) {
			cerr<<"Could not startup engine"<<endl;
			return 1;
		}
		engine->play2D("music.wav", true);
	#endif
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(GAME_WINDOW_HEIGHT, GAME_WINDOW_WIDTH);
	glutCreateWindow("Carrom");
	init_game_window();
	init_coins();
	glutDisplayFunc(render_game);
	glutIdleFunc(render_game);
	glutKeyboardFunc(handle_key_press);
	glutSpecialFunc(handle_special_key_press);
	glutMouseFunc(handle_mouse_press);
	PREV_TIME = CURR_TIME = time(0);
	glutMainLoop();
	return 0;
}

/* LoadBMP loads the image file to be used as a texture */
GLuint LoadBMP(const char *fileName) {
	FILE *file;
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int size;
	unsigned int width, height;
	unsigned char *data;
	file = fopen(fileName, "rb");
	if(file == NULL) {
		//MessageBox(NULL, L"Error: Invaild file path!", L"Error", MB_OK);
		return false;
	}
	if(fread(header, 1, 54, file) != 54) {
		//MessageBox(NULL, L"Error: Invaild file!", L"Error", MB_OK);
		return false;
	}
	if(header[0] != 'B' || header[1] != 'M') {
		//MessageBox(NULL, L"Error: Invaild file!", L"Error", MB_OK);
		return false;
	}
	dataPos     = *(int*)&(header[0x0A]);
	size        = *(int*)&(header[0x22]);
	width       = *(int*)&(header[0x12]);
	height      = *(int*)&(header[0x16]);
	if(size == NULL)
		size = width * height * 3;
	if(dataPos == NULL)
		dataPos = 54;
	data = new unsigned char[size];
	fread(data, 1, size, file);
	fclose(file);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	return texture;
}

/* init_game_window sets the initial game properties */
void init_game_window(void) {
	glEnable(GL_COLOR_MATERIAL);
	glClearColor(0.75f, 0.45f, 0.0f, 1.0f);
	texture = LoadBMP("texture.bmp");
}

/* init_coins sets the properties of the coins on the board */
void init_coins(void) {
	coins[0].set_color(1.0f, 0.0f, 0.0f);
	coins[1].set_color(1.0f, 1.0f, 1.0f);
	coins[2].set_color(0.0f, 0.0f, 0.0f);
	coins[3].set_color(1.0f, 1.0f, 1.0f);
	coins[4].set_color(0.0f, 0.0f, 0.0f);
	coins[5].set_color(1.0f, 1.0f, 1.0f);
	coins[6].set_color(0.0f, 0.0f, 0.0f);

	coins[0].set_coordinates(0.0f, 0.0f);
	coins[1].set_coordinates(-0.1f, 0.0f);
	coins[2].set_coordinates(-0.1f * cos(DEG2RAD(60)), 0.1f * sin(DEG2RAD(60)));
	coins[3].set_coordinates(0.1f * cos(DEG2RAD(60)), 0.1f * sin(DEG2RAD(60)));
	coins[4].set_coordinates(0.1f, 0.0f);
	coins[5].set_coordinates(0.1f * cos(DEG2RAD(60)), -0.1f * sin(DEG2RAD(60)));
	coins[6].set_coordinates(-0.1f * cos(DEG2RAD(60)), -0.1f * sin(DEG2RAD(60)));
}

/* render_game runs throughout the game and calls the other required functions for the game */
void render_game(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Draw carrom board boundary lines.
	draw_board_boundary();
	// Draw the pockets.
	draw_board_pockets();
	// Draw board coins.
	draw_board_coins();
	// Draw the striker.
	draw_board_striker();
	check_pockets();
	draw_board_text();
	glutSwapBuffers();
}

/* handle_mouse_press handles the mouse clicks */
void handle_mouse_press(int button, int state, int x, int y) {
	float current_x, current_y;
	x -= 300;
	y -= 300;
	current_x = x / 300.0;
	current_y = -y / 300.0;
	if(button == GLUT_RIGHT_BUTTON) {
		if(state == GLUT_DOWN) {
			if((current_x > striker.x - STRIKER_RADIUS) && (current_x < striker.x + STRIKER_RADIUS) && (current_y > striker.y - STRIKER_RADIUS) && (current_y < striker.y + STRIKER_RADIUS) && (!MOVEMENT)) {
				MOUSE_PRESS = true;
			}
		}
		else if(state == GLUT_UP && MOUSE_PRESS) {
			if(current_x > (0.6f - STRIKER_RADIUS))
				current_x = 0.6f - STRIKER_RADIUS;
			else if(current_x < (-0.6f + STRIKER_RADIUS))
				current_x = -0.6f + STRIKER_RADIUS;
			if(PLAYER_TURN)
				PLAYER_1_STRIKER[0] = current_x;
			else
				PLAYER_2_STRIKER[0] = current_x;
			MOUSE_PRESS = false;
		}
	}
	else if(button == GLUT_LEFT_BUTTON) {
		if(state == GLUT_DOWN) {
			int degree;
			float power;
			power = hypot(current_x - striker.x, current_y - striker.y);
			current_x -= striker.x;
			current_y -= striker.y;
			degree = RAD2DEG(atan(current_x / current_y));
			degree = 90 - degree;
			if(!PLAYER_TURN)
				degree += 180;
			STRIKER_ANGLE = degree;
			if(power > 0.75f)
				STRIKER_POWER = 0.75f;
			else if(power < 0.5f)
				STRIKER_POWER = 0.5f;
			else
				STRIKER_POWER = power;
		}
		else if(state == GLUT_UP) {
			striker.velocity = STRIKER_HIT_VELOCITY * STRIKER_POWER;
			MOVEMENT = true;
		}
	}
}

/* handle_key_press handles the key presses */
void handle_key_press(unsigned char key, int x, int y) {
	if(key == 27) {
		exit(0);
	}
	else if(key == 32) {
		striker.velocity = STRIKER_HIT_VELOCITY * STRIKER_POWER;
		MOVEMENT = true;
	}
	else if(key == 97) {
		if((STRIKER_ANGLE < 180 && PLAYER_TURN) || (STRIKER_ANGLE < 360 && !PLAYER_TURN))
			STRIKER_ANGLE += 2;
	}
	else if(key == 99) {
		if((STRIKER_ANGLE > 0 && PLAYER_TURN) || (STRIKER_ANGLE > 180 && !PLAYER_TURN))
			STRIKER_ANGLE -= 2;
	}
}

/* handle_special_key_press handles the special key presses */
void handle_special_key_press(int key, int x, int y) {
	if(key == GLUT_KEY_LEFT) {
		if(PLAYER_TURN && PLAYER_1_STRIKER[0] > (-0.6f + STRIKER_RADIUS)) {
			PLAYER_1_STRIKER[0] -= STRIKER_MOVE_AMOUNT;
		}
		else if(!PLAYER_TURN && PLAYER_2_STRIKER[0] > (-0.6f + STRIKER_RADIUS)) {
			PLAYER_2_STRIKER[0] -= STRIKER_MOVE_AMOUNT;
		}
	}
	else if(key == GLUT_KEY_RIGHT) {
		if(PLAYER_TURN && PLAYER_1_STRIKER[0] < (0.6f - STRIKER_RADIUS)) {
			PLAYER_1_STRIKER[0] += STRIKER_MOVE_AMOUNT;
		}
		else if(!PLAYER_TURN && PLAYER_2_STRIKER[0] < (0.6f - STRIKER_RADIUS)) {
			PLAYER_2_STRIKER[0] += STRIKER_MOVE_AMOUNT;
		}
	}
	else if(key == GLUT_KEY_UP) {
		if(STRIKER_POWER < 0.75f) {
			STRIKER_POWER += STRIKER_POWER_AMOUNT;
		}
	}
	else if(key == GLUT_KEY_DOWN) {
		if(STRIKER_POWER > 0.5f) {
			STRIKER_POWER -= STRIKER_POWER_AMOUNT;
		}
	}
}

/* next_turn sets the properties for the next turn of the game */
void next_turn(void) {
	PLAYER_TURN = !PLAYER_TURN;
	if(PLAYER_TURN) {
		PLAYER_1_STRIKER[0] = 0.0f;
		STRIKER_ANGLE = 90;
	}
	else {
		PLAYER_2_STRIKER[0] = 0.0f;
		STRIKER_ANGLE = 270;
	}
	STRIKER_POWER = 0.75f;
	MOVEMENT = false;
}

/* draw_board_text is responsible for drawing the text above the board */
void draw_board_text(void) {
	CURR_TIME = time(0);
	if(CURR_TIME - PREV_TIME >= 1) {
		PREV_TIME = CURR_TIME;
		if(PLAYER_TURN) {
			PLAYER_1_TIME++;
			PLAYER_1_SCORE--;
		}
		else {
			PLAYER_2_TIME++;
			PLAYER_2_SCORE--;
		}
	}
	int len, i;
	string output;
	char temp[50];
	output += "PLAYER 1 SCORE: ";
	sprintf(temp, "%d", PLAYER_1_SCORE);
	output += temp;
	output += ", PLAYER 2 SCORE: ";
	sprintf(temp, "%d", PLAYER_2_SCORE);
	output += temp;
	glRasterPos2f(-0.65f, 0.925f);
	len = output.size();
	for(i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, output[i]);
	}
}

/* draw_board_striker draws the striker on the board based on its movement */
void draw_board_striker(void) {
	if(MOVEMENT) {
		float current_x, current_y;
		current_x = striker.velocity * cos(DEG2RAD(STRIKER_ANGLE));
		current_y = striker.velocity * sin(DEG2RAD(STRIKER_ANGLE));
		current_x += striker.last_x;
		current_y += striker.last_y;
		striker.set_coordinates(current_x, current_y);
		striker.set_last_coordinates(current_x, current_y);
		striker.velocity -= FRICTION;
		bool coin_movement = false;
		for(int i = 0; i < 7; i++) {
			if(coins[i].velocity && coins[i].visible) {
				coin_movement = true;
				break;
			}
		}
		if(striker.velocity <= 0.0f) {
			striker.velocity = 0.0f;
			if(!coin_movement) {
				next_turn();
			}
		}
		if(abs(current_y) > 0.8f)
			STRIKER_ANGLE = -STRIKER_ANGLE;
		if(abs(current_x) > 0.8f) {
			STRIKER_ANGLE = 180 - STRIKER_ANGLE;
		}
		// Coin collision.
		float distance, ball1_velx, ball1_vely, ball2_velx, ball2_vely, normal_x, normal_y, tangent_x, tangent_y, ball1_normal, ball1_tangent, ball2_normal, ball2_tangent, final_ball1_normal, final_ball2_normal;
		for(int i = 0; i < 7; i++)
			coin_collision[i] = false;
		// Coin collision with the striker.
		for(int i = 0; i < 7; i++) {
			distance = hypot(striker.x - coins[i].x, striker.y - coins[i].y);
			if(distance <= STRIKER_RADIUS + COIN_RADIUS) {
				ball1_velx = striker.velocity * cos(DEG2RAD(STRIKER_ANGLE));
				ball1_vely = striker.velocity * sin(DEG2RAD(STRIKER_ANGLE));
				ball2_velx = coins[i].velocity * cos(DEG2RAD(coins[i].angle));
				ball2_vely = coins[i].velocity * sin(DEG2RAD(coins[i].angle));
				normal_x = (striker.x - coins[i].x) / distance;
				normal_y = (striker.y - coins[i].y) / distance;
				tangent_x = -normal_y;
				tangent_y = normal_x;
				ball1_normal = normal_x * ball1_velx + normal_y * ball1_vely;
				ball2_normal = normal_x * ball2_velx + normal_y * ball2_vely;
				ball1_tangent = tangent_x * ball1_velx + tangent_y * ball1_vely;
				ball2_tangent = tangent_x * ball2_velx + tangent_y * ball2_vely;
				final_ball1_normal = (ball1_normal * (striker.mass - coins[i].mass) + 2 * coins[i].mass * ball2_normal) / (striker.mass + coins[i].mass);
				final_ball2_normal = (ball2_normal * (coins[i].mass - striker.mass) + 2 * striker.mass * ball1_normal) / (striker.mass + coins[i].mass);
				ball1_velx = normal_x * final_ball1_normal + tangent_x * ball1_tangent;
				ball1_vely = normal_y * final_ball1_normal + tangent_y * ball1_tangent;
				ball2_velx = normal_x * final_ball2_normal + tangent_x * ball2_tangent;
				ball2_vely = normal_y * final_ball2_normal + tangent_y * ball2_tangent;
				striker.velocity = hypot(ball1_velx, ball1_vely);
				coins[i].velocity = hypot(ball2_velx, ball2_vely);
				STRIKER_ANGLE = RAD2DEG(atan2(ball1_vely, ball1_velx));
				coins[i].angle = RAD2DEG(atan2(ball2_vely, ball2_velx));
			}
			// Coin collision with the other coins.
			for(int j = 0; j < 7; j++) {
				distance = hypot(coins[i].x - coins[j].x, coins[i].y - coins[j].y);
				if((distance <= 2 * COIN_RADIUS) && (i != j) && !coin_collision[i] && !coin_collision[j]) {
					coin_collision[i] = true;
					coin_collision[j] = true;
					ball1_velx = coins[i].velocity * cos(DEG2RAD(coins[i].angle));
					ball1_vely = coins[i].velocity * sin(DEG2RAD(coins[i].angle));
					ball2_velx = coins[j].velocity * cos(DEG2RAD(coins[j].angle));
					ball2_vely = coins[j].velocity * sin(DEG2RAD(coins[j].angle));
					normal_x = (coins[i].x - coins[j].x) / distance;
					normal_y = (coins[i].y - coins[j].y) / distance;
					tangent_x = -normal_y;
					tangent_y = normal_x;
					ball1_normal = normal_x * ball1_velx + normal_y * ball1_vely;
					ball2_normal = normal_x * ball2_velx + normal_y * ball2_vely;
					ball1_tangent = tangent_x * ball1_velx + tangent_y * ball1_vely;
					ball2_tangent = tangent_x * ball2_velx + tangent_y * ball2_vely;
					final_ball1_normal = ball2_normal;
					final_ball2_normal = ball1_normal;
					ball1_velx = normal_x * final_ball1_normal + tangent_x * ball1_tangent;
					ball1_vely = normal_y * final_ball1_normal + tangent_y * ball1_tangent;
					ball2_velx = normal_x * final_ball2_normal + tangent_x * ball2_tangent;
					ball2_vely = normal_y * final_ball2_normal + tangent_y * ball2_tangent;
					coins[i].velocity = hypot(ball1_velx, ball1_vely);
					coins[j].velocity = hypot(ball2_velx, ball2_vely);
					coins[i].angle = 1.1 * RAD2DEG(atan2(ball1_vely, ball1_velx));
					coins[j].angle = 0.9 * RAD2DEG(atan2(ball2_vely, ball2_velx));
				}
			}
		}
		glPushMatrix();
		glTranslatef(striker.x, striker.y, 0.0f);
		glColor3f(striker.r, striker.g, striker.b);
		glBegin(GL_TRIANGLE_FAN);
		for(int i = 0; i < 360; i++) {
			glVertex2f(striker.radius * cos(DEG2RAD(i)), striker.radius * sin(DEG2RAD(i)));
		}
		glEnd();
		glPopMatrix();
	}
	else {
		float x1, x2, y1, y2;
		STRIKER_POWER_X = STRIKER_POWER * cos(DEG2RAD(STRIKER_ANGLE));
		STRIKER_POWER_Y = STRIKER_POWER * sin(DEG2RAD(STRIKER_ANGLE));
		if(PLAYER_TURN) {
			striker.set_coordinates(PLAYER_1_STRIKER[0], PLAYER_1_STRIKER[1]);
			striker.set_last_coordinates(PLAYER_1_STRIKER[0], PLAYER_1_STRIKER[1]);
			x1 = PLAYER_1_STRIKER[0];
			y1 = PLAYER_1_STRIKER[1];
		}
		else {
			striker.set_coordinates(PLAYER_2_STRIKER[0], PLAYER_2_STRIKER[1]);
			striker.set_last_coordinates(PLAYER_2_STRIKER[0], PLAYER_2_STRIKER[1]);
			x1 = PLAYER_2_STRIKER[0];
			y1 = PLAYER_2_STRIKER[1];
		}
		x2 = STRIKER_POWER_X + x1;
		y2 = STRIKER_POWER_Y + y1;
		glPushMatrix();
		glTranslatef(striker.x, striker.y, 0.0f);
		glColor3f(striker.r, striker.g, striker.b);
		glBegin(GL_TRIANGLE_FAN);
		for(int i = 0; i < 360; i++) {
			glVertex2f(striker.radius * cos(DEG2RAD(i)), striker.radius * sin(DEG2RAD(i)));
		}
		glEnd();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0.0f, 0.0f, 0.0f);
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINES);
		glVertex2f(x1, y1);
		glVertex2f(x2, y2);
		glEnd();
		glPopMatrix();
	}
}

/* check_pockets checks the pockets for the striker or the coins */
void check_pockets(void) {
	float distance;
	for(int i = 0; i < 4; i++) {
		distance = hypot(striker.x - pockets[i].x, striker.y - pockets[i].y);
		if(distance <= STRIKER_RADIUS + POCKET_RADIUS) {
			next_turn();
		}
		for(int j = 0; j < 7; j++) {
			distance = hypot(coins[j].x - pockets[i].x, coins[j].y - pockets[i].y);
			if((distance <= COIN_RADIUS + POCKET_RADIUS) && (coins[j].visible)) {
				coins[j].visible = false;
				coins[j].set_coordinates((1.8f / 6) * j - 0.9f, -0.95f);
				if(j == 0) {
					if(PLAYER_TURN)
						PLAYER_1_SCORE += 50;
					else
						PLAYER_2_SCORE += 50;
				}
				else if(j % 2) {
					if(PLAYER_TURN)
						PLAYER_1_SCORE += 10;
					else
						PLAYER_2_SCORE -= 5;
				}
				else {
					if(PLAYER_TURN)
						PLAYER_1_SCORE -= 5;
					else
						PLAYER_2_SCORE += 10;
				}
			}
		}
	}
	bool game_over = true;
	for(int i = 0; i < 7; i++) {
		if(coins[i].visible) {
			game_over = false;
			break;
		}
	}
	if(game_over) {
		if(PLAYER_1_SCORE > PLAYER_2_SCORE)
			cout<<"Player 1 won";
		else if(PLAYER_1_SCORE < PLAYER_2_SCORE)
			cout<<"Player 2 won";
		else if(PLAYER_1_SCORE == PLAYER_2_SCORE)
			cout<<"Match draw";
		cout<<endl;
		exit(0);
	}
}

/* draw_board_pockets draws the pockets on the baord */
void draw_board_pockets(void) {
	pockets[0].set_coordinates(-0.8f, -0.8f);
	pockets[1].set_coordinates(-0.8f, 0.8f);
	pockets[2].set_coordinates(0.8f, 0.8f);
	pockets[3].set_coordinates(0.8f, -0.8f);
	for(int i = 0; i < 4; i++) {
		glPushMatrix();
		glTranslatef(pockets[i].x, pockets[i].y, 0.0f);
		glColor3f(pockets[i].r, pockets[i].g, pockets[i].b);
		glBegin(GL_TRIANGLE_FAN);
			for(int j = 0 ; j < 360 ; j++) {
				glVertex2f(pockets[i].radius * cos(DEG2RAD(j)), pockets[i].radius * sin(DEG2RAD(j)));
			}
		glEnd();
		glPopMatrix();
	}
}

/* draw_board_coins draws the coins on the game board */
void draw_board_coins() {
	for(int i = 0; i < 7; i++) {
		if(coins[i].visible) {
			if(coins[i].velocity) {
				float current_x, current_y;
				current_x = coins[i].x + coins[i].velocity * cos(DEG2RAD(coins[i].angle));
				current_y = coins[i].y + coins[i].velocity * sin(DEG2RAD(coins[i].angle));
				coins[i].set_coordinates(current_x, current_y);
				coins[i].set_last_coordinates(current_x, current_y);
				coins[i].velocity -= FRICTION;
			}
			if(coins[i].velocity <= 0.0f) {
				coins[i].velocity = 0.0f;
			}
			if(abs(coins[i].y) > 0.8f)
				coins[i].angle = -coins[i].angle;
			if(abs(coins[i].x) > 0.8f)
				coins[i].angle = 180 - coins[i].angle;
			glPushMatrix();
			glTranslatef(coins[i].x, coins[i].y, 0.0f);
			glColor3f(coins[i].r, coins[i].g, coins[i].b);
			glBegin(GL_TRIANGLE_FAN);
				for(int j = 0; j < 360; j++) {
					glVertex2f(coins[i].radius * cos(DEG2RAD(j)), coins[i].radius * sin(DEG2RAD(j)));
				}
			glEnd();
			glPopMatrix();
		}
		else {
			glPushMatrix();
			glTranslatef(coins[i].x, coins[i].y, 0.0f);
			glColor3f(coins[i].r, coins[i].g, coins[i].b);
			glBegin(GL_TRIANGLE_FAN);
				for(int j = 0; j < 360; j++) {
					glVertex2f(coins[i].radius * cos(DEG2RAD(j)), coins[i].radius * sin(DEG2RAD(j)));
				}
			glEnd();
			glPopMatrix();
		}
	}
}

/* draw_board_boundary draws the board boundaries */
void draw_board_boundary(void) {
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	glColor3f(0.6f, 0.3f, 0.0f);
	glBegin(GL_QUADS);
		glVertex2f(-0.9f, 0.9f);
		glVertex2f(0.9f, 0.9f);
		glVertex2f(0.9f, -0.9f);
		glVertex2f(-0.9f, -0.9f);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.8f, 0.6f);
	glBegin(GL_QUADS);
		glVertex2f(-0.85f, 0.85f);
		glVertex2f(0.85f, 0.85f);
		glVertex2f(0.85f, -0.85f);
		glVertex2f(-0.85f, -0.85f);
	glEnd();
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0); glVertex2f(-0.6f, 0.6f);
	glTexCoord2i(0, 1); glVertex2f(0.6f, 0.6f);
	glTexCoord2i(1, 1); glVertex2f(0.6f, -0.6f);
	glTexCoord2i(1, 0); glVertex2f(-0.6f, -0.6f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.8f, 0.6f);
	glBegin(GL_QUADS);
	glVertex2f(-0.6f + 2 * STRIKER_RADIUS, 0.6f - 2 * STRIKER_RADIUS);
	glVertex2f(0.6f - 2 * STRIKER_RADIUS, 0.6f - 2 * STRIKER_RADIUS);
	glVertex2f(0.6f - 2 * STRIKER_RADIUS, -0.6f + 2 * STRIKER_RADIUS);
	glVertex2f(-0.6f + 2 * STRIKER_RADIUS, -0.6f + 2 * STRIKER_RADIUS);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
		glVertex2f(-0.6f, 0.6f);
		glVertex2f(0.6f, 0.6f);

		glVertex2f(0.6f, 0.6f);
		glVertex2f(0.6f, -0.6f);

		glVertex2f(0.6f, -0.6f);
		glVertex2f(-0.6f, -0.6f);

		glVertex2f(-0.6f, -0.6f);
		glVertex2f(-0.6f, 0.6f);

		glVertex2f(-0.6f + 2 * STRIKER_RADIUS, 0.6f - 2 * STRIKER_RADIUS);
		glVertex2f(0.6f - 2 * STRIKER_RADIUS, 0.6f - 2 * STRIKER_RADIUS);

		glVertex2f(0.6f - 2 * STRIKER_RADIUS, 0.6f - 2 * STRIKER_RADIUS);
		glVertex2f(0.6f - 2 * STRIKER_RADIUS, -0.6f + 2 * STRIKER_RADIUS);

		glVertex2f(0.6f - 2 * STRIKER_RADIUS, -0.6f + 2 * STRIKER_RADIUS);
		glVertex2f(-0.6f + 2 * STRIKER_RADIUS, -0.6f + 2 * STRIKER_RADIUS);

		glVertex2f(-0.6f + 2 * STRIKER_RADIUS, -0.6f + 2 * STRIKER_RADIUS);
		glVertex2f(-0.6f + 2 * STRIKER_RADIUS, 0.6f - 2 * STRIKER_RADIUS);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINE_LOOP);
		for(int j = 0 ; j < 360 ; j++) {
			glVertex2f(0.2f * cos(DEG2RAD(j)), 0.2f * sin(DEG2RAD(j)));
		}
	glEnd();
	glPopMatrix();
}
