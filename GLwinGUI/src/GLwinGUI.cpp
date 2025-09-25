//#include "../../vendors/glad/glad.h" // Include glad to get the OpenGL headers
#include "../GLwinGUI.h"
#include "../../GLwin/include/GLwinLog.h"
#include "../../GLwinGUI/Shader/GLwinShaderManager.h"
#include "../../vendors/glm/glm.hpp"
#include "../../vendors/glm/gtc/matrix_transform.hpp" // Include for glm::mat4 transformations

#include <iostream>

GLwinGUI::GLwinGUI() {}
GLwinGUI::~GLwinGUI() {}

GLwinGUI* GLwinGUI::Instance() {
    static GLwinGUI* guiScreen = new GLwinGUI;
    return guiScreen;
}

void GLwinGUI::Initialize() {
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
}

void GLwinGUI::CreateGuiWindow(const glm::mat4& view, const glm::mat4& projection,
    std::vector<std::unique_ptr<GuiWindowData>>& guiwWindowsdata, int& currentIndex, int& winindex)
{
    if (ShouldAddNewWindow) {
        winindex = static_cast<int>(guiwWindowsdata.size());

        std::unique_ptr<GuiWindowData> newWindow = std::make_unique<GuiWindowData>(currentIndex, "Default_Window", winindex);
        switch (winindex) {
        case 0:
            newWindow->posX = 200;
            newWindow->posY = 200;
            newWindow->GuiWinName = "Debug";
            newWindow->width = 300;
            newWindow->height = 200;
            break;
        case 1:
            newWindow->posX = 200;
            newWindow->posY = 200;
            newWindow->GuiWinName = "Debug_02";
            newWindow->width = 300;
            newWindow->height = 200;
            break;
        default:
            newWindow->posX = 100;
            newWindow->posY = 100;
            newWindow->GuiWinName = "Window_" + std::to_string(winindex);
            newWindow->width = 250;
            newWindow->height = 180;
            break;
        }

        newWindow->modelMatrix = glm::mat4(1.0f);
        glm::translate(newWindow->modelMatrix, glm::vec3(newWindow->posX, newWindow->posY, 0.0f));
        glm::scale(newWindow->modelMatrix, glm::vec3(newWindow->width, newWindow->height, 1.0f));



        guiwWindowsdata.push_back(std::move(newWindow));
        ShouldAddNewWindow = false;
    }

}
    void GLwinGUI::GLwinDrawGuiWindow(const std::vector<std::unique_ptr<GuiWindowData>>&guiwWindowsdata)
    {
        // For each window, set up transforms and draw
        for (const auto& win : guiwWindowsdata) {
            // TODO: Set up model matrix using win->posX, win->posY, win->width, win->height
            // TODO: Bind shader, set uniforms, etc.
            /*if (GLwinShaderManager::defaultShader)
                GLwinShaderManager::defaultShader->Use();
            else
                std::cerr << "Error: defaultShader is null!" << std::endl;*/

            /*GLwinShaderManager::defaultShader->Use();
            GLwinShaderManager::defaultShader->setMat4("view", view);
            GLwinShaderManager::defaultShader->setMat4("projection", projection);*/

            if (auto* guiwin = dynamic_cast<GuiWindowData*>(win.get())) {
                guiwin->modelMatrix = glm::mat4(1.0f);
                // GLwinShaderManager::defaultShader->setMat4("model", guiwin->modelMatrix);

                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // using indices
                glBindVertexArray(0);
            }
        }
        glUseProgram(0);
    }

void GLwinGUI::GLwin_DestroyWindow(GuiWindowData* guiwindow)
{
    // Implement window destruction logic if needed
}

void GLwinGUI::GLwinHelloFromGLwinGUI()
{
    std::cout << "Hello, GLwinGUI.h Window!" << std::endl;
    GLWIN_LOG_INFO("Hello from GLwinGUI was called!");
}








