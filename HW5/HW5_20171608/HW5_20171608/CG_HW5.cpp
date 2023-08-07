#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shaders/LoadShaders.h"

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_POSITION 0
#define LOC_NORMAL 1

typedef struct light_info {
	glm::vec4 loc;
	glm::vec3 La;
	glm::vec3 L;
	glm::vec4 att;
	float cutoff;
	glm::vec3 spot_direction;
} light;

light light_arr[4];

glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ModelViewMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix;
glm::mat3 ModelViewMatrixInvTrans;

glm::vec4 Light_Position_EC[4];

// simple
GLuint h_ShaderProgram_simple, h_ShaderProgram_PS, h_ShaderProgram_GS;
GLint loc_ModelViewProjectionMatrix_simple, loc_primitive_color;

// phong
GLint loc_ModelViewProjectionMatrix_PS, loc_ModelViewMatrix_PS, loc_ModelViewMatrixInvTrans_PS;
GLint loc_Material_Ka, loc_Material_Kd, loc_Material_Ks, loc_Material_Shininess;
GLint loc_Light_Position[4], loc_Light_La[4], loc_Light_L[4], loc_Light_att[4];

// gouraud
GLint loc_ModelViewProjectionMatrix_GS, loc_ModelViewMatrix_GS, loc_ModelViewMatrixInvTrans_GS;
GLint loc_Material_Ka_GS, loc_Material_Kd_GS, loc_Material_Ks_GS, loc_Material_Shininess_GS;
GLint loc_Light_Position_GS[4], loc_Light_La_GS[4], loc_Light_L_GS[4], loc_Light_att_GS[4];

unsigned int timestamp_scene = 0;

void timer_scene(int value) {
	timestamp_scene = (timestamp_scene + 1) % UINT_MAX;
	glutPostRedisplay();
}

