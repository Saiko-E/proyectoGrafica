#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other libraries
#include "stb_image.h"

// GLM mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Model loading
#include "SOIL2/SOIL2.h"


// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// Function prototypes
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouseCallback(GLFWwindow *window, double xPos, double yPos);
void processMovement();
void updateAnimation();
bool loadWheelData(const char* filePath);

// Window dimensions
const GLuint windowWidth = 800, windowHeight = 600;
int framebufferWidth, framebufferHeight;

// Camera
Camera  camera(glm::vec3(12.0f, 7.0f, 30.0f));
GLfloat lastX = windowWidth / 2.0;
GLfloat lastY = windowHeight / 2.0;
bool pressedKeys[1024] = {};
bool firstMouse = true;
// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool isLightEnabled = false;

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.0f,2.0f, 0.0f),
	glm::vec3(0.0f,0.0f, 0.0f),
	glm::vec3(0.0f,0.0f,  0.0f),
	glm::vec3(0.0f,0.0f, 0.0f)
};

float cubeVertices[] = {
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};



glm::vec3 lightPulseFrequency = glm::vec3(0);
// Bus animation using a finite-state machine.
// Exported OBJ axes: X = sideways, Y = up, +Z = forward.
enum busState { waitingForStart, initialStraight, turning90, finalStraight, completed };
busState currentBusState = waitingForStart;
bool isAnimationRunning = false;

const float busPi = 3.14159265358979323846f;
const float busSpeed = 3.0f;        // model units per second
const float initialStraightDistance = 12.0f;          // initial straight segment distance
const float turnRadius = 9.0f;      // path radius (not the wheel radius)
const float finalStraightDistance = 10.0f;          // distance traveled after turning
const float turnArcLength = turnRadius * busPi * 0.5f;
const float totalRouteDistance = initialStraightDistance + turnArcLength + finalStraightDistance;

struct wheelData {
    glm::vec3 position;    // wheel center in the exported OBJ coordinate system
    float radius;           // physical tire radius
    bool isFrontWheel;        // whether the wheel steers during the turn
};
wheelData wheelDataArray[6];

float distanceTraveled = 0.0f;
glm::vec3 busPosition(0.0f);
float busHeading = 0.0f;             // radians: rotation around the Y axis
float steeringAngle = 0.0f;     // radians: steering around the vertical axis



// Delta time
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

