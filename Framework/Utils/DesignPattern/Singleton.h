#pragma once

namespace Framework
{
	template<class T>
	class Singleton
	{
	public:
		static T *getInstance() {
			if (nullptr == mInstance) {
				mInstance = new T;
				mInstance->initialize();
			}
			return mInstance;
		}

		static void destory() {
			SAFE_DELETE(mInstance);
		}

		virtual void initialize() {}
		virtual void finitialize() {}
	protected:
		Singleton() {}
		virtual ~Singleton() {}

		static T *mInstance;
	};

	template<class T>
	Framework::Singleton<T>::mInstance;
}
