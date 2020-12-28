#include "rpm_measure_sim.hpp"
#include <cmath>

RpmMeasureSim::RpmMeasureSim()
	: m_cycle_time(50.f)
	, m_curr_tick(0)
	, m_max_tick(200)
{

}

RpmMeasureSim::~RpmMeasureSim()
{
}

void RpmMeasureSim::initialize(Globe& globe)
{
	RpmMeasureBase::initialize(globe);
	m_curr_tick = 0;
}

RpmMeasureSim::RpmData RpmMeasureSim::getRpmData()
{
	m_curr_tick = (m_curr_tick + 1) % m_max_tick;
	int curr_pos = static_cast<int>(std::round(static_cast<float>(m_curr_tick) * m_temporal_resolution / (m_max_tick - 1)));
	return {m_cycle_time, curr_pos};

}
