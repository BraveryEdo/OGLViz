#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "GLM/glm/glm.hpp"
#include "GLM/glm/gtx/transform.hpp" 
#include "GLM/glm/gtc/matrix_transform.hpp" 
#include "GL/glut.h"
#include "shaders/shader.hpp"

//makes using GL Math (GLM) for vectors easier so a bunch of functions don't need glm:: prepended
using namespace glm;
GLFWwindow* window;

int initWindow() {
	//Boiler Plate setup for glfw, window, context, settings 
	GLfloat width = 1024.0f;
	GLfloat height = 768.0f;
	//initalize GL Frame Work (GLFW)
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return 0;
	}
	//GLFW initial settings
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

	// Open a window and create its OpenGL context
	 // (In the accompanying (tutorial) source code, this variable is global for simplicity)
	window = glfwCreateWindow(width, height, "LIQUIDSTATE", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version\n");
		glfwTerminate();
		return 0;
	}
	glfwMakeContextCurrent(window);

	return 1;
}

int initGlew() {
	// Initialize GL Eextension Wrangler (GLEW)
	glewExperimental = true; // Needed in older core profile, shouldn't be neccessary in newer vesion (if only using core profile)
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return 0;
	}
	return 1;
}

int getInput() {
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	return 1;
}


int main(int argc, char* argv[]) {
	fprintf(stdout, "Visualizer Project by LiquidState, C++ build utilizing OpenGL\n");
	
	if (!initWindow()) {
		fprintf(stdout, "Main window initialization failed\n");
		return -1;
	}
	
	if (!initGlew()) {
		fprintf(stdout, "Main glew initialization failed \n");
		return -1;
	}

	if (!getInput()) {
		fprintf(stdout, "Main input handler initialization failed\n");
		return -1;
	}
	
	// load in shaders
	GLuint programID = LoadShaders("..\\Include\\shaders\\vertexShader.txt", "..\\Include\\shaders\\fragmentShader.txt");

	// make sure its clear
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// An array of 3 vectors which represents 3 vertices
	static const GLfloat g_vertex_buffer_data[] = {
	   -1.0f, -1.0f, 0.0f,
	   1.0f, -1.0f, 0.0f,
	   0.0f,  1.0f, 0.0f,
	
	};
	// Vertex Array Obj (VAO) 
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	/* X Y Z plane 
	* right hand rules for convenience / by convention
	* +x is right
	* +y is up	
	* +z is out of the screen
	* origin (0,0,0) is at center of screen
	* ^^^ by defualt before any transforms
	*/


	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);


	//LOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOP ============================================================================
	do {
		// Clear the screen.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//tell gl which shaders to use, compiled on load, can be added to / edited on the fly
		glUseProgram(programID);

		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		//mat4 Projection = perspective(radians(45.0f), (float)4.0 / (float)3.0, 0.1f, 100.0f);
		//mat4 Projection = ortho(0.0f, 800.0f, 0.5f, 600.0f);
		// Or, for an ortho camera :
		mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

		// Camera matrix
		mat4 View = lookAt(
			vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
			vec3(0, 0, 0), // and looks at the origin
			vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

		// Model matrix : an identity matrix (model will be at the origin)
		mat4 Model = mat4(1.0f);
		// Our ModelViewProjection : multiplication of our 3 matrices
		mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

		// Get a handle for our "MVP" uniform
		// Only during the initialisation
		GLuint MatrixID = glGetUniformLocation(programID, "MVP");

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);
		
		mat4 myMatrix = glm::translate(glm::mat4(), glm::vec3(10.0f, 0.0f, 0.0f));
		vec4 myVector(10.0f, 10.0f, 10.0f, 0.0f);
		vec4 transformedVector = myMatrix * myVector; // guess the result

		//TransformedVector = TranslationMatrix * RotationMatrix * ScaleMatrix * OriginalVector;
		// c++
		// glm::mat4 myModelMatrix = myTranslationMatrix * myRotationMatrix * myScaleMatrix;
		// glm::vec4 myTransformedVector = myModelMatrix * myOriginalVector;
		// GL Shader Language (GLSL)
		// mat4 transform = mat2 * mat1;
		// vec4 out_vec = transform * in_vec;
		//ORDER OF OPS :  scale, rotate, transform

		// Swap buffers SCREEEN UPDATES HERE
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);


	return 0;
}

