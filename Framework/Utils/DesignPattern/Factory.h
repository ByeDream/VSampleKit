#pragma once

#include <map>

namespace Framework
{
	class Factory
	{
	public:
		class Product
		{
		public:
			virtual ~Product() {}
		};

		class Worker
		{
		public:
			virtual ~Worker() {}
			virtual Product *doWork() const = 0;
		};

		template<class P>
		class AutoWorker : public Worker
		{
		public:
			virtual Product *doWork() const { return dynamic_cast<Product *>(new P); }
		};

		virtual ~Factory() 
		{
			for (auto itor = mWorkerGroup.begin(); itor != mWorkerGroup.end(); itor++)
			{
				SAFE_DELETE(itor->second);
			}
			mWorkerGroup.clear();
		}

		virtual void RegisterWorker(U32 id, const Worker *worker)
		{
			mWorkerGroup[id] = worker;
		}
		virtual void UnregisterWorker(U32 id)
		{
			auto itor = mWorkerGroup.find(id);
			if (itor != mWorkerGroup.end())
				mWorkerGroup.erase(itor);
		}
		virtual Product *Produce(U32 id)
		{
			Product *product = NULL;
			auto itor = mWorkerGroup.find(id);
			if (itor != mWorkerGroup.end())
				product = itor->second->doWork();
			return product;
		}

	protected:
		std::map<U32, const Worker *> mWorkerGroup;
	};

	template<class W>
	class WorkerRegisterer
	{
	public:
		WorkerRegisterer(Factory *factory, U32 id)
		{
			factory->RegisterWorker(id, new W);
		}
	};

#	define REGISTER_WORKER(workerClass, id, pFactory) \
	Framework::WorkerRegisterer<workerClass> __reg((pFactory), (id));

#	define REGISTER_AUTO_WORKER(productClass, id, pFactory) \
	Framework::WorkerRegisterer<Framework::Factory::AutoWorker<productClass>> __reg((pFactory), (id));
}
