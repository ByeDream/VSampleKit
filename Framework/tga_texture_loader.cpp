#include "stdafx.h"
#include "tga_texture_loader.h"
#include "tga_reader.h"
#include "Memory/Allocators.h"

using namespace sce;

namespace
{
	Framework::TgaError LoadTexturesHeader(Gnm::Texture *outTexture, unsigned char *buffer)
	{
		if ((outTexture == nullptr) || (buffer == nullptr))
		{
			return Framework::kTgaErrorInvalidPointer;
		}

		int width = tgaGetWidth(buffer);
		int height = tgaGetHeight(buffer);

		Gnm::TileMode tileMode;
		int32_t res = GpuAddress::computeSurfaceTileMode(Gnm::kGpuModeBase, &tileMode, GpuAddress::kSurfaceTypeTextureFlat, Gnm::kDataFormatR8G8B8A8Uint, 1);
		if (res != (int32_t)GpuAddress::kStatusSuccess)
		{
			return Framework::kTgaErrorInitTextureHeaderFailed;
		}
		Gnm::TextureSpec textureSpec;
		textureSpec.init();
		textureSpec.m_width = width;
		textureSpec.m_height = height;
		textureSpec.m_depth = 1;
		textureSpec.m_pitch = 0;
		textureSpec.m_numFragments = Gnm::kNumFragments1;
		textureSpec.m_textureType = Gnm::kTextureType2d;
		textureSpec.m_numMipLevels = 1;
		textureSpec.m_numSlices = 1;
		textureSpec.m_format = Gnm::kDataFormatR8G8B8A8Unorm;
		textureSpec.m_tileModeHint = tileMode;
		textureSpec.m_minGpuMode = sce::Gnm::getGpuMode(); //@@mp: needed to run on NEO

		int32_t status = outTexture->init(&textureSpec);
		if (status != SCE_GNM_OK)
		{
			return Framework::kTgaErrorInitTextureHeaderFailed;
		}
		return Framework::kTgaErrorNone;
	}

	Framework::TgaError LoadTexturesData(Gnm::Texture *outTexture, unsigned char *buffer, Framework::Allocators *allocators, const char *name = nullptr)
	{
		if ((outTexture == nullptr) || (buffer == nullptr) || (allocators == nullptr))
		{
			return Framework::kTgaErrorInvalidPointer;
		}

		uint32_t pixelsSizeInByte = 4 * outTexture->getWidth() * outTexture->getHeight();
		int *pixels = tgaRead(buffer, TGA_READER_ABGR);
		
		Gnm::SizeAlign gpuMemAlign = outTexture->getSizeAlign();
		if (gpuMemAlign.m_size != pixelsSizeInByte)
		{
			return Framework::kTgaErrorContentsSizeMismatch;
		}
		
		void *gpuBaseAddrs = nullptr;
		Gnm::ResourceHandle pixelsHandle;
		allocators->allocate(&gpuBaseAddrs, SCE_KERNEL_WC_GARLIC, gpuMemAlign, Gnm::kResourceTypeTextureBaseAddress, &pixelsHandle, name);
		if (gpuBaseAddrs == nullptr) // memory allocation failed
		{
			return Framework::kTgaErrorOutOfMemory;
		}
		outTexture->setBaseAddress(gpuBaseAddrs);



		uint32_t width = outTexture->getWidth();
		uint32_t height = outTexture->getHeight();


		GpuAddress::TilingParameters tp;
		uint64_t mipOffset;
		uint64_t mipSize;
		GpuAddress::computeTextureSurfaceOffsetAndSize(&mipOffset, &mipSize, outTexture, 0, 0);
		//uint32_t srcPixelsOffset = ComputeScimitarTextureMapDataOffset(width, height, 1, 1, outTexture->getDataFormat(), false, 0, 0);
		uint32_t srcPixelsOffset = 0;

		tp.initFromTexture(outTexture, 0, 0);
		GpuAddress::tileSurface((uint8_t*)gpuBaseAddrs + mipOffset, (const uint8_t*)pixels + srcPixelsOffset, &tp);
		//memcpy_s(gpuBaseAddrs, gpuMemAlign.m_size, pixels, pixelsSizeInByte);  //@@LRF for tga.
		return Framework::kTgaErrorNone;
	}
}

Framework::TgaError Framework::loadTextureFromTga(sce::Gnm::Texture *outTexture, const char *fileName, IAllocator* allocator)
{
	Allocators allocators(*allocator, *allocator);
	return loadTextureFromTga(outTexture, fileName, &allocators);
}

Framework::TgaError Framework::loadTextureFromTga(sce::Gnm::Texture *outTexture, const char *fileName, Allocators* allocators)
{
	if ((fileName == nullptr) || (outTexture == nullptr))
	{
		return kTgaErrorInvalidPointer;
	}
	//SCE_GNM_ASSERT_MSG(access(fileName, R_OK) == 0, "** Asset Not Found: %s\n", fileName);

	TgaError result = kTgaErrorNone;
	FILE *tgaFile = nullptr;
	unsigned char *buffer = nullptr;
	do 
	{
		tgaFile = fopen(fileName, "rb");
		if (tgaFile == nullptr)
		{
			result = kTgaErrorCouldNotOpenFile;
			break;
		}
		int size;
		fseek(tgaFile, 0, SEEK_END);
		size = ftell(tgaFile);
		fseek(tgaFile, 0, SEEK_SET);
		buffer = (unsigned char *)tgaMalloc(size);
		if (buffer == nullptr) // memory allocation failed
		{
			result = kTgaErrorOutOfMemory;
			break;
		}
		fread(buffer, 1, size, tgaFile);

		result = LoadTexturesHeader(outTexture, buffer);
		if (result)
			break;

		result = LoadTexturesData(outTexture, buffer, allocators, fileName);
	} while (0);

	if (buffer != nullptr)
		tgaFree(buffer);

	if (tgaFile != nullptr)
		fclose(tgaFile);
	return result;
}

Framework::TgaError Framework::saveTextureToTga(const char *fileName, sce::Gnm::Texture *inTexture)
{
	// TODO
	return kTgaErrorNone;

}

