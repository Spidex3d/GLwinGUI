#pragma once
#include <string>


class  BaseGui { // all the things that a GUI window will need

public:
    int winindex;              // Window index
    std::string GuiWinName;        // Window name
	int winTypeID = 0;             // Window type ID
    int currentIndex;              // Current selected window index
    int width = 0;                 // window width
    int height = 0;                // window height
    int posX = 0;                  // window position X
    int posY = 0;                  // window position Y
    bool isWinSelected = false;    // selection state

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Constructor for convenient initialization
    /*GuiWindowData(int curIdx, const std::string& name, int winIdx)
        : winindex(winIdx), currentIndex(curIdx), GuiWinName(name) {
    }*/

    virtual ~BaseGui() = default;
};
