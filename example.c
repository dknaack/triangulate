#define _XOPEN_SOURCE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <glad.h>
#include <GLFW/glfw3.h>

#define TRIANGULATE_IMPLEMENTATION
#include "triangulate.h"

#define NPOINTS 200
#define PI 3.14159265358979323846


static GLFWwindow *window;
static int width = 800;
static int height = 600;
static const char *title = "Triangulate";

static const char *vs = "#version 330\n"
	"layout (location = 0) in vec2 pos;"
	"void main() {"
		"gl_Position = vec4(pos, 1.0, 1.0);"
	"}";

static const char *fs = "#version 330\n"
	"out vec4 frag_color;"
	"uniform vec4 color;"
	"void main() {"
		"frag_color = color;"
	"}";


static GLuint
create_shader(GLenum type, const char *src)
{
	GLuint shader = glCreateShader(type);
	char info[1024];
	int success;

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, sizeof(info), NULL, info);
		fprintf(stderr, "create_shader: %s\n", info);
		return 0;
	}

	return shader;
}

static GLuint
create_program(const char *vs, const char *fs)
{
	GLuint prog, vert, frag;
	char info[1024];
	int success;

	vert = create_shader(GL_VERTEX_SHADER, vs);
	frag = create_shader(GL_FRAGMENT_SHADER, fs);
	prog = glCreateProgram();

	glAttachShader(prog, vert);
	glAttachShader(prog, frag);
	glLinkProgram(prog);

	glDeleteShader(vert);
	glDeleteShader(frag);

	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(prog, sizeof(info), NULL, info);
		fprintf(stderr, "create_program: %s\n", info);
		return 0;
	}

	return prog;
}

static void
die(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

static void
randpoly(float *points, unsigned int npoints)
{
	float phi, r = 0.f;
	int i;

	for (i = 0; i < npoints; i++) {
		phi = 2 * PI * (float)i / (float)npoints;
		r += drand48();
		r /= 1.5f;

		points[2 * i + 0] = 0.5f * r * cosf(phi);
		points[2 * i + 1] = 0.5f * r * sinf(phi);
	}
}

int
main(void)
{
	unsigned int indices[3 * NPOINTS];
	float points[2 * NPOINTS];
	GLuint vao, vbo, ebo, prog;
	GLint color;

	srand48(time(NULL));
	randpoly(points, NPOINTS);
	triangulate(points, NPOINTS, indices);

	if (!glfwInit())
		die("glfwInit");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	if (!(window = glfwCreateWindow(width, height, title, NULL, NULL)))
		die("glfwCreateWindow");

	glfwMakeContextCurrent(window);

	if (!(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)))
		die("gladLoadGLLoader");

	prog = create_program(vs, fs);
	color = glGetUniformLocation(prog, "color");

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	while (!glfwWindowShouldClose(window)) {
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			randpoly(points, NPOINTS);
			triangulate(points, NPOINTS, indices);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);
		}

		glUseProgram(prog);
		glBindVertexArray(vao);
		glUniform4f(color, 1.f, 1.f, 1.f, 1.f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, 3 * (NPOINTS - 2), GL_UNSIGNED_INT, 0);

		glUniform4f(color, 0.f, 0.f, 0.f, 1.f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLES, 3 * (NPOINTS - 2), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(prog);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &vao);

	glfwDestroyWindow(window);
	glfwTerminate();
	
	return 0;
}
