#pragma once
#include <../vendors/glad/glad.h>
#include "../vendors/glm/glm.hpp"
#include "../gui/BaseGui.h"
#include "../Shader/GLwinShader.h"
#include "../Shader/GLwinShaderManager.h"
#include <vector>
#include <memory>
#include <string>

class GLwinGUI : public BaseGui{
public:

    GLwinGUI();
    ~GLwinGUI();
   
    static GLwinGUI* Instance();
    void Initialize();

	// Function to render all GUI windows and widgets
    void RenderGUI(const glm::mat4& view, const glm::mat4& projection,
        std::vector<std::unique_ptr<BaseGui>>& guiwWindowsdata, int& currentIndex, Shader& shader);

    // Function to create a new GUI window
    void CreateGuiWindow(const glm::mat4& view, const glm::mat4& projection,
        std::vector<std::unique_ptr<BaseGui>>& guiwWindowsdata, int& currentIndex, int& winindex);
        
    void GLwinHelloFromGLwinGUI();

    // Set this to true to trigger window creation
    void RequestAddNewWindow() { ShouldAddNewWindow = true; }

private:
  
    bool ShouldAddNewWindow = false;
	
};