int main()
{
	// Initialize GLFW
	glfwInit();
	// Set all the required options for GLFW
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);*/

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Bus - 90-degree turn animation (press N)", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

	// Register input callbacks
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);

	// GLFW options
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Enable modern OpenGL extension loading through GLEW
	glewExperimental = GL_TRUE;
	// Initialize GLEW to set up the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, framebufferWidth, framebufferHeight);



	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");
	
	// Models
    // Manually exported OBJs already contain world-space vertex positions.
    if (!loadWheelData("Models/Autobus/ruedas.txt")) {
        glfwTerminate();
        return EXIT_FAILURE;
    }
    Model busBodyModel((char*)"Models/Autobus/carroceria.obj");
    Model wheelModel0((char*)"Models/Autobus/llanta_1.obj");
    Model wheelModel1((char*)"Models/Autobus/llanta_2.obj");
    Model wheelModel2((char*)"Models/Autobus/llanta_3.obj");
    Model wheelModel3((char*)"Models/Autobus/llanta_4obj.obj");
    Model wheelModel4((char*)"Models/Autobus/llanta_5.obj");
    Model wheelModel5((char*)"Models/Autobus/llanta_6.obj");
    Model* wheelModels[6] = {&wheelModel0, &wheelModel1, &wheelModel2, &wheelModel3, &wheelModel4, &wheelModel5};
    Model floorModel((char*)"Models/piso.obj");



	// Set up the cube vertex array and vertex buffer
	GLuint cubeVbo, cubeVao;
	glGenVertexArrays(1, &cubeVao);
	glGenBuffers(1, &cubeVbo);
	glBindVertexArray(cubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set texture units
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)framebufferWidth / (GLfloat)framebufferHeight, 0.1f, 100.0f);

	// Main rendering loop
	while (!glfwWindowShouldClose(window))
	{

		// Calculate frame delta time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Poll events and update movement and animation
		glfwPollEvents();
		processMovement();
		updateAnimation();

		// Clear the color buffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	   
		// OpenGL options
		glEnable(GL_DEPTH_TEST);

		
		
		
	

		// Use corresponding shader when setting uniforms/drawing objects
		lightingShader.Use();

        glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);
		//glUniform1i(glGetUniformLocation(lightingShader.Program, "specular"),1);

		GLint viewPositionLocation = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPositionLocation, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);


		// Directional light
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"),0.6f,0.6f,0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"),0.3f, 0.3f, 0.3f);


		// Point light 1
	    glm::vec3 lightColor;
		lightColor.x= abs(sin(glfwGetTime() *lightPulseFrequency.x));
		lightColor.y= abs(sin(glfwGetTime() *lightPulseFrequency.y));
		lightColor.z= sin(glfwGetTime() *lightPulseFrequency.z);

		
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), lightColor.x,lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), lightColor.x,lightColor.y,lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 1.0f, 0.2f, 0.2f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"),0.075f);


		// Spotlight
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.7f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));
		

		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

		// Build view and projection matrices
		glm::mat4 view;
		view = camera.GetViewMatrix();

		// Get the uniform locations
		GLint modelLocation = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLocation = glGetUniformLocation(lightingShader.Program, "view");
		GLint projectionLocation = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));


		glm::mat4 model(1);

	
		
		//Draw scene models 
        view = camera.GetViewMatrix();	
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		floorModel.Draw(lightingShader);

        // 1) Parent transform: translate and rotate the entire bus as one object.
        glm::mat4 busMatrix(1.0f);
        busMatrix = glm::translate(busMatrix, busPosition);
        busMatrix = glm::rotate(busMatrix, busHeading, glm::vec3(0.0f, 1.0f, 0.0f));
        // The bus turning pivot is placed between its axles.
        // The meshes retain their original Blender world-space coordinates.
        const glm::vec3 busPivot(0.36695f, 0.0f, 3.16461f);
        busMatrix = glm::translate(busMatrix, -busPivot);

        // 2) Bus body: inherits the main bus transform.
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(busMatrix));
        busBodyModel.Draw(lightingShader);

        // 3) Wheels: manually exported OBJs already include their world-space positions.
        // T(wheelCenter) * R(steeringAngle) * R(rollingAngle) * T(-wheelCenter)
        // Rotate each wheel around its own center, keeping its mounted position.
        for (int i = 0; i < 6; ++i) {
            const glm::vec3 wheelCenter = wheelDataArray[i].position;
            glm::mat4 wheelMatrix = glm::translate(busMatrix, wheelCenter);
            if (wheelDataArray[i].isFrontWheel) {
                wheelMatrix = glm::rotate(wheelMatrix, steeringAngle,
                                          glm::vec3(0.0f, 1.0f, 0.0f));
            }
            const float rollingAngle = -distanceTraveled / wheelDataArray[i].radius;
            wheelMatrix = glm::rotate(wheelMatrix, rollingAngle,
                                      glm::vec3(1.0f, 0.0f, 0.0f));
            wheelMatrix = glm::translate(wheelMatrix, -wheelCenter);
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(wheelMatrix));
            wheelModels[i]->Draw(lightingShader);
        }
		glBindVertexArray(0);
	

		// Draw the light indicator with the lamp shader
		lampShader.Use();
		// Get the transform uniform locations for the lamp shader
		modelLocation = glGetUniformLocation(lampShader.Program, "model");
		viewLocation = glGetUniformLocation(lampShader.Program, "view");
		projectionLocation = glGetUniformLocation(lampShader.Program, "projection");

		// Set matrices
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		// Draw the light object (using light's vertex attributes)
		
			model = glm::mat4(1);
			model = glm::translate(model, pointLightPositions[0]);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
			glBindVertexArray(cubeVao);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		
		glBindVertexArray(0);



		// Swap the screen buffers
		glfwSwapBuffers(window);
	}


	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();



	return 0;
}

