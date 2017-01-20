#pragma once

template <typename TComInterface>
inline static void ReleaseCom(TComInterface** x)
{
	if (x && *x)
	{
		(*x)->Release();
		(*x) = nullptr;
	}
}

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

	TComInterface* Get()
	{
		return raw;
	}

	TComInterface* const* Raw()
	{
		return &raw;
	}

	~ComObject()
	{
		if (raw)
		{
			raw->Release();
			raw = nullptr;
		}
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