void init_light() {
	// world light
	light_arr[0].loc = glm::vec4(100.0f, 200.0f, 50.0f, 1.0f);
	light_arr[0].La = glm::vec3(0.0f, 0.0f, 0.0f);
	light_arr[0].L = glm::vec3(0.5f, 0.5f, 0.5f);
	light_arr[0].att = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	
	// red light
	light_arr[1].loc = glm::vec4(70.0f, 50.0f, 0.0f, 1.0f);
	light_arr[1].La = glm::vec3(0.0f, 0.0f, 0.0f);
	light_arr[1].L = glm::vec3(0.4f, 0.0f, 0.0f);
	light_arr[1].att = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// model light
	light_arr[2].loc = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	light_arr[2].La = glm::vec3(0.0f, 0.0f, 0.0f);
	light_arr[2].L = glm::vec3(0.0f, 1.0f, 0.0f);
	light_arr[2].att = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// cam light
	light_arr[3].loc = glm::vec4(0.0f, 60.0f, 0.0f, 1.0f);
	light_arr[3].La = glm::vec3(0.0f, 0.0f, 0.0f);
	light_arr[3].L = glm::vec3(0.0f, 0.0f, 1.0f);
	light_arr[3].att = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

// axes object
GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) { // draw coordinate axes
	// initialize vertex buffer object
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	// initialize vertex array object
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_axes(void) {
	// assume ShaderProgram_simple is used
	glBindVertexArray(axes_VAO);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
}

// floor
GLuint rectangle_VBO, rectangle_VAO;
GLfloat rectangle_vertices[12][3] = {  // vertices enumerated counterclockwise
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};

void prepare_floor(void) { // Draw coordinate axes.
   // Initialize vertex buffer object.
	glGenBuffers(1, &rectangle_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_vertices), &rectangle_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &rectangle_VAO);
	glBindVertexArray(rectangle_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void set_material_floor(void) {
	glUniform3f(loc_Material_Ka, 0.25f, 0.25f, 0.25f);
	glUniform3f(loc_Material_Kd, 0.4f, 0.4f, 0.4f);
	glUniform3f(loc_Material_Ks, 0.7745f, 0.7745f, 0.7745f);
	glUniform1f(loc_Material_Shininess, 78.6f);
}

void draw_floor(void) {
	glFrontFace(GL_CCW);

	glBindVertexArray(rectangle_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define INDEX_VERTEX_POSITION 0
#define INDEX_VERTEX_NORMAL 1

#define N_OBJECTS 2
#define OBJECT_DRAGON 0
#define OBJECT_TIGER 1

GLuint object_VBO[N_OBJECTS], object_VAO[N_OBJECTS];
int object_n_triangles[N_OBJECTS];
GLfloat* object_vertices[N_OBJECTS];

int read_triangular_mesh(GLfloat** object, int bytes_per_primitive, char* filename) {
	int n_triangles;
	FILE* fp;

	fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);
	*object = (float*)malloc(n_triangles * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

void set_up_object(int object_ID, char* filename, int n_bytes_per_vertex) {
	object_n_triangles[object_ID] = read_triangular_mesh(&object_vertices[object_ID],
		3 * n_bytes_per_vertex, filename);
	// Error checking is needed here.

	// Initialize vertex buffer object.
	glGenBuffers(1, &object_VBO[object_ID]);

	glBindBuffer(GL_ARRAY_BUFFER, object_VBO[object_ID]);
	glBufferData(GL_ARRAY_BUFFER, object_n_triangles[object_ID] * 3 * n_bytes_per_vertex,
		object_vertices[object_ID], GL_STATIC_DRAW);

	// As the geometry data exists now in graphics memory, ...
	free(object_vertices[object_ID]);

	// Initialize vertex array object.
	glGenVertexArrays(1, &object_VAO[object_ID]);
	glBindVertexArray(object_VAO[object_ID]);

	glBindBuffer(GL_ARRAY_BUFFER, object_VBO[object_ID]);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
	glVertexAttribPointer(INDEX_VERTEX_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_object(int object_ID) {
	glBindVertexArray(object_VAO[object_ID]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * object_n_triangles[object_ID]);
	glBindVertexArray(0);
}

// dragon

void prepare_dragon() {
	set_up_object(OBJECT_DRAGON, "Data/static_objects/dragon_vnt.geom", 8 * sizeof(float));
}

void set_material_dragon(void) {
	glUniform3f(loc_Material_Ka, 0.1745f, 0.1175f, 0.1175f);
	glUniform3f(loc_Material_Kd, 0.6142f, 0.0413f, 0.0413f);
	glUniform3f(loc_Material_Ks, 0.7278f, 0.6269f, 0.6269f);
	glUniform1f(loc_Material_Shininess, 76.8f);
}

float dragon_angle = 0;
void draw_dragon() {
	glUseProgram(h_ShaderProgram_PS);
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	glm::mat4 ModelMatrix_L = glm::mat4(1.0f);

	if (dragon_angle > 360) dragon_angle = 0.0f;
	else dragon_angle += 0.25f;
	set_material_dragon();
	ModelMatrix = glm::rotate(ModelMatrix, dragon_angle * TO_RADIAN, glm::vec3(0, 1, 0));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, 20, 100));
	ModelMatrix = glm::rotate(ModelMatrix, -90 * TO_RADIAN, glm::vec3(1, 0, 0));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2, 2, 2));

	ModelViewMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_object(OBJECT_DRAGON);

	ModelMatrix_L = glm::rotate(ModelMatrix_L, dragon_angle * TO_RADIAN, glm::vec3(0, 1, 0));
	ModelMatrix_L = glm::translate(ModelMatrix_L, glm::vec3(0, 20, 100));

	glm::mat4 ModelViewMatrix_L = ViewMatrix * ModelMatrix_L;
	Light_Position_EC[2] = ModelViewMatrix_L * light_arr[2].loc;
	glUniform4fv(loc_Light_Position[2], 1, &Light_Position_EC[2][0]);
	glUseProgram(h_ShaderProgram_GS);
	glUniform4fv(loc_Light_Position_GS[2], 1, &Light_Position_EC[2][0]);
	glUseProgram(h_ShaderProgram_PS);

	glutPostRedisplay();
}

// tiger

void prepare_tiger() {
	set_up_object(OBJECT_TIGER, "Data/Tiger_00_triangles_vnt.geom", 8 * sizeof(float));
}

void set_material_tiger() {
	glUniform3f(loc_Material_Ka_GS, 0.0215f, 0.1745f, 0.0215f);
	glUniform3f(loc_Material_Kd_GS, 0.0756f, 0.6142f, 0.0756f);
	glUniform3f(loc_Material_Ks_GS, 0.633f, 0.7278f, 0.633f);
	glUniform1f(loc_Material_Shininess_GS, 76.8f);
}

void draw_tiger() {
	glUseProgram(h_ShaderProgram_GS);
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	set_material_tiger();

	ModelMatrix = glm::rotate(ModelMatrix, -90 * TO_RADIAN, glm::vec3(1, 0, 0));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));

	ModelViewMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_object(OBJECT_TIGER);
	glutPostRedisplay();
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// call drawing function

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	draw_axes();
	glLineWidth(1.0f);

	glUseProgram(h_ShaderProgram_PS);
	set_material_floor();
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-250.0f, 0.0f, 250.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(500.0f, 500.0f, 500.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_floor();

	draw_dragon();
	draw_tiger();

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 27) { // ESC key
		glutLeaveMainLoop(); // incur destuction callback for cleanups.
		return;
	}
	switch (key) {
	case 'W':
	case 'w':
		if (light_arr[0].L != glm::vec3(0.0f, 0.0f, 0.0f))
			light_arr[0].L = glm::vec3(0.0f, 0.0f, 0.0f);
		else
			light_arr[0].L = glm::vec3(0.5f, 0.5f, 0.5f);
		glUseProgram(h_ShaderProgram_PS);
		glUniform3f(loc_Light_L[0], light_arr[0].L.x, light_arr[0].L.y, light_arr[0].L.z);
		glUseProgram(h_ShaderProgram_GS);
		glUniform3f(loc_Light_L_GS[0], light_arr[0].L.x, light_arr[0].L.y, light_arr[0].L.z);
		break;
	case 'M':
	case 'm':
		if (light_arr[2].L != glm::vec3(0.0f, 0.0f, 0.0f))
			light_arr[2].L = glm::vec3(0.0f, 0.0f, 0.0f);
		else
			light_arr[2].L = glm::vec3(0.0f, 1.0f, 0.0f);
		glUseProgram(h_ShaderProgram_PS);
		glUniform3f(loc_Light_L[2], light_arr[2].L.x, light_arr[2].L.y, light_arr[2].L.z);
		glUseProgram(h_ShaderProgram_GS);
		glUniform3f(loc_Light_L_GS[2], light_arr[2].L.x, light_arr[2].L.y, light_arr[2].L.z);
		break;
	}
	glutPostRedisplay();
}

void special(int key, int x, int y) {
}

void reshape(int width, int height) {
	float window_aspect_ratio;
	glViewport(0, 0, width, height);

	window_aspect_ratio = (float)width / height;
	ProjectionMatrix = glm::perspective(45.0f * TO_RADIAN, window_aspect_ratio, 10.0f, 1000.0f);

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &rectangle_VAO);
	glDeleteBuffers(1, &rectangle_VBO);

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


void initialize_OpenGL(void) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	init_light();
	ViewMatrix = glm::lookAt(glm::vec3(200.0f, 300.0f, 50.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	Light_Position_EC[0] = ViewMatrix * light_arr[0].loc;
	Light_Position_EC[1] = ViewMatrix * light_arr[1].loc;
	Light_Position_EC[3] = light_arr[3].loc;

	char temp[50];
	glUseProgram(h_ShaderProgram_PS);
	for (int i = 0; i < 4; i++) {
		glUniform4fv(loc_Light_Position[i], 1, &Light_Position_EC[i][0]);
		glUniform3f(loc_Light_La[i], light_arr[i].La.x, light_arr[i].La.y, light_arr[i].La.z);
		glUniform3f(loc_Light_L[i], light_arr[i].L.x, light_arr[i].L.y, light_arr[i].L.z);
		glUniform4f(loc_Light_att[i], light_arr[i].att.x, light_arr[i].att.y, light_arr[i].att.z, light_arr[i].att.w);
	}

	glUseProgram(h_ShaderProgram_GS);
	for (int i = 0; i < 4; i++) {
		glUniform4fv(loc_Light_Position_GS[i], 1, &Light_Position_EC[i][0]);
		glUniform3f(loc_Light_La_GS[i], light_arr[i].La.x, light_arr[i].La.y, light_arr[i].La.z);
		glUniform3f(loc_Light_L_GS[i], light_arr[i].L.x, light_arr[i].L.y, light_arr[i].L.z);
		glUniform4f(loc_Light_att_GS[i], light_arr[i].att.x, light_arr[i].att.y, light_arr[i].att.z, light_arr[i].att.w);
	}
	
	glUseProgram(0);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_floor();
	prepare_dragon();
	prepare_tiger();
}

void prepare_shader_program(void) {
	ShaderInfo shader_info_simple[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_PS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/phong.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/phong.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_GS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/gouraud.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/gouraud.frag" },
		{ GL_NONE, NULL }
	};
	// simple
	h_ShaderProgram_simple = LoadShaders(shader_info_simple);
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");
	loc_ModelViewProjectionMatrix_simple = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");

	// phong
	h_ShaderProgram_PS = LoadShaders(shader_info_PS);
	loc_ModelViewProjectionMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "MVP");
	loc_ModelViewMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_PS = glGetUniformLocation(h_ShaderProgram_PS, "NormalMatrix");

	char temp[50];
	for (int i = 0; i < 4; i++) {
		sprintf(temp, "light_position[%d]", i);
		loc_Light_Position[i] = glGetUniformLocation(h_ShaderProgram_PS, temp);
		sprintf(temp, "light_La[%d]", i);
		loc_Light_La[i] = glGetUniformLocation(h_ShaderProgram_PS, temp);
		sprintf(temp, "light_L[%d]", i);
		loc_Light_L[i] = glGetUniformLocation(h_ShaderProgram_PS, temp);
		sprintf(temp, "light_att[%d]", i);
	}

	loc_Material_Ka = glGetUniformLocation(h_ShaderProgram_PS, "Material.Ka");
	loc_Material_Kd = glGetUniformLocation(h_ShaderProgram_PS, "Material.Kd");
	loc_Material_Ks = glGetUniformLocation(h_ShaderProgram_PS, "Material.Ks");
	loc_Material_Shininess = glGetUniformLocation(h_ShaderProgram_PS, "Material.Shininess");

	// gouraud
	h_ShaderProgram_GS = LoadShaders(shader_info_GS);
	loc_ModelViewProjectionMatrix_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrixInvTrans");

	loc_Material_Ka_GS = glGetUniformLocation(h_ShaderProgram_GS, "ambient_color");
	loc_Material_Kd_GS = glGetUniformLocation(h_ShaderProgram_GS, "diffuse_color");
	loc_Material_Ks_GS = glGetUniformLocation(h_ShaderProgram_GS, "specular_color");
	loc_Material_Shininess_GS = glGetUniformLocation(h_ShaderProgram_GS, "specular_exponent");

	for (int i = 0; i < 4; i++) {
		sprintf(temp, "u_Light_loc[%d]", i);
		loc_Light_Position_GS[i] = glGetUniformLocation(h_ShaderProgram_GS, temp);
		sprintf(temp, "u_Light_La[%d]", i);
		loc_Light_La_GS[i] = glGetUniformLocation(h_ShaderProgram_GS, temp);
		sprintf(temp, "u_Light_L[%d]", i);
		loc_Light_L_GS[i] = glGetUniformLocation(h_ShaderProgram_GS, temp);
		sprintf(temp, "u_Light_att[%d]", i);
		loc_Light_att_GS[i] = glGetUniformLocation(h_ShaderProgram_GS, temp);
	}
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
int main(int argc, char *argv[]) {
	char program_name[64] = "HW5";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: w, m" };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
	return 1;
}
