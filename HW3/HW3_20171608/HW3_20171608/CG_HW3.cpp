#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

// Begin of shader setup
#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}
// End of shader setup

// Begin of geometry setup
#include "CG_HW3.h"
// End of geometry setup

// Begin of Callback function definitions

// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

typedef struct _camera {
	glm::vec3 prp;
	glm::vec3 vrp;
	glm::vec3 vup;
	int is_fixed;
} camera;

glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix;

camera cam[5];

float tank_y = 0.0f;
float tank_y_offset = 0.05f;
float tank_angle = 0.0f;

float ironman_z = 0;

float godzilla_y = 0;

float dragon_x = 0, dragon_z = 0, dragon_angle = 0;

int stop_flag = FALSE;
int cam_num = 4;

unsigned int timestamp_scene = 0; // the global clock in the scene
int flag_tiger_animation, flag_polygon_fill;
int cur_frame_tiger = 0, cur_frame_ben = 0, cur_frame_wolf, cur_frame_spider = 0;
float rotation_angle_tiger = 0.0f;

void timer_scene(int value) {
	timestamp_scene = (timestamp_scene + 1) % UINT_MAX;
	cur_frame_tiger = timestamp_scene % N_TIGER_FRAMES;
	cur_frame_ben = timestamp_scene % N_BEN_FRAMES;
	cur_frame_wolf = timestamp_scene % N_WOLF_FRAMES;
	cur_frame_spider = timestamp_scene % N_SPIDER_FRAMES;
	rotation_angle_tiger = (timestamp_scene % 360) * TO_RADIAN;
	glutPostRedisplay();
	if(stop_flag == FALSE) glutTimerFunc(20, timer_scene, 0);
}

void init_cam() {
	cam_num = 4;
	cam[4].prp = glm::vec3(0.0f, 40.0f, 100.0f);
	cam[4].vrp = glm::vec3(0.0f, 0.0f, 0.0f);
	cam[4].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	cam[4].is_fixed = FALSE;
}

void display_cam() {
	cam[0].prp = glm::vec3(-20.0f, 15.0f, 30.0f);
	cam[0].vrp = glm::vec3(-10.0f, tank_y, 0.0f);
	cam[0].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	cam[0].is_fixed = TRUE;

	cam[1].prp = glm::vec3(50.0f, 100.0f, 80.0f);
	cam[1].vrp = glm::vec3(30.0f, 30.0f, 0.0f);
	cam[1].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	cam[1].is_fixed = TRUE;

	cam[2].prp = glm::vec3(-30.0f, -90.0f, 20.0f);
	cam[2].vrp = glm::vec3(-40.0f, -20.0f, 30.0f);
	cam[2].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	cam[2].is_fixed = TRUE;

	cam[3].prp = glm::vec3(-50.0f, 40.0f, 50.0f);
	cam[3].vrp = glm::vec3(dragon_x + 40, -20.0f, dragon_z);
	cam[3].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	cam[3].is_fixed = TRUE;
}

void change_cam() {
	display_cam();
	ViewMatrix = glm::lookAt(cam[cam_num].prp, cam[cam_num].vrp, cam[cam_num].vup);
}

void move_cam(float x, float y, float z) {
	if (cam[cam_num].is_fixed == TRUE) return;
	cam[cam_num].prp += glm::vec3(x, y, z);
	cam[cam_num].vrp += glm::vec3(x, y, z);
	ViewMatrix = glm::lookAt(cam[cam_num].prp, cam[cam_num].vrp, cam[cam_num].vup);
}

void move_vrp(float x, float y, float z) {
	if (cam[cam_num].is_fixed == TRUE) return;
	cam[cam_num].vrp += glm::vec3(x, y, z);
	ViewMatrix = glm::lookAt(cam[cam_num].prp, cam[cam_num].vrp, cam[cam_num].vup);
}

void draw_floor() {
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	glm::mat4 ViewModelMatrix;

	for (int i = 3; i >= 0; i--) {
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -5));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
		ModelMatrix = glm::rotate(ModelMatrix, 90 * i * TO_RADIAN, glm::vec3(0, 0, 1));
		ViewModelMatrix = ViewMatrix * ModelMatrix;
		ModelViewProjectionMatrix = ProjectionMatrix * ViewModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glLineWidth(3.0f);
		draw_object(OBJECT_SQUARE16, 0.0f / 255.0f, 120.0f / 255.0f, 0.0f / 255.0f);
		glLineWidth(1.0f);
	}
	glLineWidth(10.0f);
	draw_axes();
	glLineWidth(1.0f);
	glutPostRedisplay();
}

