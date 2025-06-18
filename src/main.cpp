// STL includes
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <array>

// External libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Our files
#include "ThreeDModel.h"
#include "Raytracer.h"

// Global variables
GLFWwindow* window;
Raytracer* raytracer;
int windowWidth = 1920;
int windowHeight = 1080;
GLuint raytracerTextureID;
RenderParameters renderParameters;
bool launchRaytracer;
std::byte movementKeys;
double mouseXpos, mouseYpos;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Raytracing keys
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		launchRaytracer = true;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		renderParameters.interpolationRendering = !renderParameters.interpolationRendering;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		renderParameters.phongEnabled = !renderParameters.phongEnabled;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		renderParameters.shadowsEnabled = !renderParameters.shadowsEnabled;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		renderParameters.reflectionEnabled = !renderParameters.reflectionEnabled;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
		renderParameters.refractionEnabled = !renderParameters.refractionEnabled;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
		renderParameters.fresnelRendering = !renderParameters.fresnelRendering;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_7 && action == GLFW_PRESS) {
		renderParameters.monteCarloEnabled = !renderParameters.monteCarloEnabled;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		renderParameters.orthoProjection = !renderParameters.orthoProjection;
		
	}

	if (key == GLFW_KEY_W){
		if(action == GLFW_PRESS)
			movementKeys |= std::byte{ BIT_FW };
		if(action == GLFW_RELEASE)
			movementKeys &= ~std::byte{ BIT_FW };
	}
	if (key == GLFW_KEY_S) {
		if (action == GLFW_PRESS)
			movementKeys |= std::byte{ BIT_BACK };
		if (action == GLFW_RELEASE)
			movementKeys &= ~std::byte{ BIT_BACK };
	}
	if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS)
			movementKeys |= std::byte{ BIT_RIGHT };
		if (action == GLFW_RELEASE)
			movementKeys &= ~std::byte{ BIT_RIGHT };
	}
	if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS)
			movementKeys |= std::byte{BIT_LEFT };
		if (action == GLFW_RELEASE)
			movementKeys &= ~std::byte{ BIT_LEFT };
	}
	if (key == GLFW_KEY_Q) {
		if (action == GLFW_PRESS)
			movementKeys |= std::byte{ BIT_UP };
		if (action == GLFW_RELEASE)
			movementKeys &= ~std::byte{ BIT_UP};
	}
	if (key == GLFW_KEY_E) {
		if (action == GLFW_PRESS)
			movementKeys |= std::byte{BIT_DOWN };
		if (action == GLFW_RELEASE)
			movementKeys &= ~std::byte{ BIT_DOWN };
	}
}

void mousePosCallback(GLFWwindow* window, double x, double y) {
	mouseXpos = x;
	mouseYpos = y;

	float scaledX = float(2.0f * mouseXpos - windowWidth) / float(windowWidth);
	float scaledY = float(windowHeight - 2.0f * mouseYpos) / float(windowHeight);
	if ((movementKeys & std::byte{ BIT_LEFTMOUSE }) != std::byte{ 0 })
		renderParameters.CameraArcball.ContinueDrag(scaledX, scaledY);
	if ((movementKeys & std::byte{ BIT_RIGHTMOUSE }) != std::byte{ 0 })
		renderParameters.ModelArcball.ContinueDrag(scaledX, scaledY);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	float scaledX = float(2.0f * mouseXpos - windowWidth) / float(windowWidth);
	float scaledY = float(windowHeight - 2.0f * mouseYpos) / float(windowHeight);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		renderParameters.CameraArcball.BeginDrag(scaledX,scaledY);
		movementKeys |= std::byte{ BIT_LEFTMOUSE };
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		renderParameters.ModelArcball.BeginDrag(scaledX, scaledY);
		movementKeys |= std::byte{ BIT_RIGHTMOUSE };
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		renderParameters.CameraArcball.EndDrag(scaledX, scaledY);
		movementKeys &= ~std::byte{ BIT_LEFTMOUSE };
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		renderParameters.ModelArcball.EndDrag(scaledX, scaledY);
		movementKeys &= ~std::byte{ BIT_RIGHTMOUSE };
	}
}

void windowResize(GLFWwindow* window, int width, int height) {
	windowWidth = width;
	windowHeight = height;
	raytracer->stopRaytracer();
	raytracer->resize(int(width / 2.0f), height);
}

