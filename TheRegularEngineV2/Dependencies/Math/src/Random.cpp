#include "Random.h"
#include <chrono>

constexpr float PI = 3.141592653f;

Random::state_type Random::state = 0;
bool Random::isInitialized = false;
std::minstd_rand Random::randomEngine{};

void Random::SetState(state_type state_) { state = state_; isInitialized = false; }
Random::state_type Random::GetState() { return state; }

float Random::RangeFloat(float minInclusive, float maxInclusive)
{
	isInitialized || Initialize();

	// ensure that min is actually smaller than max
	if (minInclusive > maxInclusive)
		std::swap(minInclusive, maxInclusive);

	// holy cow this looks nuts, this better work...
	// shoutout to URVO, hell yeah
	return std::uniform_real_distribution
	{ minInclusive, std::nextafter(maxInclusive, std::numeric_limits<float>::max()) }
	(randomEngine);
}

int Random::RangeInt(int minInclusive, int maxExclusive)
{
	int maxInclusive = maxExclusive - 1;
	isInitialized || Initialize();

	// ensure that min is actually smaller than max
	if (minInclusive > maxInclusive)
		std::swap(minInclusive, maxInclusive);

	// holy cow this looks nuts, this better work...
	// shoutout to URVO, hell yeah
	return std::uniform_int_distribution{ minInclusive, maxInclusive }(randomEngine);
}

// Color ColorHSV();

float Random::Value01() { return RangeFloat(0, 1); }

// quaternion?? Rotation();

Random::vec2_type Random::InsideUnitCircle()
{
	const float radius = Value01();
	const float radians = RangeFloat(0, 2 * PI);
	return { radius * cosf(radians), radius * sinf(radians) };
}

Random::vec3_type Random::InsideUnitSphere()
{
	const float radius = Value01();
	const vec3_type pt{ OnUnitSphere() };
	return { pt.x * radius, pt.y * radius, pt.z * radius };
	// return OnUnitSphere() * Value01();
}

Random::vec3_type Random::OnUnitSphere()
{
	// orite, trust https://en.wikipedia.org/wiki/N-vector#Converting_latitude/longitude_to_n-vector
	const float longitude = RangeFloat(0, 2 * PI);
	const float latitude = RangeFloat(0, 2 * PI);
	return { cos(latitude) * cos(longitude), cos(latitude) * sin(longitude), sin(latitude) };
}

// ============================================================
bool Random::Initialize()
{
	// I hope this will be fine!!
	if (!state)
		state = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	randomEngine.seed(static_cast<std::minstd_rand::result_type>(state));
	return isInitialized = true;
}
