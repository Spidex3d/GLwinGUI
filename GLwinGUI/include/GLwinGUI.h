#pragma once

class GLwinGUI {

public:
	typedef struct GLwinGui GLwinGui;
	
	GLwinGUI();
	~GLwinGUI();

	static GLwinGUI* Instance();
	void Initialize();

	GLwinGui* CreateGuiWindow(const char* name, int winpos_x, int winpos_y, int width, int height);
	void GLwinDrawGuiWindow();  // Draw all GUI windows
	
	void GLwin_DestroyWindow(GLwinGui* guiwindow);

	void GLwinHelloFromGLwinGUI();


private:
	GLwinGUI* guiwindow;
	GLuint VBO, VAO, EBO;

};





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


