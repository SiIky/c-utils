static inline int qc_map_int_cmp (const int k1, const int k2)
{
    return (k1 < k2) ?
        -1 :
        (k1 > k2) ?
        1 :
        0;
}

static unsigned int qc_map_int_hash (const int k)
{
    return (unsigned int) k;
}

#include <common.h>

#define QC_MKID_MOD_TEST(FUNC, TEST) \
    QC_MKID(map, FUNC, TEST, test)

#define QC_MKID_MOD_PROP(FUNC, TEST) \
    QC_MKID(map, FUNC, TEST, prop)

#define QC_MKID_MOD_ALL(FUNC) \
    QC_MKID_ALL(map, FUNC)

#define MAP_CFG_IMPLEMENTATION
#define MAP_CFG_HASH_FUNC qc_map_int_hash
#define MAP_CFG_KEY_CMP qc_map_int_cmp
#define MAP_CFG_KEY_DATA_TYPE int
#define MAP_CFG_VALUE_DATA_TYPE int
#include <utils/map.h>

static bool     qc_map_cardinal_eq (const struct map * map, const struct map * other);
static bool     qc_map_insert      (struct map * self, int key, int value, unsigned hash, unsigned tblidx);
static bool     qc_map_lsearch     (const struct map * self, int key, unsigned hash, unsigned tblidx, unsigned * _i);
static unsigned qc_map_cardinal    (const struct map * map);
static void     qc_map_free        (void * instance, void * env);

static enum theft_alloc_res qc_map_alloc (struct theft * t, void * env, void ** output)
{
    UNUSED(env);

    struct map * map = malloc(sizeof(struct map));
    if (map == NULL)
        return THEFT_ALLOC_SKIP;

    *map = (struct map) {0};

    { /* Allocate & initialize tables */
        map->size = (unsigned) theft_random_choice(t, 125) + 3;
        map->table = calloc(map->size, sizeof(*map->table));
        if (map->table == NULL)
            return free(map), THEFT_ALLOC_SKIP;
    }

    { /* Fill map */
        unsigned nelems = (unsigned) theft_random_bits(t, 10);
        unsigned _i = 0;
        for (unsigned i = 0; i < nelems; i++) {
            int k = 0;
            unsigned h = 0;
            unsigned tblidx = 0;

            do { /* unique keys */
                k = (int) theft_random_bits(t, 32);
                h = qc_map_int_hash(k);
                tblidx = h % map->size;
            } while (qc_map_lsearch(map, k, h, tblidx, &_i));

            int v = 0;
            if (!qc_map_insert(map, k, v, h, tblidx))
                return qc_map_free(map, NULL), THEFT_ALLOC_SKIP;
        }
    }

    { /* TODO: random valid iterator */
    }

    *output = map;
    return THEFT_ALLOC_OK;
}

static void qc_map_free (void * instance, void * env)
{
    UNUSED(env);
    struct map * map = instance;
    for (unsigned i = 0; i < map->size; i++)
        ifnotnull(map->table[i].entries, free);
    free(map->table);
    free(map);
}

static void qc_map_print (FILE * f, const void * instance, void * env)
{
    UNUSED(f);
    UNUSED(env);

    const struct map * map = instance;
    size_t size = map->size;
    fprintf(f, "{ ");
    size_t t = 0;
    for (size_t i = 0; i < size; i++) {
        size_t len = map->table[i].length;
        for (size_t j = 0; j < len; t++, j++)
            fprintf(f, "(%d, %d),%c",
                    map->table[i].entries[j].key,
                    map->table[i].entries[j].value,
                    (((t & 0x3) == 0) ? '\n' : ' '));
    }
    fprintf(f, "}\n");
}

const struct theft_type_info qc_map_info = {
    .alloc = qc_map_alloc,
    .free  = qc_map_free,
    .print = qc_map_print,
};

static inline int qc_map_entry_cmp (unsigned int ha, int ka, unsigned int hb, int kb)
{
    return (ha < hb) ?
        -1:
        (ha > hb) ?
        1:
        qc_map_int_cmp(ka, kb);
}

static bool qc_map_lsearch (const struct map * self, int key, unsigned hash, unsigned tblidx, unsigned * _i)
{
    unsigned int i = 0;
    unsigned int size = self->table[tblidx].length;
    int cmp = 1;

#define _f(I, FIELD) (self->table[tblidx].entries[I].FIELD)
#define _(I) \
    ((cmp = qc_map_entry_cmp(hash, key, _f(I, hash), _f(I, key))) < 0)
    /* NOTE: ASC OR DESC ? */
    for (i = 0; i < size && _(i); i++);
#undef _
#undef _f

    *_i = i;
    return cmp == 0;
}

