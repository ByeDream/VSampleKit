#include "stdafx.h"

#include "Shader.h"

#include "Memory/Allocators.h"
#include "GPUResourceViews.h"
#include "ChunkBasedRenderContext/RenderContext.h"

Framework::Shader::Shader()
{

}

Framework::Shader::~Shader()
{
	SCE_GNM_ASSERT_MSG(mHeaderAddr == nullptr && mGpuBaseAddr == nullptr, "deinit me[%s] before destroy me", mDesc.mName);
}

void Framework::Shader::init(const Description &desc, Allocators *allocators)
{
	mDesc = desc;

	createShaderView();
	allocMemory(allocators);
}

void Framework::Shader::deinit(Allocators *allocators)
{
	SCE_GNM_ASSERT(allocators != nullptr);
	if (mDesc.mType == SHADER_VERTEX)
	{
		allocators->release(mFetchShaderAddr, SCE_KERNEL_WC_GARLIC, &mFetchShaderHandle);
		mFetchShaderAddr = nullptr;
	}
	allocators->release(mGpuBaseAddr, SCE_KERNEL_WC_GARLIC, &mHandle);
	mGpuBaseAddr = nullptr;
	allocators->release(mHeaderAddr, SCE_KERNEL_WB_ONION);
	mHeaderAddr = nullptr;

	SAFE_DELETE(mShaderView);
}

void Framework::Shader::bindAsShader(RenderContext *context) const
{
	switch (mDesc.mType)
	{
	case SHADER_VERTEX:
		context->setVertexShader(typeCast<BaseShaderView, VertexShaderView>(mShaderView));
		break;
	case SHADER_PIXEL:
		context->setPixelShader(typeCast<BaseShaderView, PixelShaderView>(mShaderView));
		break;
	case SHADER_COMPUTE:
		context->setComputeShader(typeCast<BaseShaderView, ComputeShaderView>(mShaderView));
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not support yet");
		break;
	}
}

void Framework::Shader::createShaderView()
{
	SCE_GNM_ASSERT(mDesc.mType < SHADER_TYPE_COUNT);
	SCE_GNM_ASSERT(mDesc.mDataPtr != nullptr);
	switch (mDesc.mType)
	{
	case SHADER_VERTEX:
		mShaderView = new VertexShaderView(mDesc.mDataPtr);
		break;
	case SHADER_PIXEL:
		mShaderView = new PixelShaderView(mDesc.mDataPtr);
		break;
	case SHADER_COMPUTE:
		mShaderView = new ComputeShaderView(mDesc.mDataPtr);
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not support yet");
		break;
	}
}

void Framework::Shader::allocMemory(Allocators *allocators)
{
	SCE_GNM_ASSERT(mShaderView != nullptr);
	SCE_GNM_ASSERT(allocators != nullptr);

	Gnm::SizeAlign _headerAlign = mShaderView->getHeaderSizeAlign();
	Gnm::SizeAlign _binaryAlign = mShaderView->getBinarySizeAlign();

	allocators->allocate(&mHeaderAddr, SCE_KERNEL_WB_ONION, _headerAlign);
	SCE_GNM_ASSERT_MSG(mHeaderAddr != nullptr, "Out of memory");
	allocators->allocate(&mGpuBaseAddr, SCE_KERNEL_WC_GARLIC, _binaryAlign, Gnm::kAlignmentOfShaderInBytes, &mHandle, mDesc.mName);
	SCE_GNM_ASSERT_MSG(mGpuBaseAddr != nullptr, "Out of memory");

	// transferData
	memcpy(mHeaderAddr, mShaderView->getHeaderPtr(), _headerAlign.m_size);
	memcpy(mGpuBaseAddr, mShaderView->getBinaryPtr(), _binaryAlign.m_size);

	if (mDesc.mType == SHADER_VERTEX)
	{
		VertexShaderView* _vs = typeCast<BaseShaderView, VertexShaderView>(mShaderView);
		Gnm::SizeAlign _fetchAlign = _vs->getFetchShaderSizeAlign();
		allocators->allocate(&mFetchShaderAddr, SCE_KERNEL_WC_GARLIC, _fetchAlign, Gnm::kResourceTypeFetchShaderBaseAddress, &mFetchShaderHandle, "%s - fetch shader", mDesc.mName);
		SCE_GNM_ASSERT_MSG(mFetchShaderAddr != nullptr, "Out of memory");
		_vs->assignAddress(mHeaderAddr, mGpuBaseAddr, mFetchShaderAddr);
	}
	else
	{
		mShaderView->assignAddress(mHeaderAddr, mGpuBaseAddr);
	}
}
