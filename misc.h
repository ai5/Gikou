#pragma once

#include <chrono>
#include <thread>

// ms�P�ʂł̎��Ԍv�������K�v�Ȃ��̂ł����TimePoint�^�̂悤�Ɉ����B
typedef std::chrono::milliseconds::rep TimePoint;

// ms�P�ʂŌ��ݎ�����Ԃ�
inline TimePoint now() {
	return std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::steady_clock::now().time_since_epoch()).count();
}

// �w�肳�ꂽ�~���b����sleep����B
inline void sleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::microseconds(ms));
}

struct Timer
{
	// �^�C�}�[������������B�ȍ~�Aelapsed()��init()���Ă���̌o�ߎ��Ԃ�������B
	void reset() { startTime = startTimeFromPonderhit = now(); }

	int elapsed() const { return int(now() - startTime); }

private:
	// �T���J�n����
	TimePoint startTime;

	TimePoint startTimeFromPonderhit;
};