/*
 * Exact copy of _MAP_INSERT_SORTED(), except for using
 * qc_map_lsearch() (defined above) instead of _MAP_SEARCH()
 */
static bool qc_map_insert (struct map * self, int key, int value, unsigned hash, unsigned tblidx)
{
    unsigned i = 0;
    bool exists = qc_map_lsearch(self, key, hash, tblidx, &i);

    if (!exists) {
        if (!map__increase_capacity(self, tblidx))
            return false;

        /* move entries to the right */
        unsigned len = self->table[tblidx].length;
        if (i < len)
            memmove(&self->table[tblidx].entries[i + 1],
                    &self->table[tblidx].entries[i],
                    sizeof(*self->table[tblidx].entries) * (len - i));

        self->cardinal++;

        self->lc.valid = true;
        self->lc.hash = hash;
        self->lc.idx = i;
    }

    self->table[tblidx].entries[i].hash = hash;
    self->table[tblidx].entries[i].key = key;
    self->table[tblidx].entries[i].value = value;
    self->table[tblidx].length++;

    return true;
}

static bool qc_map_cardinal_eq (const struct map * map, const struct map * other)
{
    return map->cardinal == other->cardinal;
}

static unsigned qc_map_cardinal (const struct map * map)
{
    unsigned ret = 0;
    for (unsigned tblidx = 0; tblidx < map->size; tblidx++)
        ret += map->table[tblidx].length;
    return ret;
}

static bool qc_map_clone (const struct map * map, struct map * other)
{
    *other = *map;
    other->table = calloc(map->size, sizeof(*other->table));
    if (other->table == NULL)
        return false;

    const unsigned size = map->size;
    const size_t ent_size = sizeof(*other->table[0].entries);
    for (unsigned tblidx = 0; tblidx < size; tblidx++) {
        unsigned length = map->table[tblidx].length;
        size_t nbytes = length * ent_size;
        other->table[tblidx].entries = calloc(map->table[tblidx].length, ent_size);
        assert(other->table[tblidx].entries != NULL);
        other->table[tblidx].capacity = map->table[tblidx].capacity;
        other->table[tblidx].length = length;
        memcpy(other->table[tblidx].entries, map->table[tblidx].entries, nbytes);
    }

    return true;
}

static bool qc_map_content_eq (const struct map * map, const struct map * other)
{
    bool ret = map->size == other->size;
    unsigned size = map->size;
    for (unsigned tblidx = 0; tblidx < size && ret; tblidx++) {
        unsigned length = map->table[tblidx].length;
        size_t nbytes = length * sizeof(*map->table[tblidx].entries);
        ret = other->table[tblidx].length == length
            && (memcmp(map->table[tblidx].entries, other->table[tblidx].entries, nbytes) == 0);
    }
    return ret;
}

static bool qc_map_iter_eq (const struct map * map, const struct map * other)
{
    return memcmp(&map->iter, &other->iter, sizeof(map->iter)) == 0;
}

static int qc_map_get (const struct map * map, int k)
{
    unsigned h = qc_map_int_hash(k);
    unsigned tblidx = h % map->size;
    unsigned i;
    bool exists = qc_map_lsearch(map, k, h, tblidx, &i);
    assert(exists);
    return map->table[tblidx].entries[i].value;
}

static bool qc_map_contains (const struct map * map, int k)
{
    unsigned h = qc_map_int_hash(k);
    unsigned tblidx = h % map->size;
    unsigned i;
    return qc_map_lsearch(map, k, h, tblidx, &i);
}

static int qc_map_random_in (struct theft * t, const struct map * map)
{
    int ret = 0;

    /*
     * Create a random "index" and look for the corresponding key.
     * For obvious reasons, the map cant be empty!
     */
    unsigned k = (unsigned) theft_random_choice(t, qc_map_cardinal(map)) + 1;
    for (unsigned tblidx = 0; k > 0 && tblidx < map->size; tblidx++)
        for (unsigned i = 0; k > 0 && i < map->table[tblidx].length; i++, k--)
            ret = map->table[tblidx].entries[i].key;

    return ret;
}

static int qc_map_random_not_in (const struct map * map, int k)
{
    while (qc_map_contains(map, k))
        k++;
    return k;
}
