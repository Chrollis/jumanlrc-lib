//
// jumanlrc_capi.cc — C API implementation for jumanlrc-lib
//

#include <cstdlib>
#include <cstring>
#include <string>

#include "jumanlrc_capi.h"
#include "jumanlrc_lib.h"

struct jumanlrc_handle
{
  jumanlrc::lib::JumanppLib impl;
  std::string lastError;
};

jumanlrc_handle *jumanlrc_init(const char *model_path)
{
  return jumanlrc_init_ex(model_path, 5, 6, 5, 1);
}

jumanlrc_handle *jumanlrc_init_ex(const char *model_path, int beam_size,
                                  int global_beam, int right_beam,
                                  int right_check)
{
  if (!model_path || model_path[0] == '\0')
    return nullptr;

  auto *handle = new (std::nothrow) jumanlrc_handle;
  if (!handle)
    return nullptr;

  if (!handle->impl.loadModel(model_path, beam_size, global_beam,
                              right_beam, right_check))
  {
    handle->lastError = handle->impl.error();
    return handle;
  }
  return handle;
}

int jumanlrc_analyze(jumanlrc_handle *handle, const char *sentence,
                     char ***surfaces, char ***readings)
{
  if (!handle || !sentence || !surfaces || !readings)
    return -1;

  if (!handle->impl.analyze(sentence))
  {
    handle->lastError = handle->impl.error();
    return -1;
  }

  const auto &result = handle->impl.lastResult();
  int count = static_cast<int>(result.size());

  *surfaces = static_cast<char **>(std::malloc(count * sizeof(char *)));
  *readings = static_cast<char **>(std::malloc(count * sizeof(char *)));
  if (!*surfaces || !*readings)
  {
    std::free(*surfaces);
    std::free(*readings);
    *surfaces = nullptr;
    *readings = nullptr;
    handle->lastError = "Memory allocation failed";
    return -1;
  }

  for (int i = 0; i < count; ++i)
  {
    const auto &w = result[i];
    (*surfaces)[i] = static_cast<char *>(std::malloc(w.surface.size() + 1));
    (*readings)[i] = static_cast<char *>(std::malloc(w.reading.size() + 1));
    if (!(*surfaces)[i] || !(*readings)[i])
    {
      for (int j = 0; j <= i; ++j)
      {
        std::free((*surfaces)[j]);
        std::free((*readings)[j]);
      }
      std::free(*surfaces);
      std::free(*readings);
      *surfaces = nullptr;
      *readings = nullptr;
      handle->lastError = "Memory allocation failed";
      return -1;
    }
    std::memcpy((*surfaces)[i], w.surface.c_str(), w.surface.size() + 1);
    std::memcpy((*readings)[i], w.reading.c_str(), w.reading.size() + 1);
  }
  return count;
}

void jumanlrc_free_result(char **surfaces, char **readings, int count)
{
  if (surfaces)
  {
    for (int i = 0; i < count; ++i)
      std::free(surfaces[i]);
    std::free(surfaces);
  }
  if (readings)
  {
    for (int i = 0; i < count; ++i)
      std::free(readings[i]);
    std::free(readings);
  }
}

const char *jumanlrc_error(jumanlrc_handle *handle)
{
  if (!handle)
    return "Null handle";
  return handle->lastError.c_str();
}

void jumanlrc_destroy(jumanlrc_handle *handle) { delete handle; }
