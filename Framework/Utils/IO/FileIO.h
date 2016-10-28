#pragma once

namespace Framework
{
	class FileIO
	{
	public:
		FileIO(const char *path);
		~FileIO();
		
		inline bool				isExist() const { return mHandle; }
		inline U32				getByteSize() const { return mByteSize; }
		inline const char *		getName() const { return mName; }
		inline const U8 *		getBuffer() const { return mBuffer; }

		void					load();
		void					unload();

	private:
		const char *			mPath{ nullptr };
		const char *			mName{ nullptr };
		FILE *					mHandle{ nullptr };
		U8 *					mBuffer{ nullptr };
		U32						mByteSize{ 0 };
	};
}