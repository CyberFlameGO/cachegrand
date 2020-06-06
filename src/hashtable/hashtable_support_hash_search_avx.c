#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <immintrin.h>

#include "hashtable.h"
#include "hashtable_support_hash_search.h"

__attribute__((noinline)) hashtable_bucket_chain_ring_index_t hashtable_support_hash_search_avx(
        hashtable_bucket_hash_half_t hash,
        hashtable_bucket_hash_half_atomic_t* hashes,
        uint32_t skip_indexes_mask) {
    uint32_t compacted_result_mask = 0;
    uint32_t skip_indexes_mask_inv = ~skip_indexes_mask;
    __m256 cmp_vector = _mm256_castsi256_ps(_mm256_set1_epi32(hash));

    // The second load, load from the 6th uint32 to the 14th uint32, _mm256_loadu_si256 always loads 8 x uint32
    for(uint8_t base_index = 0; base_index < 12; base_index += 6) {
        __m256 ring_vector = _mm256_castpd_ps(_mm256_loadu_pd((double*)(hashes + base_index)));
        __m256 result_mask_vector = _mm256_cmp_ps(ring_vector, cmp_vector, _CMP_EQ_OQ);

        // Uses _mm256_movemask_ps to reduce the bandwidth
        compacted_result_mask |= (uint32_t)_mm256_movemask_ps(result_mask_vector) << (base_index);
    }

    return _tzcnt_u32(compacted_result_mask & skip_indexes_mask_inv);
}