int cow_angle;
void draw_cow() {
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	glm::mat4 ViewModelMatrix;

	if(stop_flag == FALSE) cow_angle = (cow_angle + 1) % 360;

	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(50, 50, 0));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(cow_angle / 10, cow_angle / 10, cow_angle / 10));
	ModelMatrix = glm::rotate(ModelMatrix, cow_angle * TO_RADIAN, glm::vec3(0, 0, 1));
	ModelMatrix = glm::rotate(ModelMatrix, 90 * TO_RADIAN, glm::vec3(1, 0, 0));

	ViewModelMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ViewModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_object(OBJECT_COW, 0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f);
	glutPostRedisplay();
}

void draw_wolf(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(wolf_VAO);
	glDrawArrays(GL_TRIANGLES, wolf_vertex_offset[cur_frame_wolf], 3 * wolf_n_triangles[cur_frame_wolf]);
	glBindVertexArray(0);
}

float wolf_speed, wolf_x;
int wolf_angle;

void draw_moving_wolf(void) {
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	glm::mat4 ViewModelMatrix;

	if (stop_flag == FALSE) {
		if (wolf_x < 30) wolf_speed += 1;
		else if (wolf_x < 120) wolf_speed += 1.3;
		wolf_angle = (wolf_angle + 3) % 360;
	}
	if(wolf_x < 120) wolf_x = 0.002f * wolf_speed * wolf_speed;
	else {
		wolf_x = 0, wolf_speed = 0;
	}

	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(wolf_x, 0, 0));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-70, 40, 0));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(10, 10, 10));
	ModelMatrix = glm::rotate(ModelMatrix, wolf_angle * TO_RADIAN, glm::vec3(1, 0, 0));
	ModelMatrix = glm::rotate(ModelMatrix, 90 * TO_RADIAN, glm::vec3(0, 0, 1));
	ModelMatrix = glm::rotate(ModelMatrix, 90 * TO_RADIAN, glm::vec3(1, 0, 0));

	ViewModelMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ViewModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_wolf();
	glutPostRedisplay();
}

void draw_tiger(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);
}

int tiger_angle;
float tiger_dist, tiger_offset = 0.05f;

void draw_moving_tiger() {
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	glm::mat4 ViewModelMatrix;

	if (stop_flag == FALSE) {
		tiger_angle = (tiger_angle + 1) % 360;
		if (tiger_dist > 40) tiger_offset = -0.05f;
		else if (tiger_dist < 5) tiger_offset = 0.05f;
		tiger_dist += tiger_offset;
	}

	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(50, -40, 0));
	ModelMatrix = glm::rotate(ModelMatrix, (tiger_angle) * TO_RADIAN, glm::vec3(0, 0, 1));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-tiger_dist, 0, 0));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));

	ViewModelMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ViewModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_tiger();
	glutPostRedisplay();
}

void draw_ben(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(ben_VAO);
	glDrawArrays(GL_TRIANGLES, ben_vertex_offset[cur_frame_ben], 3 * ben_n_triangles[cur_frame_ben]);
	glBindVertexArray(0);
}

int phase = 0;
float ben_pos = 0;
void draw_moving_ben() {
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	glm::mat4 ViewModelMatrix;

	if (stop_flag == FALSE) {
		ben_pos += 0.1f;
		if (ben_pos > 20) {
			ben_pos = 0;
			phase = (phase + 1) % 4;
		}
	}

	switch (phase) {
	case 0:
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, -ben_pos, 0));
		break;
	case 1:
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(ben_pos, 0, 0));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, -20, 0));
		break;
	case 2:
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, ben_pos, 0));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(20, -20, 0));
		break;
	case 3:
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-ben_pos, 0, 0));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(20, 0, 0));
		break;
	}
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(20, 20, 0));
	for (int i = 0; i < phase; i++)
		ModelMatrix = glm::rotate(ModelMatrix, 90 * TO_RADIAN, glm::vec3(0, 0, 1));
	ModelMatrix = glm::rotate(ModelMatrix, -90 * TO_RADIAN, glm::vec3(1, 0, 0));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(20, 20, 20));

	ViewModelMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ViewModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_ben();
	glutPostRedisplay();
}