// Moves/alters the camera positions based on user input
void processMovement()
{

	// Camera controls
	if (pressedKeys[GLFW_KEY_W] || pressedKeys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);

	}

	if (pressedKeys[GLFW_KEY_S] || pressedKeys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);


	}

	if (pressedKeys[GLFW_KEY_A] || pressedKeys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);


	}

	if (pressedKeys[GLFW_KEY_D] || pressedKeys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);


	}

	if (pressedKeys[GLFW_KEY_T])
	{
		pointLightPositions[0].x += 0.01f;
	}
	if (pressedKeys[GLFW_KEY_G])
	{
		pointLightPositions[0].x -= 0.01f;
	}

	if (pressedKeys[GLFW_KEY_Y])
	{
		pointLightPositions[0].y += 0.01f;
	}

	if (pressedKeys[GLFW_KEY_H])
	{
		pointLightPositions[0].y -= 0.01f;
	}
	if (pressedKeys[GLFW_KEY_U])
	{
		pointLightPositions[0].z -= 0.1f;
	}
	if (pressedKeys[GLFW_KEY_J])
	{
		pointLightPositions[0].z += 0.01f;
	}
	
}

// Is called whenever a key is pressed/released via GLFW
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			pressedKeys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			pressedKeys[key] = false;
		}
	}

    // Handle the key only once per press, not while held or released.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        isLightEnabled = !isLightEnabled;
        lightPulseFrequency = isLightEnabled ? glm::vec3(1.0f, 1.0f, 0.0f) : glm::vec3(0.0f);
    }
    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        if (currentBusState == waitingForStart || currentBusState == completed) {
            distanceTraveled = 0.0f;
            busPosition = glm::vec3(0.0f);
            busHeading = 0.0f;
            steeringAngle = 0.0f;
            currentBusState = initialStraight;
            isAnimationRunning = true;
            std::cout << "Bus animation started" << std::endl;
        } else {
            isAnimationRunning = !isAnimationRunning; // N pauses/resumes
        }
    }
}

void updateAnimation() {
    if (!isAnimationRunning) return;

    // Travel distance depends on elapsed time rather than frame count.
    const float frameStep = std::max(0.0f, std::min(deltaTime, 0.1f));
    distanceTraveled = std::min(totalRouteDistance,
                                  distanceTraveled + busSpeed * frameStep);
    const float travel = distanceTraveled;

    if (travel <= initialStraightDistance) {
        currentBusState = initialStraight;
        busPosition = glm::vec3(0.0f, 0.0f, travel);
        busHeading = 0.0f;
        steeringAngle = 0.0f;
    } else if (travel <= initialStraightDistance + turnArcLength) {
        currentBusState = turning90;
        // Exact quarter-circle arc of radius R.
        float arcDistance = travel - initialStraightDistance;
        float turnAngle = arcDistance / turnRadius;
        busPosition = glm::vec3(turnRadius * (1.0f - cosf(turnAngle)),
                                0.0f,
                                initialStraightDistance + turnRadius * sinf(turnAngle));
        busHeading = turnAngle;  // 0 to 90 degrees around the Y axis
        steeringAngle = glm::radians(18.0f) * sinf(2.0f * turnAngle);
    } else {
        currentBusState = finalStraight;
        float finalStraightTravel = travel - initialStraightDistance - turnArcLength;
        busPosition = glm::vec3(turnRadius + finalStraightTravel,
                                0.0f,
                                initialStraightDistance + turnRadius);
        busHeading = busPi * 0.5f;
        steeringAngle = 0.0f;
    }

    if (distanceTraveled >= totalRouteDistance) {
        currentBusState = completed;
        isAnimationRunning = false;
        std::cout << "Route completed; press N to replay." << std::endl;
    }
}

bool loadWheelData(const char* filePath) {
    std::ifstream fileStream(filePath);
    if (!fileStream) {
        std::cerr << "ERROR: Could not find " << filePath
                  << ". Place ruedas.txt next to the OBJ files.\n";
        return false;
    }
    std::string line;
    int i = 0;
    while (std::getline(fileStream, line) && i < 6) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream lineStream(line);
        float x, y, z, radius;
        int frontWheelFlag;
        if (!(lineStream >> x >> y >> z >> radius >> frontWheelFlag) || radius <= 0.0f) {
            std::cerr << "ERROR: Invalid wheel data in " << filePath << std::endl;
            return false;
        }
        wheelDataArray[i++] = {glm::vec3(x, y, z), radius, frontWheelFlag != 0};
    }
    if (i != 6) {
        std::cerr << "ERROR: Expected 6 wheels in " << filePath << std::endl;
        return false;
    }
    return true;
}

void mouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Invert Y movement to match viewport coordinates

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}