/* Project 2B- transforms using matrix stack built on glm 
	CPE 471 Cal Poly Z. Wood + S. Sueda
*/
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

/* to use glee */
#define GLEE_OVERWRITE_GL_FUNCTIONS
#include "glee.hpp"

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
shared_ptr<Program> prog1; // Snowman program
shared_ptr<Program> prog2; // Snow program
shared_ptr<Program> prog3; // Globe Program
shared_ptr<Shape> shape; 

GLuint VertexArrayID;
static const GLfloat g_vertex_buffer_data[] = {
-1.94f, 0.36f, -4.0f,
1.1f, 1.65f, -4.1f,
-1.4f, -1.21f, -4.2f,
0.08f, -1.56f, -4.3f,
-1.09f, -1.26f, -4.4f,
-1.18f, .39f, -4.5f,
1.31f, 1.29f, -4.4f,
.86f, 0.31f, -4.3f,
-0.73f, 1.86f, -4.2f,
-1.05f, 1.65f, -4.1f,
0.37f, -1.81f, -4.0f,
-1.29f, 1.38f, -4.1f,
1.08f, .44f, -4.2f,
0.64f, 1.87f, -4.3f,
-1.23f, -.35f, -4.4f,
1.45f, 1.46f, -4.5f,
-.62f, .91f, -4.4f,
-1.18f, 1.84f, -4.3f,
-1.35f, -1.65f, -4.2f,
1.75f, 0.44f, -4.1f,
1.1f, -1.01f, -4.0f,
-1.8f, 0.3f, -4.1f,
1.04f, -1.6f, -4.2f,
-1.17f, -1.64f, -4.3f,
1.74f, -1.63f, -4.4f,
1.01f, -1.06f, -4.5f,
-0.16f, -1.55f, -4.4f,
-1.54f, -1.67f, -4.3f,
1.27f, .27f, -4.2f,
1.83f, -0.78f, -4.1f,
1.21f, 1.05f, -4.0f,
1.23f, 1.57f, -4.1f,
-1.34f, 1.45f, -4.2f,
-1.25f, -0.44f, -4.3f,
1.75f, 1.11f, -4.4f,
1.63f, 1.49f, -4.5f,
-1.57f, 1.69f, -4.4f,
-1.29f, 1.8f, -4.3f,
1.89f, .9f, -4.2f,
1.16f, -1.38f, -4.1f,
};

GLuint vertexbuffer; 
int g_width, g_height, flag;
double keyRotate;
float sTheta, snowTheta, gTheta;


static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	/*if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}*/
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		keyRotate = -.01;
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		keyRotate = .01;
	}
	else if (action == GLFW_RELEASE) {
		keyRotate = 0;
	}
}


static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
   double posX, posY;
   if (action == GLFW_PRESS) {
      glfwGetCursorPos(window, &posX, &posY);
      cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
	}
}

static void resize_callback(GLFWwindow *window, int width, int height) {
    g_width = width;
    g_height = height;
    glViewport(0, 0, width, height);
}

static void initGeom() {
	//generate the VAO
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	//generate vertex buffer to hand off to OGL
	glGenBuffers(1, &vertexbuffer);
	//set the current state to focus on our vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	//actually memcopy the data - only do this once
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);
}