//#include <../../vendors/glad/glad.h>// Include glad to get the OpenGL headers
//#include "../GLwinGUI.h"
//#include "../../GLwin/include/GLwinLog.h"
//#include <iostream>
//#include "../GLwinMaths.h"
//
//
//GLwinGUI::GLwinGUI()
//{
//}
//
//GLwinGUI::~GLwinGUI()
//{
//}
//
//GLwinGUI* GLwinGUI::Instance()
//{
//	static GLwinGUI* guiScreen = new GLwinGUI;
//	return guiScreen;
//}
//
//void GLwinGUI::Initialize()
//{
//	// Setup a simple rectangle for the GUI window using glad
//	if (!gladLoadGL()) {
//
//		GLWIN_LOG_WARNING("Failed to initialize GLAD!");
//		return;
//	}
//	else {
//		GLWIN_LOG_INFO("GLAD initialized successfully.");
//	}
//	float vertices[] = {
//		//Positions          Normals          Text coords
//		 0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
//		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
//		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
//		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
//
//	};
//	unsigned int indices[] = {
//		  0, 1, 3,
//		  1, 2, 3
//	};
//
//	vec3 position = vec3(0.0f, 0.0f, 0.0f);
//
//	glGenVertexArrays(1, &VAO);
//	glBindVertexArray(VAO);
//
//	glGenBuffers(1, &VBO);
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//	glGenBuffers(1, &EBO);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
//	// Vertex positions
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//	// Normal attribute
//	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
//	//glEnableVertexAttribArray(2);
//	//// Texture coordinates
//	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
//	//glEnableVertexAttribArray(1);	
//
//}
//
////void GLwinGUI::CreateGuiWindow(const mat4& view, const mat4& projection,
////	std::vector<std::unique_ptr<GuiWindowData>>& guiwWindowsdata, int& currentIndex, int& winindex)
////{
////	if (ShouldAddNewWindow) {
////		winindex = guiwWindowsdata.size();
////
////		std::unique_ptr<GuiWindowData> newWindow = std::make_unique<GuiWindowData>(currentIndex, "Default_Window", winindex);
////		switch (winindex) {
////		case 0:
////			newWindow->posX = 200; // 200 pixels from left
////			newWindow->posY = 200; // 200 pixels from top
////			newWindow->GuiWinName = "Debug";
////			newWindow->width = 300;
////			newWindow->height = 200;
////			break;
////		case 1:
////			newWindow->posX = 200; // 200 pixels from left
////			newWindow->posY = 200; // 200 pixels from top
////			newWindow->GuiWinName = "Debug_02";
////			newWindow->width = 300;
////			newWindow->height = 200;
////			break;
////		}
////		guiwWindowsdata.push_back(std::move(newWindow));
////
////	}
////		ShouldAddNewWindow = false;
////}
//
//void GLwinGUI::CreateGuiWindow(const mat4& view, const mat4& projection,
//	std::vector<std::unique_ptr<GuiWindowData>>& guiwWindowsdata, int& currentIndex, int& winindex)
//{
//	if (ShouldAddNewWindow) {
//		winindex = guiwWindowsdata.size();
//
//		std::unique_ptr<GuiWindowData> newWindow = std::make_unique<GuiWindowData>(currentIndex, "Default_Window", winindex);
//		switch (winindex) {
//		case 0:
//			newWindow->posX = 200;
//			newWindow->posY = 200;
//			newWindow->GuiWinName = "Debug";
//			newWindow->width = 300;
//			newWindow->height = 200;
//			break;
//		case 1:
//			newWindow->posX = 200;
//			newWindow->posY = 200;
//			newWindow->GuiWinName = "Debug_02";
//			newWindow->width = 300;
//			newWindow->height = 200;
//			break;
//		}
//		guiwWindowsdata.push_back(std::move(newWindow));
//		ShouldAddNewWindow = false;
//	}
//}
//
//	void GLwinGUI::GLwinDrawGuiWindow()
//{
//	glBindVertexArray(VAO);
//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // using indices
//	glBindVertexArray(0);
//	glUseProgram(0);
//}
//
//void GLwinGUI::GLwin_DestroyWindow(GLwinGui* window)
//{
//}
//
//void GLwinGUI::GLwinHelloFromGLwinGUI()
//{
//	std::cout << "Hello, GLwinGUI.h Window!" << std::endl;
//	GLWIN_LOG_INFO("Hello from GLwinGUI was called!");
//}
