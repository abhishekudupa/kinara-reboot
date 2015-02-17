/*
 * fnv - Fowler/Noll/Vo- hash code
 *
 * @(#) $Revision: 5.4 $
 * @(#) $Id: fnv.h,v 5.4 2009/07/30 22:49:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/fnv/RCS/fnv.h,v $
 *
 ***
 *
 * Fowler/Noll/Vo- hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv/)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * for more details as well as other forms of the FNV hash.
 *
 ***
 *
 * NOTE: The FNV-0 historic hash is not recommended.  One should use
 *	 the FNV-1 hash instead.
 *
 * To use the 32 bit FNV-0 historic hash, pass FNV0_32_INIT as the
 * Fnv32_t hashval argument to fnv_32_buf() or fnv_32_str().
 *
 * To use the 64 bit FNV-0 historic hash, pass FNV0_64_INIT as the
 * Fnv64_t hashval argument to fnv_64_buf() or fnv_64_str().
 *
 * To use the recommended 32 bit FNV-1 hash, pass FNV1_32_INIT as the
 * Fnv32_t hashval argument to fnv_32_buf() or fnv_32_str().
 *
 * To use the recommended 64 bit FNV-1 hash, pass FNV1_64_INIT as the
 * Fnv64_t hashval argument to fnv_64_buf() or fnv_64_str().
 *
 * To use the recommended 32 bit FNV-1a hash, pass FNV1_32A_INIT as the
 * Fnv32_t hashval argument to fnv_32a_buf() or fnv_32a_str().
 *
 * To use the recommended 64 bit FNV-1a hash, pass FNV1A_64_INIT as the
 * Fnv64_t hashval argument to fnv_64a_buf() or fnv_64a_str().
 *
 ***
 *
 * Please do not copyright this code.  This code is in the public domain.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * By:
 *	chongo <Landon Curt Noll> /\oo/\
 *      http://www.isthe.com/chongo/
 *
 * Share and Enjoy!	:-)
 */

// audupa: adapted for use with kinara

#if !defined KINARA_KINARA_COMMON_HASHFUNCS_FNV_HPP_
#define KINARA_KINARA_COMMON_HASHFUNCS_FNV_HPP_

#include <sys/types.h>
#include <cstdint>

#include "../basetypes/KinaraBase.hpp"

namespace kinara {
namespace utils {
namespace fnv_hash_detail_ {
/*
 * 32 bit FNV-0 hash type
 */
typedef uint32_t Fnv32_t;

/*
 * 32 bit FNV-1 and FNV-1a non-zero initial basis
 *
 * The FNV-1 initial basis is the FNV-0 hash of the following 32 octets:
 *
 *              chongo <Landon Curt Noll> /\../\
 *
 * NOTE: The \'s above are not back-slashing escape characters.
 * They are literal ASCII  backslash 0x5c characters.
 *
 * NOTE: The FNV-1a initial basis is the same value as FNV-1 by definition.
 */
static constexpr Fnv32_t FNV1_32_INIT = ((Fnv32_t)0x811c9dc5);
static constexpr Fnv32_t FNV1_32A_INIT = FNV1_32_INIT;
static constexpr Fnv32_t FNV1_32_PRIME = ((Fnv32_t)0x01000193);

typedef uint64_t Fnv64_t;

/*
 * 64 bit FNV-1 non-zero initial basis
 *
 * The FNV-1 initial basis is the FNV-0 hash of the following 32 octets:
 *
 *              chongo <Landon Curt Noll> /\../\
 *
 * NOTE: The \'s above are not back-slashing escape characters.
 * They are literal ASCII  backslash 0x5c characters.
 *
 * NOTE: The FNV-1a initial basis is the same value as FNV-1 by definition.
 */
static constexpr Fnv64_t FNV1_64_INIT = ((Fnv64_t)0xcbf29ce484222325ULL);
static constexpr Fnv64_t FNV1_64A_INIT = FNV1_64_INIT;
static constexpr Fnv64_t FNV1_64_PRIME = ((Fnv64_t)0x100000001b3ULL);

extern Fnv32_t fnv_32_buf(const void* buf, size_t len, Fnv32_t hashval);
extern Fnv32_t fnv_32_str(const char* buf, Fnv32_t hashval);

extern Fnv32_t fnv_32a_buf(const void* buf, size_t len, Fnv32_t hashval);
extern Fnv32_t fnv_32a_str(const char* buf, Fnv32_t hashval);

extern Fnv64_t fnv_64_buf(const void* buf, size_t len, Fnv64_t hashval);
extern Fnv64_t fnv_64_str(const char* buf, Fnv64_t hashval);

extern Fnv64_t fnv_64a_buf(const void* buf, size_t len, Fnv64_t hashval);
extern Fnv64_t fnv_64a_str(const char* buf, Fnv64_t hashval);

} /* end namespace fnv_hash_detail_ */

static inline u32 fnv_hash_32(const void* data, u64 length)
{
    return fnv_hash_detail_::fnv_32_buf(data, length, fnv_hash_detail_::FNV1_32_INIT);
}

static inline u32 fnv_hash_32a(const void* data, u64 length)
{
    return fnv_hash_detail_::fnv_32a_buf(data, length, fnv_hash_detail_::FNV1_32A_INIT);
}

static inline u64 fnv_hash_64(const void* data, u64 length)
{
    return fnv_hash_detail_::fnv_64_buf(data, length, fnv_hash_detail_::FNV1_64_INIT);
}

static inline u64 fnv_hash_64a(const void* data, u64 length)
{
    return fnv_hash_detail_::fnv_64a_buf(data, length, fnv_hash_detail_::FNV1_64A_INIT);
}

} /* end namespace utils */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_HASHFUNCS_FNV_HPP_ */