bool initializeGL() {
	// Initialise GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return false;
	}

	// GLFW window hints
	glfwWindowHint(GLFW_SAMPLES, 1); //no anti-aliasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy;
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(windowWidth, windowHeight, "OpenGLRenderer", NULL, NULL);

	if (window == NULL) {
		std::cerr << "Failed to open GLFW window. If you have an Intel GPU, they may not be 4.5 compatible." << std::endl;
		glfwTerminate();
		return false;
	}
	
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return false;
	} 

	if (!GLEW_ARB_debug_output) return false;

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwPollEvents();

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetWindowSizeCallback(window, windowResize);
	
	return true;
}

void ThreeDToGL(
	const ThreeDModel& model, 
	std::vector<std::array<float, 3>>& outVertices,
	std::vector<std::array<float, 2>>& outUVs,
	std::vector<std::array<float, 3>>& outNormals) {

	for (unsigned int face = 0; face < model.faceVertices.size(); face++) {
		for (unsigned int triangle = 0; triangle < model.faceVertices[face].size() - 2; triangle++) {
			for (unsigned int vertex = 0; vertex < 3; vertex++) {
				unsigned int faceVertex = 0;
				if (vertex != 0)
					faceVertex = triangle + vertex;
			
				outNormals.push_back(std::array<float, 3>{
					model.normals[model.faceNormals[face][faceVertex]].x,
					model.normals[model.faceNormals[face][faceVertex]].y,
					model.normals[model.faceNormals[face][faceVertex]].z
				});
				outUVs.push_back(std::array<float, 2>{
					model.textureCoords[model.faceTexCoords[face][faceVertex]].x,
					model.textureCoords[model.faceTexCoords[face][faceVertex]].y
				});
				outVertices.push_back(std::array<float, 3>{
					model.vertices[model.faceVertices[face][faceVertex]].x,
					model.vertices[model.faceVertices[face][faceVertex]].y,
					model.vertices[model.faceVertices[face][faceVertex]].z
				});
			} // per vertex
		} // per triangle
	} // per face
}

void loadModelGL(
	const std::vector<ThreeDModel>& objects,
	std::vector<GLuint>& vaoIDs,
	std::vector<GLuint>& vbIDs,
	std::vector<GLuint>& nbIDs,
	std::vector<GLuint>& tbIDs,
	std::vector<unsigned int>& count) {
	
	for (const auto& to : objects) {
		GLuint vertexArrayID;
		GLuint vertexBuffer;
		GLuint uvBuffer;
		GLuint normalBuffer;

		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);
	
		std::vector<std::array<float, 3>> n;
		std::vector<std::array<float, 2>> t;
		std::vector<std::array<float, 3>> v;
		ThreeDToGL(to, v, t, n);

		glEnableVertexAttribArray(0);
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(std::array<float, 3>), &v[0], GL_STATIC_DRAW);
		glVertexAttribPointer(
			0,			// attribute
			3,			// size (we have x y z)
			GL_FLOAT,	// type of each individual element
			GL_FALSE,	// normalised
			0,			// stride
			(void*)0	// array buffer offset
		);

		glEnableVertexAttribArray(1);
		glGenBuffers(1, &uvBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
		glBufferData(GL_ARRAY_BUFFER, t.size() * sizeof(std::array<float, 2>), &t[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(2);
		glGenBuffers(1, &normalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, n.size() * sizeof(std::array<float, 3>), &n[0], GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		vaoIDs.push_back(vertexArrayID);
		vbIDs.push_back(vertexBuffer);
		tbIDs.push_back(uvBuffer);
		nbIDs.push_back(normalBuffer);
		count.push_back(GLuint(v.size()));
	}
}

bool readAndCompileShader(const char* shaderPath, const GLuint& id) {
	std::string shaderCode;
	std::ifstream shaderStream(shaderPath, std::ios::in);
	if (shaderStream.is_open()) {
		std::stringstream sstr;
		sstr << shaderStream.rdbuf();
		shaderCode = sstr.str();
		shaderStream.close();
	} else {
		std::cout << "Impossible to open " << shaderPath << ". Are you in the right directory?" << std::endl;
		return false;
	}

	std::cout << "Compiling shader: " << shaderPath << std::endl;

	const char* sourcePointer = shaderCode.c_str();
	glShaderSource(id, 1, &sourcePointer, NULL);
	glCompileShader(id);

	GLint result = GL_FALSE;

	int infoLogLength;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> shaderErrorMessage(infoLogLength + 1);
		glGetShaderInfoLog(id, infoLogLength, NULL, &shaderErrorMessage[0]);
		std::cout << &shaderErrorMessage[0] << std::endl;
	}

	std::cout << "Compilation of shader: " << shaderPath << " " << (result == GL_TRUE ? "Success" : "Failed!") << std::endl;
	return result == 1;
}

void LoadShaders(GLuint& program, const char* vertexFilePath, const char* fragmentFilePath) {
	// Create the shaders
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	bool vok = readAndCompileShader(vertexFilePath, vertexShaderID);
	bool fok = readAndCompileShader(fragmentFilePath, fragmentShaderID);

	if (vok && fok) {
		GLint result = GL_FALSE;
		int infoLogLength;

		std::cout << "Linking program" << std::endl;
		program = glCreateProgram();
		glAttachShader(program, vertexShaderID);
		glAttachShader(program, fragmentShaderID);
		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &result);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0) {
			std::vector<char> programErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(program, infoLogLength, NULL, &programErrorMessage[0]);
			std::cout << &programErrorMessage[0] << std::endl;
		}

		std::cout << "Linking program: " << (result == GL_TRUE ? "Success" : "Failed!") << std::endl;
	} else {
		std::cout << "Program will not be linked: one of the shaders has an error" << std::endl;
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
}

