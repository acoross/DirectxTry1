#pragma once

class GameTimer
{
public:
	GameTimer();

	float DeltaTime() const;
	float TotalTime() const;

	void Tick();
	void Start();
	void Stop();
	void Reset();

private:
	double secondsPerCount_{ 0 };
	double deltaTime_{ -1.0 };

	__int64 prevTick_{ 0 };
	__int64 currTick_{ 0 };

	bool stopped_{ false };
	__int64 baseTick_{ 0 };
	__int64 stopTick_{ 0 };
	__int64 pausedTickAmount_{ 0 };
};