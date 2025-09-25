#include "GLwinShaderManager.h"

Shader* GLwinShaderManager::defaultShader = nullptr;

void GLwinShaderManager::SetUpShaders()
{
	defaultShader = new Shader("Shader/Shaders/test.vert", "Shader/Shaders/test.frag");

	/*defaultShader = new Shader("C:\Users\marty\Desktop\GLwinGUI\GLwinTest\GLwinGUI\Shader\Shaders\test.vert",
		"C:\Users\marty\Desktop\GLwinGUI\GLwinTest\GLwinGUI\Shader\Shaders\test.frag");*/
}