void draw_spider(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(spider_VAO);
	glDrawArrays(GL_TRIANGLES, spider_vertex_offset[cur_frame_spider], 3 * spider_n_triangles[cur_frame_spider]);
	glBindVertexArray(0);
}

int spider_angle;
void draw_moving_spider() {
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	glm::mat4 ViewModelMatrix;

	if (stop_flag == FALSE) {
		spider_angle = (spider_angle + 1) % 360;
	}

	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, 60, 30));
	ModelMatrix = glm::rotate(ModelMatrix, spider_angle * TO_RADIAN, glm::vec3(1, 0, 0));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, 0, 20));
	ModelMatrix = glm::rotate(ModelMatrix, 90 * TO_RADIAN, glm::vec3(1, 0, 0));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(10, 10, 10));

	ViewModelMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ViewModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_spider();
	glutPostRedisplay();
}

void draw_tank() {
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	glm::mat4 ViewModelMatrix;
	
	if (stop_flag == FALSE) {
		if (tank_y > 15) {
			if (tank_angle < 180) tank_angle++;
			else {
				tank_y_offset = -0.05f;
				tank_y += tank_y_offset;
			}
		}
		else if (tank_y < -15) {
			if (tank_angle > 0) tank_angle--;
			else {
				tank_y_offset = 0.05f;
				tank_y += tank_y_offset;
			}
		}
		else tank_y += tank_y_offset;
	}
	
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-10.0f, tank_y, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, (tank_angle + 180) * TO_RADIAN, glm::vec3(0, 0, 1));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, -10, 0));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.7f, 0.7f, 0.7f));

	ViewModelMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ViewModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_object(OBJECT_TANK, 50.0f / 255.0f, 0.0f / 255.0f, 50.0f / 255.0f);
	glutPostRedisplay();
}

void draw_ironman() {
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	glm::mat4 ViewModelMatrix;

	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, 0, (ironman_z * ironman_z * ironman_z * ironman_z * 0.00000001)));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-15, -20, 0));
	ModelMatrix = glm::rotate(ModelMatrix, ironman_z * 2 * TO_RADIAN, glm::vec3(0, 0, 1));
	ModelMatrix = glm::rotate(ModelMatrix, 90 * TO_RADIAN, glm::vec3(1, 0, 0));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2, 2, 2));

	if (stop_flag == FALSE) {
		if (ironman_z * ironman_z * ironman_z * ironman_z * 0.00000001 > 100) ironman_z = 0;
		else ironman_z++;
	}

	ViewModelMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ViewModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_object(OBJECT_IRONMAN, 20.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f);
	glutPostRedisplay();
}

void draw_dragon() {
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	glm::mat4 ViewModelMatrix;

	dragon_z = 20 * cos(dragon_x * TO_RADIAN * 6);
	dragon_angle = sin(dragon_x * TO_RADIAN * 6);

	if (stop_flag == FALSE) {
		if (dragon_x < -60) dragon_x = 40;
		else dragon_x -= 0.1f;
	}
	
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(dragon_x, 0, dragon_z));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(40, -20, 15));
	ModelMatrix = glm::rotate(ModelMatrix, dragon_angle, glm::vec3(0, 1, 0));
	ModelMatrix = glm::rotate(ModelMatrix, 180 * TO_RADIAN, glm::vec3(0, 0, 1));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));

	ViewModelMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ViewModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_object(OBJECT_DRAGON, 255.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f);
	glutPostRedisplay();
}

int zoom_level;
float window_aspect_ratio;

void change_perspective(int level) {
	if (cam[cam_num].is_fixed == TRUE) return;
	switch (level) {
	case 0:
		ProjectionMatrix = glm::perspective(30.0f * TO_RADIAN, window_aspect_ratio, 1.0f, 300.0f);
		break;
	case 1:
		ProjectionMatrix = glm::perspective(50.0f * TO_RADIAN, window_aspect_ratio, 1.0f, 300.0f);
		break;
	case 2:
		ProjectionMatrix = glm::perspective(70.0f * TO_RADIAN, window_aspect_ratio, 1.0f, 300.0f);
		break;
	case 3:
		ProjectionMatrix = glm::perspective(100.0f * TO_RADIAN, window_aspect_ratio, 1.0f, 300.0f);
		break;
	}
}

