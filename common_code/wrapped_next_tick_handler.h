#ifndef __WRAPPED_NEXT_TICK_HANDLER_H
#define __WRAPPED_NEXT_TICK_HANDLER_H

#include "wrapped_capture.h"

template <typename Poster, typename Handler, bool = false>
class wrapped_next_tick_handler
{
public:
	template <typename H>
	wrapped_next_tick_handler(Poster* poster, H&& handler)
		: _poster(poster),
		_handler(TRY_MOVE(handler))
	{
	}

	wrapped_next_tick_handler(wrapped_next_tick_handler&& sp)
		:_poster(sp._poster),
		_handler(std::move(sp._handler))
	{
	}

	wrapped_next_tick_handler& operator=(wrapped_next_tick_handler&& sp)
	{
		_poster = sp._poster;
		_handler = std::move(sp._handler);
	}

#if _MSC_VER == 1600
	void operator()()
	{
		_poster->next_tick(_handler);
	}

	void operator()() const
	{
		_poster->next_tick(_handler);
	}

	template <typename Arg1>
	void operator()(Arg1&& arg1)
	{
		_poster->next_tick(wrap_capture(_handler, TRY_MOVE(arg1)));
	}

	template <typename Arg1>
	void operator()(Arg1&& arg1) const
	{
		_poster->next_tick(wrap_capture(_handler, TRY_MOVE(arg1)));
	}

	template <typename Arg1, typename Arg2>
	void operator()(Arg1&& arg1, Arg2&& arg2)
	{
		_poster->next_tick(wrap_capture(_handler, TRY_MOVE(arg1), TRY_MOVE(arg2)));
	}

	template <typename Arg1, typename Arg2>
	void operator()(Arg1&& arg1, Arg2&& arg2) const
	{
		_poster->next_tick(wrap_capture(_handler, TRY_MOVE(arg1), TRY_MOVE(arg2)));
	}

	template <typename Arg1, typename Arg2, typename Arg3>
	void operator()(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3)
	{
		_poster->next_tick(wrap_capture(_handler, TRY_MOVE(arg1), TRY_MOVE(arg2), TRY_MOVE(arg3)));
	}

	template <typename Arg1, typename Arg2, typename Arg3>
	void operator()(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3) const
	{
		_poster->next_tick(wrap_capture(_handler, TRY_MOVE(arg1), TRY_MOVE(arg2), TRY_MOVE(arg3)));
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	void operator()(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4)
	{
		_poster->next_tick(wrap_capture(_handler, TRY_MOVE(arg1), TRY_MOVE(arg2), TRY_MOVE(arg3), TRY_MOVE(arg4)));
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	void operator()(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4) const
	{
		_poster->next_tick(wrap_capture(_handler, TRY_MOVE(arg1), TRY_MOVE(arg2), TRY_MOVE(arg3), TRY_MOVE(arg4)));
	}

#elif _MSC_VER > 1600
	template <typename... Args>
	void operator()(Args&&... args)
	{
		_poster->next_tick(wrap_capture(_handler, TRY_MOVE(args)...));
	}

	template <typename... Args>
	void operator()(Args&&... args) const
	{
		_poster->next_tick(wrap_capture(_handler, TRY_MOVE(args)...));
	}
#endif

	Poster* _poster;
	Handler _handler;
};
//////////////////////////////////////////////////////////////////////////

template <typename Poster, typename Handler>
class wrapped_next_tick_handler<Poster, Handler, true>
{
public:
	template <typename H>
	wrapped_next_tick_handler(Poster* poster, H&& handler)
		: _poster(poster),
		_handler(TRY_MOVE(handler))
#ifdef _DEBUG
		, _checkOnce(new boost::atomic<bool>(false))
#endif
	{
	}

	wrapped_next_tick_handler(wrapped_next_tick_handler&& sd)
		:_poster(sd._poster),
		_handler(std::move(sd._handler))
#ifdef _DEBUG
		, _checkOnce(sd._checkOnce)
#endif
	{
	}

	wrapped_next_tick_handler& operator=(wrapped_next_tick_handler&& sd)
	{
		_poster = sd._poster;
		_handler = std::move(sd._handler);
#ifdef _DEBUG
		_checkOnce = sd._checkOnce;
#endif
	}

#if _MSC_VER == 1600
	void operator()()
	{
		assert(!_checkOnce->exchange(true));
		_poster->next_tick(FORCE_MOVE(_handler));
	}

	template <typename Arg1>
	void operator()(Arg1&& arg1)
	{
		assert(!_checkOnce->exchange(true));
		_poster->next_tick(wrap_capture(FORCE_MOVE(_handler), FORCE_MOVE(arg1)));
	}

	template <typename Arg1, typename Arg2>
	void operator()(Arg1&& arg1, Arg2&& arg2)
	{
		assert(!_checkOnce->exchange(true));
		_poster->next_tick(wrap_capture(FORCE_MOVE(_handler), FORCE_MOVE(arg1), FORCE_MOVE(arg2)));
	}

	template <typename Arg1, typename Arg2, typename Arg3>
	void operator()(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3)
	{
		assert(!_checkOnce->exchange(true));
		_poster->next_tick(wrap_capture(FORCE_MOVE(_handler), FORCE_MOVE(arg1), FORCE_MOVE(arg2), FORCE_MOVE(arg3)));
	}

	template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	void operator()(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4)
	{
		assert(!_checkOnce->exchange(true));
		_poster->next_tick(wrap_capture(FORCE_MOVE(_handler), FORCE_MOVE(arg1), FORCE_MOVE(arg2), FORCE_MOVE(arg3), FORCE_MOVE(arg4)));
	}
#elif _MSC_VER > 1600
	template <typename... Args>
	void operator()(Args&&... args)
	{
		_poster->next_tick(wrap_capture(FORCE_MOVE(_handler), FORCE_MOVE(args)...));
	}
#endif

	Poster* _poster;
	Handler _handler;
#ifdef _DEBUG
	std::shared_ptr<boost::atomic<bool> > _checkOnce;
#endif
};

#endif