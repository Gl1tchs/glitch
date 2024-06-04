#include "core/uid.h"

static std::random_device random_device;

static thread_local std::mt19937_64 engine(random_device());
static thread_local std::uniform_int_distribution<uint64_t>
		uniform_distribution;

UID::UID() : value(uniform_distribution(engine)) {}

UID::UID(const uint64_t& uuid) : value(uuid) {}

UID::UID(uint64_t&& uuid) : value(std::move(uuid)) {}

UID& UID::operator=(const UID& other) {
	value = (uint64_t)other;
	return *this;
}

UID& UID::operator=(UID&& other) {
	value = (uint64_t)other;
	return *this;
}

UID& UID::operator=(const uint64_t& other) {
	value = other;
	return *this;
}

UID& UID::operator=(uint64_t&& other) {
	value = other;
	return *this;
}