void draw_godzilla() {
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	glm::mat4 ViewModelMatrix;

	if (stop_flag == FALSE) {
		if (godzilla_y < -60) godzilla_y = 0;
		else godzilla_y -= 0.07f;
	}
	
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, godzilla_y, 0));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-40, 30, 0));
	ModelMatrix = glm::rotate(ModelMatrix, -90 * TO_RADIAN, glm::vec3(1, 0, 0));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(godzilla_y / 15.0f, godzilla_y / 15.0f, godzilla_y / 15.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.05f, 0.05f, 0.05f));

	ViewModelMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ViewModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_object(OBJECT_GODZILLA, 150.0f / 255.0f, 75.0f / 255.0f, 0.0f / 255.0f);
	glutPostRedisplay();
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// call drawing function

	draw_floor();
	draw_cow();
	draw_tank();
	draw_ironman();
	draw_dragon();
	draw_godzilla();
	draw_moving_wolf();
	draw_moving_spider();
	draw_moving_tiger();
	draw_moving_ben();
	change_cam();

	glutSwapBuffers();
}

int sign = -1;
float val = 10.0f;
void keyboard(unsigned char key, int x, int y) {
	if (key == 27) { // ESC key
		glutLeaveMainLoop(); // incur destuction callback for cleanups.
		return;
	}
	switch (key) {
	case '1':
		cam_num = 0;
		break;
	case '2':
		cam_num = 1;
		break;
	case '3':
		cam_num = 2;
		break;
	case '4':
		cam_num = 3;
		break;
	case '5':
		init_cam();
		cam_num = 4;
		break;
	case 'W':
	case 'w': // w
		move_cam(0, -1, 0);
		break;
	case 'S':
	case 's': // s
		move_cam(0, 1, 0);
		break;
	case 'A':
	case 'a': // a
		move_cam(1, 0, 0);
		break;
	case 'D':
	case 'd': // d
		move_cam(-1, 0, 0);
		break;
	case 'C':
	case 'c':
		move_cam(0, 0, -1);
		break;
	case 'F':
	case 'f':
		move_cam(0, 0, 1);
		break;
	case 'P':
	case 'p':
		stop_flag = !stop_flag;
		glutTimerFunc(20, timer_scene, 0);
		break;
	case 'Z':
	case 'z':
		change_perspective((++zoom_level) % 4);
		break;
	}
	glutPostRedisplay();
}

void special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		move_vrp(0, 0, 1);
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		move_vrp(0, 0, -1);
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		move_vrp(1, 0, 0);
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		move_vrp(-1, 0, 0);
		glutPostRedisplay();
		break;
	}
}

void reshape(int width, int height) {
	glViewport(0, 0, width, height);
	window_aspect_ratio = (float)width / height;
	ProjectionMatrix = glm::perspective(50.0f * TO_RADIAN, window_aspect_ratio, 1.0f, 300.0f);
	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &points_VAO);
	glDeleteBuffers(1, &points_VBO);

	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(N_OBJECTS, object_VAO);
	glDeleteBuffers(N_OBJECTS, object_VBO);
}
// End of callback function definitions

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);
	glutTimerFunc(20, timer_scene, 0);
}

#define PRINT_DEBUG_INFO  
void initialize_OpenGL(void) {
	glClearColor(150 / 255.0f, 150 / 255.0f, 150 / 255.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);

	init_cam();
	ViewMatrix = glm::lookAt(cam[cam_num].prp, cam[cam_num].vrp, cam[cam_num].vup);
}

void prepare_scene(void) {
	prepare_points();
	prepare_axes();
	prepare_square();
	prepare_cow();
	prepare_tank();
	prepare_dragon();
	prepare_godzila();
	prepare_ironman();
	prepare_wolf();
	prepare_spider();
	prepare_ben();
	prepare_tiger();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void print_message(const char * m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 3
void main(int argc, char *argv[]) {
	char program_name[64] = "CG_HW3";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 1, 2, 3, 4, 5, w, a, s, d, c, f, p, z",
		"    - Special key used: UP, DOWN, LEFT, RIGHT"
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1600, 800);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
