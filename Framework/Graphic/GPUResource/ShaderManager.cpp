#include "stdafx.h"

#include "ShaderManager.h"
#include "IncludedShaders.h"

using namespace sce;

Framework::Shader::Handle Framework::ShaderManager::createShader(Shader **out_shader, const Shader::Description *desc)
{
	Shader::Handle _handle = Shader::SHADER_HANDLE_INVALID;
	if (out_shader != nullptr && desc != nullptr)
	{
		_handle = genShaderHandle();
		SCE_GNM_ASSERT(mAllocators != nullptr);
		SCE_GNM_ASSERT(getShader(_handle) == nullptr);

		Shader *_shader = new Shader;
		_shader->init(*desc, mAllocators);
		_shader->setHandle(_handle);
		mShaderTable[_handle] = _shader;
		*out_shader = _shader;
	}
	return _handle;
}

Framework::Shader::Handle Framework::ShaderManager::createShader(Shader **out_shader, ShaderType type, const U8 *pData, const char *name)
{
	Shader::Description _desc;
	_desc.mType				= type;
	_desc.mDataPtr			= pData;
	_desc.mName				= name;
	return createShader(out_shader, &_desc);
}

Framework::Shader::Handle Framework::ShaderManager::createShaderFromFile(Shader **out_shader, const char *filePath, Shader::Description *desc)
{
	FileIO _file(filePath);
	_file.load();

	desc->mDataPtr	= _file.getBuffer();
	desc->mName		= _file.getName();

	return createShader(out_shader, desc);
}

void Framework::ShaderManager::saveShaderToFile(const char *filePath, Shader::Handle handle)
{
	SCE_GNM_ASSERT(filePath != nullptr);
	Shader *_shader = getShader(handle);
	SCE_GNM_ASSERT(_shader != nullptr);
	// TODO
}

void Framework::ShaderManager::destoryShader(Shader::Handle handle)
{
	if (handle != Shader::SHADER_HANDLE_INVALID)
	{
		auto itor = mShaderTable.find(handle);
		if (itor != mShaderTable.end())
		{
			itor->second->deinit(mAllocators);
			SAFE_DELETE(itor->second);
			mShaderTable.erase(itor);
		}
	}
}

Framework::Shader * Framework::ShaderManager::getShader(Shader::Handle handle) const
{
	auto itor = mShaderTable.find(handle);
	return (itor != mShaderTable.end()) ? itor->second : nullptr;
}

Framework::ShaderManager::ShaderManager()
{

}

Framework::ShaderManager::~ShaderManager()
{
	for (auto itor = mShaderTable.begin(); itor != mShaderTable.end(); itor++)
	{
		if (itor->second)
		{
			itor->second->deinit(mAllocators);
			SAFE_DELETE(itor->second);
		}
	}
	mShaderTable.clear();
}

Framework::Shader::Handle Framework::ShaderManager::genShaderHandle()
{
	static Shader::Handle _handle = CUSTOM_SHADER_HANDLE_START - 1;
	_handle++;
	return _handle;
}

