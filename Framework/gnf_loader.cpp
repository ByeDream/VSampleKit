/* SCE CONFIDENTIAL
PlayStation(R)4 Programmer Tool Runtime Library Release 03.508.201
* Copyright (C) 2016 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include "stdafx.h"

#include "gnf_loader.h"
// #include <unistd.h>
// #include <stdlib.h>
// #include <stdio.h>
#include "Memory/Allocators.h"
#include "gnf.h"
using namespace sce;

namespace
{
	/** @brief Loads a GNF file header and verifies that header contains valid information
	  * @param outHeader Pointer to GNF header structure to be filled with this call
	  * @param gnfFile File pointer to read this data from
	  * @return Zero if successful; otherwise, a non-zero error code.
	  */
    Framework::GnfError loadGnfHeader(sce::Gnf::Header *outHeader, FILE *gnfFile)
	{
		if (outHeader == NULL || gnfFile == NULL)
		{
			return Framework::kGnfErrorInvalidPointer;
		}
		outHeader->m_magicNumber     = 0;
		outHeader->m_contentsSize    = 0;
		fseek(gnfFile, 0, SEEK_SET);
		fread(outHeader, sizeof(sce::Gnf::Header), 1, gnfFile);
		if (outHeader->m_magicNumber != sce::Gnf::kMagic)
		{
			return Framework::kGnfErrorNotGnfFile;
		}
		return Framework::kGnfErrorNone;
	}

    sce::Gnf::UserData *getUserData(const Gnf::Contents *contents)
    {
        if(NULL == contents)
            return NULL;
        return const_cast<sce::Gnf::UserData *>
		(
			static_cast<const sce::Gnf::UserData *>
			(
				static_cast<const void *>
				(
					static_cast<const uint8_t*>(static_cast<const void *>(contents + 1)) 
					+ contents->m_numTextures * sizeof(sce::Gnm::Texture)
				)
			)
		);
    }

    // content size is sizeof(sce::Gnf::Contents)+gnfContents->m_numTextures*sizeof(sce::Gnm::Texture)+ paddings which is a variable of: gnfContents->alignment
    uint32_t computeContentSize(const sce::Gnf::Contents *gnfContents)
    {
        if(NULL == gnfContents)
            return 0;
        sce::Gnf::UserData *userData = getUserData(gnfContents);
		const int userDataSize = (Gnf::kUserDataMagic == userData->m_magicNumber) ? userData->m_size : 0;
        uint32_t headerSize = (uint32_t) (((const uint8_t *)userData  - (const uint8_t *)gnfContents) + userDataSize + sizeof(sce::Gnf::Header));
        // add the paddings
        uint32_t align = 1<<gnfContents->m_alignment; // actual alignment
	    size_t mask = align-1;
        uint32_t missAligned = ( headerSize & mask ); // number of bytes after the alignemnet point
	    if(missAligned) // if none zero we have to add paddings
	    {
            headerSize += align- missAligned;
        }
        return headerSize-sizeof(sce::Gnf::Header);
    }

	/** @brief Loads GNF contents and verifies that the contents contain valid information
	  * @param outContents Pointer to gnf contents to be read
	  * @param contentsSizeInBytes The size (int bytes) to read into gnfContents. Usually read from the Gnf::Header object.
	  * @param gnfFile File pointer to read this data from
	  * @return Zero if successful; otherwise, a non-zero error code.
	*/
	Framework::GnfError readGnfContents(sce::Gnf::Contents *outContents, uint32_t contentsSizeInBytes, FILE *gnfFile)
	{
		if(outContents == NULL || gnfFile == 0)
		{
			return Framework::kGnfErrorInvalidPointer;
		}
		fseek(gnfFile, sizeof(sce::Gnf::Header), SEEK_SET);
		size_t bytesRead = fread(outContents, 1, contentsSizeInBytes, gnfFile);
		if(bytesRead!=contentsSizeInBytes)
		{
			return Framework::kGnfErrorFileIsTooShort;
		}

		if(outContents->m_alignment>31)
		{
			return Framework::kGnfErrorAlignmentOutOfRange;
		}

        if( outContents->m_version == 1 )
	    {
	        if( (outContents->m_numTextures*sizeof(sce::Gnm::Texture) + sizeof(sce::Gnf::Contents)) != contentsSizeInBytes )
	        {
		        return Framework::kGnfErrorContentsSizeMismatch;
	        }
	    }else
        {
            if( outContents->m_version != sce::Gnf::kVersion )
	        {
		        return Framework::kGnfErrorVersionMismatch;
	        }else
            {
                if( computeContentSize(outContents) > contentsSizeInBytes)
    		        return Framework::kGnfErrorContentsSizeMismatch;
            }
        }

        return Framework::kGnfErrorNone;
	}
}

