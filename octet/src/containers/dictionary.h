////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// map strings to value_t.
//
// strings and values are owned by the dictionary.
//
template <class value_t, class allocator_t=allocator> class dictionary {
  struct entry_t { const char *key; unsigned hash; value_t value; };
  entry_t *entries;
  unsigned num_entries;
  unsigned max_entries;
  
  // hash is a compromise between fewer collisions and
  // faster hash calculation.
  unsigned calc_hash( const char *key ) {
    unsigned hash = 0;
    for (int i = 0; key[i]; ++i) {
      hash = ( hash << 5 ) ^ ( hash << 3 ) ^ (key[i] & 0xff);
    }
    return hash;
  }
  
  entry_t *find( const char *key, unsigned hash ) {
    unsigned mask = max_entries - 1;
    for (unsigned i = 0; i != max_entries; ++i) {
      entry_t *entry = &entries[ ( i + hash ) & mask ];
      if (!entry->key) {
        return entry;
      }
      if (entry->hash == hash && !strcmp(entry->key, key)) {
        return entry;
      }
    }
    return 0;
  }
  
  void expand() {
    entry_t *old_entries = entries;
    unsigned old_max_entries = max_entries;
    entries = (entry_t *)allocator_t::malloc(sizeof(entry_t) * max_entries*2);
    memset(entries, 0, sizeof(entry_t) * max_entries*2);
    max_entries *= 2;
    for (unsigned i = 0; i != old_max_entries; ++i) {
      entry_t *old_entry = &old_entries[i];
      if (old_entry->key) {
        entry_t *new_entry = find(old_entry->key, old_entry->hash);
        *new_entry = *old_entry;
      }
    }
    allocator_t::free(old_entries, sizeof(entry_t) * old_max_entries);
  }
public:
  dictionary() {
    num_entries = 0;
    max_entries = 4;
    entries = (entry_t*)allocator_t::malloc(sizeof(entry_t) * max_entries);
    memset(entries, 0, sizeof(entry_t) * max_entries);
  }
  
  value_t &operator[]( const char *key ) {
    unsigned hash = calc_hash( key );
    entry_t *entry = find( key, hash );
    if (!entry || !entry->key) {
      // reducing this ratio decreases hot search time at the
      // expense of size (cold search time).
      if (num_entries > max_entries * 3 / 4) {
        expand();
        entry = find(key, hash);
      }
      num_entries++;
      size_t bytes = strlen(key) + 1;
      entry->key = (char *)allocator_t::malloc(bytes);
      entry->hash = hash;
      memcpy((void*)entry->key, key, bytes);
    }
    return entry->value;
  }

  ~dictionary() {
    for (unsigned i = 0; i != max_entries; ++i) {
      entry_t *entry = &entries[i];
      if (entry->key ) {
        allocator_t::free((void*)entry->key, strlen(entry->key)+1);
      }
    }
    allocator_t::free(entries, sizeof(entry_t) * max_entries);
    entries = 0;
    num_entries = 0;
    max_entries = 0;
  }
};
