#pragma once

namespace Framework
{
	struct IAllocator;
	class Allocators;

	/**
	* @brief Indicates the result of a TGA load operation
	*/
	enum TgaError
	{
		kTgaErrorNone = 0, // Operation was successful; no error
		kTgaErrorInvalidPointer = -1, // Caller passed an invalid/NULL pointer to a TGA loader function
		kTgaErrorNotTGAFile = -2, // Attempted to load a file that isn't a TGA file (bad magic number in header)
		kTgaErrorCorruptHeader = -3, // Attempted to load a TGA file with corrupt header data
		kTgaErrorFileIsTooShort = -4, // Attempted to load a TGA file whose size is smaller than the size reported in its header
		kTgaErrorVersionMismatch = -5, // Attempted to load a TGA file created by a different version of the TGA code
		kTgaErrorAlignmentOutOfRange = -6, // Attempted to load a TGA file with corrupt header data (surface alignment > 2^31 bytes)
		kTgaErrorContentsSizeMismatch = -7, // Attempted to load a TGA file with corrupt header data (wrong size in TGA header contents)
		kTgaErrorCouldNotOpenFile = -8, // Unable to open a file for reading
		kTgaErrorOutOfMemory = -9, // Internal memory allocation failed
		kTgaErrorCouldNotWriteTgaFile = -10, // Attempted to write the TGA header to the file, but fwrite failed.
		kTgaErrorCouldNotWriteTexture = -11, // Attempted to write the Texture object to the file, but fwrite failed.
		kTgaErrorCouldNotWriteContents = -12, // Attempted to fseek to the end of the Contents structure, but fseek failed.
		kTgaErrorCouldNotWriteTexels = -13, // Attempted to write the texels to the file, but fwrite failed.
		kTgaErrorInitTextureHeaderFailed = -14,
		kTgaErrorInitTextureDataFailed = -15,
	};

	TgaError loadTextureFromTga(sce::Gnm::Texture *outTexture, const char *fileName, Allocators* allocators);
	TgaError loadTextureFromTga(sce::Gnm::Texture *outTexture, const char *fileName, IAllocator* allocator);
	TgaError saveTextureToTga(const char *fileName, sce::Gnm::Texture *inTexture);
}