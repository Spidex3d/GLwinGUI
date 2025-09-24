#include <../../vendors/glad/glad.h>// Include glad to get the OpenGL headers
#include "../GLwinGUI.h"
#include "../../GLwin/include/GLwinLog.h"
#include <iostream>
#include "../GLwinMaths.h"

// Internal struct definition for GUI window
struct WindowData {

    int index;              // Window index
    std::string objectName; // Window name
	int WidgetIndex;        // Widget index ie: Button, Slider, Checkbox
	int WidgetTypeID;       // Widget type ID

	int width;              // window width
	int height; 		    // window height

	int posX;              // window position X
	int posY;              // window position Y

	bool isWinSelected;    // this for the selection in the Window for dragging
	
};

GLwinGUI::GLwinGUI()
{
}

GLwinGUI::~GLwinGUI()
{
}

GLwinGUI* GLwinGUI::Instance()
{
	static GLwinGUI* guiScreen = new GLwinGUI;
	return guiScreen;
}

void GLwinGUI::Initialize()
{
	// Setup a simple rectangle for the GUI window using glad
	if (!gladLoadGL()) {

		GLWIN_LOG_WARNING("Failed to initialize GLAD!");
		return;
	}
	else {
		GLWIN_LOG_INFO("GLAD initialized successfully.");
	}
	float vertices[] = {
		//Positions          Normals          Text coords
		 0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f

	};
	unsigned int indices[] = {
		  0, 1, 3,
		  1, 2, 3
	};

	vec3 position = vec3(0.0f, 0.0f, 0.0f);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// Vertex positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	//glEnableVertexAttribArray(2);
	//// Texture coordinates
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);	

}

GLwinGui* GLwinGUI::CreateGuiWindow(const char* name, int winpos_x, int winpos_y, int width, int height)
{
	
	return nullptr;
}

void GLwinGUI::GLwinDrawGuiWindow()
{
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // using indices
	glBindVertexArray(0);
	glUseProgram(0);
}

void GLwinGUI::GLwin_DestroyWindow(GLwinGui* window)
{
}

void GLwinGUI::GLwinHelloFromGLwinGUI()
{
	std::cout << "Hello, GLwinGUI.h Window!" << std::endl;
	GLWIN_LOG_INFO("Hello from GLwinGUI was called!");
}
