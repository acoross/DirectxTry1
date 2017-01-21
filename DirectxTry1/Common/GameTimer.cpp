#include "GameTimer.h"
#include <Windows.h>

namespace Acoross {
	namespace DirectX {

		GameTimer::GameTimer()
		{
			__int64 countsPerSec;
			::QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
			secondsPerCount_ = 1.0 / (double)countsPerSec;
		}

		float GameTimer::DeltaTime() const
		{
			return (float)deltaTime_;
		}

		void GameTimer::Start()
		{
			__int64 startTime;
			::QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

			if (stopped_)
			{
				pausedTickAmount_ += (startTime - stopTick_);

				prevTick_ = startTime;
				stopTick_ = 0;
				stopped_ = false;
			}
		}

		void GameTimer::Stop()
		{
			if (!stopped_)
			{
				__int64 currTime;
				::QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

				stopTick_ = currTime;
				stopped_ = true;
			}
		}

		void GameTimer::Reset()
		{
			__int64 currTime;
			::QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

			baseTick_ = currTime;
			prevTick_ = currTime;
			stopTick_ = 0;
			stopped_ = false;
		}

		void GameTimer::Tick()
		{
			if (stopped_)
			{
				deltaTime_ = 0.0;
				return;
			}

			__int64 currTime;
			::QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
			currTick_ = currTime;

			deltaTime_ = (currTick_ - prevTick_) * secondsPerCount_;

			prevTick_ = currTick_;

			if (deltaTime_ < 0.0)
			{
				deltaTime_ = 0.0;
			}
		}

		float GameTimer::TotalTime() const
		{

			return 0.0f;
		}
	}
}