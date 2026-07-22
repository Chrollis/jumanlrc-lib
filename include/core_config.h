#pragma once
#ifndef __JUMANPP_CORE_CONFIG_H__
#define __JUMANPP_CORE_CONFIG_H__

#include <stddef.h>

namespace jumanpp {
namespace core {

constexpr size_t JPP_MAX_DIC_FIELDS = 16;

#define JPP_PREFETCH_FEATURE_WEIGHTS

static constexpr char JPP_DEFAULT_CONFIG_DIR[]{""};

}  // namespace core
}  // namespace jumanpp

#endif  //__JUMANPP_CORE_CONFIG_H__
