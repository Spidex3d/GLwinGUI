#pragma once
#include <../vendors/glad/glad.h>
#include "../vendors/glm/glm.hpp"
#include <vector>
#include <memory>
#include <string>
//#include "GLwinMaths.h"

struct GuiWindowData {
    int winindex = 0;              // Window index
    int currentIndex;              // Current selected window index
    std::string GuiWinName;        // Window name
    int width = 0;                 // window width
    int height = 0;                // window height
    int posX = 0;                  // window position X
    int posY = 0;                  // window position Y
    bool isWinSelected = false;    // selection state

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Constructor for convenient initialization
    GuiWindowData(int curIdx, const std::string& name, int winIdx)
        : winindex(winIdx), currentIndex(curIdx), GuiWinName(name) {
    }
};

class GLwinGUI {
public:
    GLwinGUI();
    ~GLwinGUI();

    static GLwinGUI* Instance();
    void Initialize();

    // Function to create a new GUI window
    void CreateGuiWindow(const glm::mat4& view, const glm::mat4& projection,
        std::vector<std::unique_ptr<GuiWindowData>>& guiwWindowsdata, int& currentIndex, int& winindex);

    // Draw all GUI windows
    void GLwinDrawGuiWindow(const std::vector<std::unique_ptr<GuiWindowData>>& guiwWindowsdata);

    void GLwin_DestroyWindow(GuiWindowData* guiwindow);
    void GLwinHelloFromGLwinGUI();

    // Set this to true to trigger window creation
    void RequestAddNewWindow() { ShouldAddNewWindow = true; }

private:
    GLuint VBO, VAO, EBO;
    bool ShouldAddNewWindow = false;
	
};







//#pragma once
//#include <vector>
//#include <memory>
//#include <string>
//
//struct GuiWindowData {
//	int winindex = 0;              // Window index
//	int currentIndex;       // Current selected window index
//	std::string GuiWinName; // Window name
//	int width;              // window width
//	int height; 		    // window height
//	int posX;               // window position X
//	int posY;               // window position Y
//	bool isWinSelected;     // this for the selection in the Window for dragging
//	// Add more GUI elements as needed (buttons, sliders, etc.)
//
//	GuiWindowData(int curIdx, const std::string& name, int winIdx)
//		: winindex(winIdx), currentIndex(curIdx), GuiWinName(name),
//		width(0), height(0), posX(0), posY(0), isWinSelected(false) {
//	}
//};
//
//class GLwinGUI {
//
//public:
//	typedef struct GLwinGui GLwinGui;
//
//		
//	GLwinGUI();
//	~GLwinGUI();
//
//	static GLwinGUI* Instance();
//	void Initialize();
//
//	// Function to create a new GUI window
//	void CreateGuiWindow(const mat4& view, const mat4& projection,
//		std::vector<std::unique_ptr<GuiWindowData>>& guiwWindowsdata, int& currentIndex, int& winindex);
//
//
//	void GLwinDrawGuiWindow();  // Draw all GUI windows
//	
//	void GLwin_DestroyWindow(GLwinGui* guiwindow);
//
//	void GLwinHelloFromGLwinGUI();
//
//private:
//	GLwinGUI* guiwindow;
//	GLuint VBO, VAO, EBO;
//
//	bool ShouldAddNewWindow = false;
//
//
//	
//
//};






//#ifdef __cplusplus
//extern "C" {
//#endif
//       
//
//
//	typedef struct GLwinGui GLwinGui;
//
//    bool GLwinGui(const char* name, int winpos_x, int winpoy_y, int width, int height);
//   
//
//#ifdef __cplusplus
//}
//#endif


