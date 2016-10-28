#include "stdafx.h"

#include "FileIO.h"

Framework::FileIO::FileIO(const char *path)
{
	mPath = path;
	mHandle = fopen(mPath, "rb"); // TODO writing
	if (mHandle)
	{
		fseek(mHandle, 0, SEEK_END);
		mByteSize = ftell(mHandle);
		fseek(mHandle, 0, SEEK_SET);
		mName = path; //TODO, remove path only keep file name.
	}
}

Framework::FileIO::~FileIO()
{
	unload();
	if (mHandle != nullptr)
		fclose(mHandle);
}

void Framework::FileIO::load()
{
	SCE_GNM_ASSERT_MSG(mHandle != nullptr && mByteSize > 0 && mByteSize <= MAX_VALUE_32, "Load file[%s] failed", mName);
	mBuffer = (U8 *)malloc(mByteSize);
	SCE_GNM_ASSERT_MSG(mBuffer != nullptr, "Out of memory");
	fread(mBuffer, 1, mByteSize, mHandle);
}

void Framework::FileIO::unload()
{
	if (mBuffer != nullptr)
	{
		free(mBuffer);
		mBuffer = nullptr;
	}
}
