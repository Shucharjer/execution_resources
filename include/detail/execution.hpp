#pragma once

#if __cplusplus > 202302L && __has_include(<execution>)
#include <execution>
namespace neutron::execution {
using namespace ::std::execution;
}
#elif __has_include(<stdexec/stdexec.hpp>)
#include <stdexec/stdexec.hpp>
namespace neutron::execution {
using namespace stdexec;
}
#else
#error "do not have supported execution header"
#endif