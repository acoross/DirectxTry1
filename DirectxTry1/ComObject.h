#pragma once

namespace Acoross {
	namespace DirectX {

		template <typename TComInterface>
		inline static void ReleaseCom(TComInterface** x)
		{
			if (x && *x)
			{
				(*x)->Release();
				(*x) = nullptr;
			}
		}

		// This is NOT thread-safe.
		// This is for client.
		template <typename TComInterface>
		class ComObject
		{
		public:
			ComObject()
			{}

			ComObject(TComInterface* obj)
				: raw(obj)
			{}

			ComObject(ComObject& rhs) = delete;
			ComObject& operator=(ComObject& rhs) = delete;

			ComObject(ComObject&& rhs)
			{
				std::swap(raw, rhs.raw);
			}

			ComObject& operator=(ComObject&& rhs)
			{
				std::swap(raw, rhs.raw);
				return *this;
			}

			void Release()
			{
				if (raw)
				{
					raw->Release();
					raw = nullptr;
				}
			}

			TComInterface* Get()
			{
				return raw;
			}

			TComInterface* const* Raw()
			{
				return &raw;
			}

			TComInterface** Reset()
			{
				Release();
				return &raw;
			}

			~ComObject()
			{
				Release();
			}

			TComInterface* operator->()
			{
				return raw;
			}

		private:
			TComInterface* raw{ nullptr };
		};

		template<typename TComInterface>
		inline ComObject<TComInterface> make_com(TComInterface* obj)
		{
			return ComObject<TComInterface>(obj);
		}
	}
}