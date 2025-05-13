/**
 * @file profiling.h
 *
 */

#pragma once

#ifdef GL_ENABLE_PROFILING
#include <tracy/Tracy.hpp>

#define GL_PROFILE_SCOPE ZoneScoped
#define GL_PROFILE_SCOPE_N(p_X) ZoneScopedN(p_X)
#else
#define GL_PROFILE_SCOPE
#define GL_PROFILE_SCOPE_N(X)
#endif
