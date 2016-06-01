#pragma once

#include <chrono>
#include <thread>

// ms単位での時間計測しか必要ないのでこれをTimePoint型のように扱う。
typedef std::chrono::milliseconds::rep TimePoint;

// ms単位で現在時刻を返す
inline TimePoint now() {
	return std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::steady_clock::now().time_since_epoch()).count();
}

// 指定されたミリ秒だけsleepする。
inline void sleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::microseconds(ms));
}

struct Timer
{
	// タイマーを初期化する。以降、elapsed()でinit()してからの経過時間が得られる。
	void reset() { startTime = startTimeFromPonderhit = now(); }

	int elapsed() const { return int(now() - startTime); }

private:
	// 探索開始時間
	TimePoint startTime;

	TimePoint startTimeFromPonderhit;
};