void loadScreenspaceTexture() {
	glGenTextures(1, &raytracerTextureID);
	glBindTexture(GL_TEXTURE_2D, raytracerTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, GLsizei(windowWidth / 2.0f), windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, raytracer->frameBuffer.block);
	glBindTexture(GL_TEXTURE_2D, -1);
}

int main(int argc, char**argv) {
	if (argc != 3) { // bad arg count
		// print an error message
		std::cout << "Usage: " << argv[0] << " geometry material" << std::endl;
		// and leave
		return 0;
	}

	// Try intialise GLFW and GLEW
	if (!initializeGL()) return -1;

	std::vector<ThreeDModel> objects;
	std::ifstream geometryFile(argv[1]);
	std::ifstream materialFile(argv[2]);

	// try reading the files
	if (!(geometryFile.good()) || !(materialFile.good())) {
		std::cout << "Read failed for object " << argv[1] << " or material " << argv[2] << std::endl;
		return 0;
	}

	std::string s = argv[2];
	// if is actually passing a material. This will trigger the modified obj read code.
	if (s.find(".mtl") != std::string::npos) {
		objects = ThreeDModel::ReadObjectStreamMaterial(geometryFile, materialFile);
	}

	if (objects.size() == 0) {
		std::cout << "Read failed for object " << argv[1] << " or material " << argv[2] << std::endl;
		return 0;
	}

	renderParameters.findLights(objects);
	std::cout << renderParameters.lights.size() << std::endl;

	std::vector<GLuint> vaoIDs;
	std::vector<GLuint> vbIDs;
	std::vector<GLuint> nbIDs;
	std::vector<GLuint> tbIDs;
	std::vector<unsigned int> counts;

	// setting up opengl
	loadModelGL(objects, vaoIDs, vbIDs, nbIDs, tbIDs, counts);

	raytracer = new Raytracer(&objects, &renderParameters);
	raytracer->resize(int(windowWidth / 2.0f), windowHeight);

	GLuint raytracerVAO;
	glGenVertexArrays(1, &raytracerVAO);
	glBindVertexArray(raytracerVAO);

	GLuint programID, ssProgramID;
	LoadShaders(programID, "Basic.vert", "Phong.frag");
	LoadShaders(ssProgramID, "Screenspace.vert", "Screenspace.frag");

	loadScreenspaceTexture();

	glClearColor(0, 0, 0, 0.0f);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	do {
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);
		nbFrames++;
		if (deltaTime >= 1.0f) {
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Matrix4 modelMatrix = renderParameters.getModelMatrix();
		Matrix4 viewMatrix = renderParameters.getViewMatrix();
		Matrix4 projectionMatrix = renderParameters.getProjectionMatrix(float(windowWidth) / 2.0f, float(windowHeight));
		Matrix4 MVP = projectionMatrix * (viewMatrix * modelMatrix);

		// Get a handle for our uniforms
		glUseProgram(programID);
		GLuint matrixID = glGetUniformLocation(programID, "MVP");
		GLuint viewMatrixID = glGetUniformLocation(programID, "V");
		GLuint modelMatrixID = glGetUniformLocation(programID, "M");
		std::vector<GLuint> posLightIDs;
		std::vector<GLuint> colLightIDs;

		// Set the light position and colour
		int currentLight = 0;
		for (Light* l : renderParameters.lights) {
			posLightIDs.push_back(glGetUniformLocation(programID, ("lights[" + std::to_string(currentLight) + "].position").c_str()));
			colLightIDs.push_back(glGetUniformLocation(programID, ("lights[" + std::to_string(currentLight) + "].colour").c_str()));
			currentLight++;
		}

		GLuint nLightsID = glGetUniformLocation(programID, "nLights");
		GLuint emissiveID = glGetUniformLocation(programID, "meshMaterial.emissiveColour");
		GLuint diffuseID = glGetUniformLocation(programID, "meshMaterial.diffuseColour");
		GLuint ambientID = glGetUniformLocation(programID, "meshMaterial.ambientColour");
		GLuint specularID = glGetUniformLocation(programID, "meshMaterial.specularColour");
		GLuint shininessID = glGetUniformLocation(programID, "meshMaterial.shininess");

		renderParameters.computeMatricesFromInputs(deltaTime, movementKeys);

		// First pass: Rasterisation

		// Send our transformation to the currently bound shader
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, MVP.columnMajor().coordinates);
		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, modelMatrix.columnMajor().coordinates);
		glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, viewMatrix.columnMajor().coordinates);

		// Set the light position and colour
		int cl = 0;
		for (Light* l : renderParameters.lights) {
			Homogeneous4 lp = l->GetPositionCenter();
			Homogeneous4 c = l->GetColor();
			glUniform3f(posLightIDs[cl], lp.x, lp.y, lp.z);
			glUniform3f(colLightIDs[cl], c.x, c.y, c.z);
			cl++;
		}

		glUniform1i(nLightsID, currentLight);

		// This will draw on the left side
		glViewport(0, 0, GLsizei(windowWidth / 2.0f), windowHeight);
		for (int i = 0; i < vaoIDs.size(); i++) {
			// Setting material properties
			Cartesian3 d = objects[i].material->diffuse;
			Cartesian3 a = objects[i].material->ambient;
			Cartesian3 s = objects[i].material->specular;
			Cartesian3 e = objects[i].material->emissive;
			float shin = objects[i].material->shininess;
			glUniform3f(diffuseID, d.x,d.y,d.z);
			glUniform3f(ambientID, a.x,a.y,a.z);
			glUniform3f(specularID, s.x, s.y, s.z);
			glUniform3f(emissiveID, e.x, e.y, e.z);
			glUniform1f(shininessID, shin);
			glBindVertexArray(vaoIDs[i]);

			//Draw the triangles
			glDrawArrays(GL_TRIANGLES, 0, counts[i]);
		}

		glUseProgram(ssProgramID);
		glViewport(GLint(windowWidth / 2.0f), 0, GLsizei(windowWidth / 2.0f), windowHeight);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, raytracerTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, GLsizei(windowWidth / 2.0f), windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, raytracer->frameBuffer.block);
		glBindVertexArray(raytracerVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		if (launchRaytracer) {
			raytracer->Raytrace();
			launchRaytracer = false;
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();


	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	glDeleteShader(programID);
	glDeleteShader(ssProgramID);
	glDeleteTextures(1, &raytracerTextureID);
	for (auto vaoID : vaoIDs) glDeleteVertexArrays(1, &vaoID);
	for (auto vbID : vbIDs) glDeleteVertexArrays(1, &vbID);
	for (auto nbID : nbIDs) glDeleteVertexArrays(1, &nbID);
	for (auto tbID : tbIDs) glDeleteVertexArrays(1, &tbID);
	raytracer->stopRaytracer();
	glfwTerminate();

	return 0;
}

