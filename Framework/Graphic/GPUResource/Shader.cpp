#include "stdafx.h"

#include "Shader.h"

#include "Memory/Allocators.h"
#include "GPUResourceViews.h"
#include "ChunkBasedRenderContext/RenderContext.h"

using namespace sce;

Framework::Shader::Shader()
{

}

Framework::Shader::~Shader()
{
	SCE_GNM_ASSERT_MSG(mHeaderAddr == nullptr && mBinaryAddr == nullptr, "deinit me[%s] before destroy me", mDesc.mName);
}

void Framework::Shader::init(const BaseGPUResource::Description *desc, Allocators *allocators)
{
	const Description *_desc = typeCast<Description>(desc);
	mDesc = *_desc;

	createShaderView();
	allocMemory(allocators);

	mShaderView->assignFence(mFence);
}

void Framework::Shader::deinit(Allocators *allocators)
{
	SCE_GNM_ASSERT(allocators != nullptr);
	if (mDesc.mStage == Gnm::kShaderStageVs)
	{
		allocators->release(mFetchShaderAddr, SCE_KERNEL_WC_GARLIC, &mFetchShaderHandle);
		mFetchShaderAddr = nullptr;
	}
	allocators->release(mBinaryAddr, SCE_KERNEL_WC_GARLIC, &mBinaryHandle);
	mBinaryAddr = nullptr;
	allocators->release(mHeaderAddr, SCE_KERNEL_WB_ONION);
	mHeaderAddr = nullptr;

	SAFE_DELETE(mShaderView);
}

void Framework::Shader::bindAsShader(RenderContext *context) const
{
	switch (mDesc.mStage)
	{
	case Gnm::kShaderStageVs:
		context->setVertexShader(typeCast<VertexShaderView>(mShaderView));
		break;
	case Gnm::kShaderStagePs:
		context->setPixelShader(typeCast<PixelShaderView>(mShaderView));
		break;
	case Gnm::kShaderStageCs:
		context->setComputeShader(typeCast<ComputeShaderView>(mShaderView));
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not support yet");
		break;
	}
}

void Framework::Shader::createShaderView()
{
	SCE_GNM_ASSERT(mDesc.mStage < Gnm::kShaderStageCount);
	SCE_GNM_ASSERT(mDesc.mDataPtr != nullptr);
	switch (mDesc.mStage)
	{
	case Gnm::kShaderStageVs:
		mShaderView = new VertexShaderView(mDesc.mDataPtr);
		break;
	case Gnm::kShaderStagePs:
		mShaderView = new PixelShaderView(mDesc.mDataPtr);
		break;
	case Gnm::kShaderStageCs:
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
	allocators->allocate(&mBinaryAddr, SCE_KERNEL_WC_GARLIC, _binaryAlign, Gnm::kResourceTypeShaderBaseAddress, &mBinaryHandle, mDesc.mName);
	SCE_GNM_ASSERT_MSG(mBinaryAddr != nullptr, "Out of memory");

	// transferData
	memcpy(mHeaderAddr, mShaderView->getHeaderPtr(), _headerAlign.m_size);
	memcpy(mBinaryAddr, mShaderView->getBinaryPtr(), _binaryAlign.m_size);

	if (mDesc.mStage == Gnm::kShaderStageVs)
	{
		VertexShaderView* _vs = typeCast<VertexShaderView>(mShaderView);
		Gnm::SizeAlign _fetchAlign = _vs->getFetchShaderSizeAlign();
		allocators->allocate(&mFetchShaderAddr, SCE_KERNEL_WC_GARLIC, _fetchAlign, Gnm::kResourceTypeFetchShaderBaseAddress, &mFetchShaderHandle, "%s - fetch shader", mDesc.mName);
		SCE_GNM_ASSERT_MSG(mFetchShaderAddr != nullptr, "Out of memory");
		_vs->assignAddress(mHeaderAddr, mBinaryAddr, mFetchShaderAddr);
	}
	else
	{
		mShaderView->assignAddress(mHeaderAddr, mBinaryAddr);
	}
}
