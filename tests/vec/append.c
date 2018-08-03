#include "vec.h"

static enum theft_trial_res qc_vec_append_len_prop (struct theft * t, void * arg1, void * arg2)
{
    UNUSED(t);

    struct vec * vec = arg1;
    struct vec * other = arg2;

    struct vec vec_dup = {0};
    if (!qc_vec_dup_contents(vec, &vec_dup))
        return THEFT_TRIAL_SKIP;

    struct vec other_dup = {0};

    if (!qc_vec_dup_contents(other, &other_dup)) {
        qc_vec_dup_free(&vec_dup);
        return THEFT_TRIAL_SKIP;
    }

    size_t pre_vec_len = vec_dup.length;
    size_t pre_other_len = other_dup.length;
    size_t pre_len = pre_vec_len + pre_other_len;

    bool succ = vec_append(&vec_dup, &other_dup);

    size_t pos_vec_len = vec_dup.length;
    size_t pos_other_len = other_dup.length;

    bool res = (succ) ?
        pre_len     == pos_vec_len && pos_other_len == 0:
        pre_vec_len == pos_vec_len && pre_other_len == pos_other_len;

    qc_vec_dup_free(&vec_dup);
    qc_vec_dup_free(&other_dup);

    return QC_BOOL2TRIAL(res);
}

static enum theft_trial_res qc_vec_append_content_prop (struct theft * t, void * arg1, void * arg2)
{
    UNUSED(t);

    struct vec * vec = arg1;
    struct vec * other = arg2;

    struct vec vec_dup = {0};
    if (!qc_vec_dup_contents(vec, &vec_dup))
        return THEFT_TRIAL_SKIP;

    struct vec other_dup = {0};
    if (!qc_vec_dup_contents(other, &other_dup)) {
        qc_vec_dup_free(&vec_dup);
        return THEFT_TRIAL_SKIP;
    }

    size_t pre_vec_len = vec->length;
    size_t pre_other_len = other->length;

    bool succ = vec_append(vec, other);

    bool ret = (succ) ?
        ((memcmp(vec->ptr, vec_dup.ptr, pre_vec_len * sizeof(int)) == 0)
         && (memcmp(vec->ptr + pre_vec_len, other_dup.ptr, pre_other_len * sizeof(int)) == 0)):
        ((memcmp(vec->ptr, vec_dup.ptr, pre_vec_len * sizeof(int)) == 0)
         && (memcmp(other->ptr, other_dup.ptr, pre_other_len * sizeof(int)) == 0));

    qc_vec_dup_free(&vec_dup);
    qc_vec_dup_free(&other_dup);

    return QC_BOOL2TRIAL(ret);
}

static enum theft_trial_res qc_vec_append_iter_prop (struct theft * t, void * arg1, void * arg2)
{
    UNUSED(t);

    struct vec * vec = arg1;
    struct vec * other = arg2;

    size_t pre_vec_idx = vec->idx;
    unsigned char pre_vec_iterating = vec->iterating;
    unsigned char pre_vec_reverse = vec->reverse;
    size_t pre_other_idx = other->idx;
    unsigned char pre_other_iterating = other->iterating;
    unsigned char pre_other_reverse = other->reverse;

    vec_append(vec, other);

    size_t pos_vec_idx = vec->idx;
    unsigned char pos_vec_iterating = vec->iterating;
    unsigned char pos_vec_reverse = vec->reverse;
    size_t pos_other_idx = other->idx;
    unsigned char pos_other_iterating = other->iterating;
    unsigned char pos_other_reverse = other->reverse;

    bool res = true
        && pre_vec_iterating   == pos_vec_iterating
        && pre_vec_reverse     == pos_vec_reverse
        && pre_vec_idx         == pos_vec_idx
        && pre_other_iterating == pos_other_iterating
        && pre_other_reverse   == pos_other_reverse
        && pre_other_idx       == pos_other_idx;

    return QC_BOOL2TRIAL(res);
}

#define QC_MKID_FUNC(TEST, TYPE) \
    QC_MKID_MOD(append, TEST, TYPE)

#define QC_MKID_PROP(TEST) \
    QC_MKID_FUNC(TEST, prop)

#define QC_MKID_TEST(TEST) \
    QC_MKID_FUNC(TEST, test)

#define QC_MKTEST_FUNC(TEST)      \
    QC_MKTEST(QC_MKID_TEST(TEST), \
            prop2,                \
            QC_MKID_PROP(TEST),   \
            &qc_vec_info,         \
            &qc_vec_info)

QC_MKTEST_FUNC(content);
QC_MKTEST_FUNC(iter);
QC_MKTEST_FUNC(len);

QC_MKTEST_ALL(qc_vec_append_test_all,
        QC_MKID_TEST(content),
        QC_MKID_TEST(content),
        QC_MKID_TEST(iter),
        QC_MKID_TEST(len),
        );
