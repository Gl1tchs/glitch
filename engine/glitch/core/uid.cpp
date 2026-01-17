#include "glitch/core/uid.h"

namespace gl {

static uint32_t s_counter = 0;

UID::UID() : value(++s_counter) {}

UID::UID(const uint32_t& uid) : value(uid) {}

UID::UID(uint32_t&& uid) : value(std::move(uid)) {}

UID& UID::operator=(const UID& other) {
	value = (uint32_t)other;
	return *this;
}

UID& UID::operator=(UID&& other) {
	value = (uint32_t)other;
	return *this;
}

UID& UID::operator=(const uint32_t& other) {
	value = other;
	return *this;
}

UID& UID::operator=(uint32_t&& other) {
	value = other;
	return *this;
}

void to_json(json& j, const UID& uid) { j = uid.value; }

void from_json(const json& j, UID& uid) { j.get_to(uid.value); }

} //namespace gl