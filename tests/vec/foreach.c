#include <stdbool.h>
#include <string.h>

#include <common.h>
#include <unused.h>

#include "vec.h"

static size_t what = 0;

static void qc_vec_foreach_do_nothing_func (const int x)
{
    UNUSED(x);
    what++;
}

static enum theft_trial_res qc_vec_foreach_len_prop (struct theft * t, void * arg1)
{
    UNUSED(t);
    struct vec * vec = arg1;

    size_t pre_len = vec->length;
    vec_foreach(vec, qc_vec_foreach_do_nothing_func);
    what = 0;
    size_t pos_len = vec->length;

    return QC_BOOL2TRIAL(pre_len == pos_len);
}

static enum theft_trial_res qc_vec_foreach_iter_prop (struct theft * t, void * arg1)
{
    UNUSED(t);

    struct vec * vec = arg1;

    size_t pre_idx = vec->idx;
    unsigned char pre_iterating = vec->iterating;
    unsigned char pre_reverse = vec->reverse;

    vec_foreach(vec, qc_vec_foreach_do_nothing_func);
    what = 0;

    size_t pos_idx = vec->idx;
    unsigned char pos_iterating = vec->iterating;
    unsigned char pos_reverse = vec->reverse;

    bool ret = true
        && pre_iterating == pos_iterating
        && pre_reverse   == pos_reverse
        && pre_idx       == pos_idx;

    return QC_BOOL2TRIAL(ret);
}

static enum theft_trial_res qc_vec_foreach_content_prop (struct theft * t, void * arg1)
{
    UNUSED(t);

    struct vec * vec = arg1;

    struct vec * dup = qc_vec_dup_contents(vec);
    if (dup == NULL)
        return THEFT_TRIAL_SKIP;

    size_t pre_len = vec->length;
    vec_foreach(vec, qc_vec_foreach_do_nothing_func);
    what = 0;

    bool res = pre_len == 0
        || memcmp(dup->ptr, vec->ptr, pre_len * sizeof(int)) == 0;

    qc_vec_dup_free(dup);

    return QC_BOOL2TRIAL(res);
}

static enum theft_trial_res qc_vec_foreach_side_effects_prop (struct theft * t, void * arg1)
{
    UNUSED(t);
    struct vec * vec = arg1;

    size_t pre_len = vec->length;
    vec_foreach(vec, qc_vec_foreach_do_nothing_func);
    bool res = what == pre_len;
    what = 0;

    return QC_BOOL2TRIAL(res);
}

#define QC_MKTEST_FOREACH(TEST)                 \
    QC_MKTEST(qc_vec_foreach_ ## TEST ## _test, \
            prop1,                              \
            qc_vec_foreach_ ## TEST ## _prop,   \
            &qc_vec_info);

QC_MKTEST_FOREACH(content);
QC_MKTEST_FOREACH(iter);
QC_MKTEST_FOREACH(len);
QC_MKTEST_FOREACH(side_effects);

QC_MKTEST_ALL(qc_vec_foreach_test_all,
        qc_vec_foreach_content_test,
        qc_vec_foreach_iter_test,
        qc_vec_foreach_len_test,
        qc_vec_foreach_side_effects_test,
        );