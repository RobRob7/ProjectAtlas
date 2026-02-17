#ifndef PROFILER_H
#define PROFILER_H

#include <string>
#include <chrono>
#include <map>
#include <utility>

using steadyClock = std::chrono::steady_clock;

struct CPUGPUCollection
{
public:
	static CPUGPUCollection& getInstance()
	{
		static CPUGPUCollection cpugpuInstance;
		return cpugpuInstance;
	} // end of getInstance()

	void flush()
	{
		for (auto& pair : cpugpuTimeMap)
		{
			pair.second.first = 0.0;
			pair.second.second = 0.0;
		} // end for
	} // end of flush()

	CPUGPUCollection(const CPUGPUCollection&) = delete;
	CPUGPUCollection& operator=(const CPUGPUCollection&) = delete;
	CPUGPUCollection(CPUGPUCollection&&) = delete;
	CPUGPUCollection& operator=(CPUGPUCollection&&) = delete;

	// all CPU time trackers
	std::map<std::string, std::pair<double, double>> cpugpuTimeMap =
	{
		{"Gbuffer", {0.0, 0.0}},
		{"SSAO",	{0.0, 0.0}},
		{"Debug",	{0.0, 0.0}},
		{"Water",	{0.0, 0.0}},
		{"FXAA",	{0.0, 0.0}},
		{"Fog",		{0.0, 0.0}},
		{"Present",	{0.0, 0.0}},


		{"Render Time",	{0.0, 0.0}},
	};

private:
	CPUGPUCollection() = default;
};

struct CPUTimer
{
	steadyClock::time_point t0;
	double& outMs;

	explicit CPUTimer(double& out) 
		: t0(steadyClock::now()), outMs(out)
	{
		outMs = 0.0;
	} // end of constructor

	~CPUTimer()
	{
		outMs = std::chrono::duration<double, std::milli>(steadyClock::now() - t0).count();
	} // end of destructor
};

#endif
