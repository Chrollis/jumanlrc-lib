//
// jumanlrc_capi.h — C API for jumanlrc-lib (lyrics ruby annotation)
//

#ifndef JUMANLRC_CAPI_H
#define JUMANLRC_CAPI_H

#ifdef __cplusplus
extern "C"
{
#endif

    /* Opaque handle for Juman++ instance */
    typedef struct jumanlrc_handle jumanlrc_handle;

    jumanlrc_handle *jumanlrc_init(const char *model_path);
    jumanlrc_handle *jumanlrc_init_ex(const char *model_path, int beam_size,
                                      int global_beam, int right_beam,
                                      int right_check);

    /*
     * Analyze a sentence.
     * Returns the number of words on success, or -1 on failure.
     * On success, *surfaces and *readings are allocated and must be freed
     * with jumanpp_free_result().
     *
     * The returned arrays contain exactly `count` strings.
     */
    int jumanlrc_analyze(jumanlrc_handle *handle, const char *sentence,
                         char ***surfaces, char ***readings);
    void jumanlrc_free_result(char **surfaces, char **readings, int count);
    const char *jumanlrc_error(jumanlrc_handle *handle);
    void jumanlrc_destroy(jumanlrc_handle *handle);

#ifdef __cplusplus
}
#endif

#endif /* JUMANPP_CAPI_H */