Framework::GnfError Framework::loadTextureFromGnf(sce::Gnm::Texture *outTexture, const char *fileName, uint8_t textureIndex, Allocators* allocators)
{
	if( (fileName==NULL) || (outTexture==NULL) )
	{
		return kGnfErrorInvalidPointer;
	}
	//SCE_GNM_ASSERT_MSG(access(fileName, R_OK) == 0, "** Asset Not Found: %s\n", fileName);

	GnfError result = kGnfErrorNone;
	FILE *gnfFile = NULL;
	sce::Gnf::Contents *gnfContents = NULL;
	do
	{
		gnfFile = fopen(fileName, "rb");
		if (gnfFile == 0)
		{
			result = kGnfErrorCouldNotOpenFile;
			break;
		}

		sce::Gnf::Header header;
		result = loadGnfHeader(&header,gnfFile);
		if (result != 0)
		{
			break;
		}

		gnfContents = (sce::Gnf::Contents *)malloc(header.m_contentsSize);
		result = readGnfContents( gnfContents, header.m_contentsSize, gnfFile );
		if (result)
		{
			break;
		}

		sce::Gnm::SizeAlign pixelsSa = getTexturePixelsSize(gnfContents, textureIndex);
		void *pixelsAddr = nullptr;
		Gnm::ResourceHandle pixelsHandle;
		allocators->allocate(&pixelsAddr, SCE_KERNEL_WC_GARLIC, pixelsSa, Gnm::kResourceTypeTextureBaseAddress, &pixelsHandle, fileName);
		if( pixelsAddr==0 ) // memory allocation failed
		{
			result = kGnfErrorOutOfMemory;
			break;
		}

		fseek(gnfFile, sizeof(sce::Gnf::Header) + header.m_contentsSize + getTexturePixelsByteOffset(gnfContents, textureIndex) , SEEK_SET);
		fread(pixelsAddr, pixelsSa.m_size, 1, gnfFile);
		*outTexture = *patchTextures(gnfContents, textureIndex, 1, &pixelsAddr);
	}
	while(0);

	free(gnfContents);
	if (gnfFile != NULL)
		fclose(gnfFile);
	return result;
}

Framework::GnfError Framework::loadTextureFromGnf(sce::Gnm::Texture *outTexture, const char *fileName, uint8_t textureIndex, IAllocator* allocator)
{
	Allocators allocators(*allocator, *allocator);
	return loadTextureFromGnf(outTexture, fileName, textureIndex, &allocators);
}

uint32_t computeContentsSize(uint32_t log2Alignment)
{
	uint32_t sizeOfContentsAndHeaderInBytes = sizeof(sce::Gnf::Header) + sizeof(sce::Gnf::Contents) + sizeof(sce::Gnm::Texture);
	const uint32_t align = 1 << log2Alignment;
	const size_t mask = align - 1;
	const uint32_t misAligned = (sizeOfContentsAndHeaderInBytes & mask); // number of bytes after the alignment point
	if(misAligned) // if not zero, we have to add padding
		sizeOfContentsAndHeaderInBytes += align - misAligned;
	return sizeOfContentsAndHeaderInBytes - sizeof(sce::Gnf::Header);
}

uint32_t floorLog2(uint32_t value)
{
	unsigned long result;
#ifndef __ORBIS__
	_BitScanReverse(&result, value);
#else
	result = sizeof(value) * 8 - __builtin_clz(value) - 1;
#endif
	return result;
}


Framework::GnfError Framework::saveTextureToGnf(const char *fileName, sce::Gnm::Texture *inTexture)
{
	if((fileName==NULL) || (inTexture==NULL))
	{
		return kGnfErrorInvalidPointer;
	}

	FILE *file = fopen(fileName, "wb");
	if(file == 0)
	{
		return kGnfErrorCouldNotOpenFile;
	}

	int32_t addrStatus;
	uint64_t gnfTexelSizeInBytes;
	sce::Gnm::AlignmentType gnfTexelAlign;
	addrStatus = sce::GpuAddress::computeTotalTiledTextureSize(&gnfTexelSizeInBytes, &gnfTexelAlign, inTexture);

	unsigned long log2Alignment = floorLog2(gnfTexelAlign);
	const uint32_t contentsSize = computeContentsSize(log2Alignment);
	const uint32_t outputFileSizeInBytes = static_cast<uint32_t>(sizeof(sce::Gnf::Header) + contentsSize + gnfTexelSizeInBytes);

	sce::Gnf::GnfFile gnfFile;
	gnfFile.header.m_magicNumber = sce::Gnf::kMagic;
	gnfFile.header.m_contentsSize = contentsSize;
	gnfFile.contents.m_version = sce::Gnf::kVersion;
	gnfFile.contents.m_numTextures = 1;
	gnfFile.contents.m_alignment = static_cast<uint8_t>(log2Alignment);
	gnfFile.contents.m_streamSize = outputFileSizeInBytes;
	if(fwrite(&gnfFile, 1, sizeof(gnfFile), file) != sizeof(gnfFile))
	{
		fclose(file);
		return kGnfErrorCouldNotWriteGnfFile;
	}

	sce::Gnm::Texture texture = *inTexture;
	texture.m_regs[0] = 0; 
	texture.m_regs[1] = (texture.m_regs[1] & 0xffffff00) | log2Alignment;
	texture.m_regs[7] = static_cast<uint32_t>(gnfTexelSizeInBytes);
	if(fwrite(&texture, 1, sizeof(texture), file) != sizeof(texture))
	{
		fclose(file);
		return kGnfErrorCouldNotWriteTexture;
	}

	if(fseek(file, sizeof(sce::Gnf::Header) + contentsSize, SEEK_SET) != 0)
	{
		fclose(file);
		return kGnfErrorCouldNotWriteContents;
	}

	if(fwrite(inTexture->getBaseAddress(), 1, gnfTexelSizeInBytes, file) != gnfTexelSizeInBytes)
	{
		fclose(file);
		return kGnfErrorCouldNotWriteTexels;
	}

	fclose(file);

	return kGnfErrorNone;
}
