#pragma once

#include "Shader.h"
#include <map>

namespace Framework
{
	class ShaderManager : public Singleton<ShaderManager>
	{
		friend class Singleton<ShaderManager>;

	public:
		inline void						setAllocator(Allocators *allocators) { mAllocators = allocators; }

		Shader::Handle					createShader(Shader **out_shader, const Shader::Description *desc);
		Shader::Handle					createShader(Shader **out_shader, ShaderType type, const U8 *pData, const char *name);

		Shader::Handle					createShaderFromFile(Shader **out_shader, const char *filePath, Shader::Description *desc);
		void							saveShaderToFile(const char *filePath, Shader::Handle handle);

		void							destoryShader(Shader::Handle handle);

		Shader *						getShader(Shader::Handle handle) const;

	private:
		ShaderManager();
		virtual ~ShaderManager();

		Shader::Handle					genShaderHandle();

	private:
		std::map<Shader::Handle, Shader *> mShaderTable;
		Allocators *					mAllocators{ nullptr };
	};
}