static void init()
{
	GLSL::checkVersion();

	sTheta = 0;
	snowTheta = 0;
	flag = 0;		// Flag for shoulder rotation 
	keyRotate = 0;	// Sets rotation angle increment value based on 'a' or 'd' being pressed
	gTheta = 0;		// Global y-rotation value 

	// Set background color.
	glClearColor(.12f, .34f, .56f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Initialize mesh.
	shape = make_shared<Shape>();
	shape->loadMesh(RESOURCE_DIR + "sphere.obj");
	shape->resize();
	shape->init();

	// Initialize the GLSL program (SNOWMAN).
	prog1 = make_shared<Program>();
	prog1->setVerbose(true);
	prog1->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
	prog1->init();
	prog1->addUniform("P");
	prog1->addUniform("MV");
	prog1->addAttribute("vertPos");
	prog1->addAttribute("vertNor");
	prog1->addUniform("colorMode");

	// Initialize the GLSL program (SNOW).
	prog2 = make_shared<Program>();
	prog2->setVerbose(true);
	prog2->setShaderNames(RESOURCE_DIR + "snow_vert.glsl", RESOURCE_DIR + "snow_frag.glsl");
	prog2->init();
	prog2->addUniform("P");
	prog2->addUniform("MV");
	prog2->addAttribute("vertPos");
	prog2->addUniform("W");
	prog2->addUniform("H");
	prog2->addUniform("T");

	// Initialize the GLSL program (GLOBE).
	prog3 = make_shared<Program>();
	prog3->setVerbose(true);
	prog3->setShaderNames(RESOURCE_DIR + "globe_vert.glsl", RESOURCE_DIR + "globe_frag.glsl");
	prog3->init();
	prog3->addUniform("P");
	prog3->addUniform("MV");
	prog3->addAttribute("vertPos");
	prog3->addAttribute("vertNor");
}

static void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float aspect = width/(float)height;


    auto P = make_shared<MatrixStack>();
    auto MV = make_shared<MatrixStack>();
    auto colorMode = 0;
    auto W = width;
	auto H = height;
	float T = glfwGetTime();

    P->pushMatrix();
    P->perspective(45.0f, aspect, 0.01f, 100.0f);

// PROGRAM 1 - SNOWMAN 
	prog1->bind();
	glUniformMatrix4fv(prog1->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));

    MV->pushMatrix();
     	MV->loadIdentity();
		//draw 'global transforms'
	    // Center - Body
		MV->translate(vec3(0, 0, -5.5));
		MV->rotate(gTheta, vec3(0, 1, 0));
	  	MV->scale(vec3(1, .9, 1));
	  	colorMode = 0;
	  	glUniformMatrix4fv(prog1->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
	  	glUniform1f(prog1->getUniform("colorMode"), colorMode);
	  	shape->draw(prog1);

	  	// Left Arm
	  	MV->pushMatrix();
	  		MV->translate(vec3(-.9, 0, .8));
	  		MV->scale(vec3(.5, .03, .02));
	  		colorMode = 1; 
	  		glUniformMatrix4fv(prog1->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
	  		glUniform1f(prog1->getUniform("colorMode"), colorMode);
	  		shape->draw(prog1);
		MV->popMatrix();

	  	// Right Arm
	  	MV->pushMatrix();
	  		MV->translate(vec3(.5, 0, .8));
	  		MV->rotate(sTheta, vec3(0, 0, 1));
	  		MV->translate(vec3(.5, 0, 0));
	  		MV->scale(vec3(.5, .03, .02));
	  		colorMode = 1;
	  		glUniformMatrix4fv(prog1->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
	  		glUniform1f(prog1->getUniform("colorMode"), colorMode);
	  		shape->draw(prog1);
		MV->popMatrix();

	  	// Bottom - Body
	  	MV->pushMatrix();
			MV->translate(vec3(0, -1, 0));
		  	MV->scale(vec3(1.25, 1.0, 1.2));
		  	colorMode = 0;
		  	glUniformMatrix4fv(prog1->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
		  	glUniform1f(prog1->getUniform("colorMode"), colorMode);
		  	shape->draw(prog1);
	  	MV->popMatrix();

	  	// Top Body
	  	MV->pushMatrix();
	  		MV->translate(vec3(0, 1, 0));
		  	MV->scale(vec3(.75, .7, .8));
		  	colorMode = 0;
	  		glUniformMatrix4fv(prog1->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
	  		glUniform1f(prog1->getUniform("colorMode"), colorMode);
		  	shape->draw(prog1);

		  	// Left Eye
		  	MV->pushMatrix();
		  		MV->translate(vec3(-.3, .05, .8));
		  		MV->scale(vec3(.09, .15, .1));
		  		colorMode = 1;
		  		glUniformMatrix4fv(prog1->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
		  		glUniform1f(prog1->getUniform("colorMode"), colorMode);
		  		shape->draw(prog1);
		  	MV->popMatrix();

		  	// Right Eye
		  	MV->pushMatrix();
		  		MV->translate(vec3(.3, .05, .8));
		  		MV->scale(vec3(.09, .15, .1));
		  		colorMode = 1;
		  		glUniformMatrix4fv(prog1->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
		  		glUniform1f(prog1->getUniform("colorMode"), colorMode);
		  		shape->draw(prog1);
		  	MV->popMatrix();
	  	MV->popMatrix();
    MV->popMatrix();
	prog1->unbind();

// PROGRAM 2 - SNOW
	/* Begin prog1 */
	// Draw the points using GLSL.
	prog2->bind();
	//send the matrices to the shaders
		MV->pushMatrix();
		MV->rotate(snowTheta, vec3(0, 0, 1));
		glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
		glUniform1i(prog2->getUniform("W"), W);
		glUniform1i(prog2->getUniform("H"), H);
		glUniform1f(prog2->getUniform("T"), T);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

		//actually draw from vertex 0, 3 vertices
		glPointSize(25.0f);
		glDrawArrays(GL_POINTS, 0, 40);
		glDisableVertexAttribArray(0);

		MV->popMatrix();
	prog2->unbind();

// PROGRAM 3 - GLOBE 
	prog3->bind();
		glUniformMatrix4fv(prog3->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
		MV->pushMatrix();
			MV->translate(vec3(0, 0, -5));
			MV->rotate(gTheta, vec3(0, 1, 0));
		  	MV->scale(vec3(3.5, 3, 3.5));
		  	glUniformMatrix4fv(prog3->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
		  	shape->draw(prog3);
		 MV->popMatrix();
	prog3->unbind();


    // Pop matrix stacks.
    P->popMatrix();

	/* update shoulder angle - animate */
	if (sTheta < .8 && flag == 0) {
		sTheta += 0.02;
		flag = 0;
	}
	else if (sTheta < 0){
		sTheta += 0.02;
		flag = 0;
	}
	else {
		sTheta -= .02;
		flag = 1;
	}

	// Update snow rotation angles
	snowTheta += .002;

	// Update global y-rotation angle
	gTheta += keyRotate;
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
   //request the highest possible version of OGL - important for mac
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "Chunk's Debut", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	//weird bootstrap of glGetError
   glGetError();
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
   cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
   //set the mouse call back
   glfwSetMouseButtonCallback(window, mouse_callback);
   //set the window resize call back
   glfwSetFramebufferSizeCallback(window, resize_callback);

	// Initialize scene. Note geometry initialized in init now
	init();
	initGeom();

	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
