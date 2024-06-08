#include "core/uid.h"

static std::random_device random_device;

static thread_local std::mt19937_64 engine(random_device());
static thread_local std::uniform_int_distribution<uint64_t>
		uniform_distribution;

UID::UID() : value(uniform_distribution(engine)) {}

UID::UID(const uint64_t& p_uuid) : value(p_uuid) {}

UID::UID(uint64_t&& p_uuid) : value(std::move(p_uuid)) {}

UID& UID::operator=(const UID& p_other) {
	value = (uint64_t)p_other;
	return *this;
}

UID& UID::operator=(UID&& p_other) {
	value = (uint64_t)p_other;
	return *this;
}

UID& UID::operator=(const uint64_t& p_other) {
	value = p_other;
	return *this;
}

UID& UID::operator=(uint64_t&& p_other) {
	value = p_other;
	return *this;
}
