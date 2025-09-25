#pragma once
#include "GLwinShader.h"


class GLwinShaderManager
{
public:
	static void SetUpShaders();

	static Shader* defaultShader;
};