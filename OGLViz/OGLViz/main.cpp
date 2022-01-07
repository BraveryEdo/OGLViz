#include "windows.h"
#include <stdio.h>
#include <GL\glew.h>
#include <GL\glut.h>

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutCreateWindow("GLEW Test");
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	fprintf(stdout, "Status: Using GLEW version: %s\n", glewGetString(GLEW_VERSION));

	return 0;
} 