#ifndef _VOS_BITFIELD_H_
#define _VOS_BITFIELD_H_

// REF:
// 1. linux-kernel/include/linux/bitfield.h
// 2. linux-kernel/include/linux/bits.h

/*
 * Bit macros
 *
 * BIT(0)      = 0x0000000000000001
 * BIT(16)     = 0x0000000000010000
 * BIT(31)     = 0x0000000080000000
 * BIT_ULL(32) = 0x0000000100000000
 * BIT_ULL(63) = 0x8000000000000000
 * GENMASK(7, 0)       = 0x00000000000000FF
 * GENMASK(15, 8)      = 0x000000000000FF00
 * GENMASK(23, 16)     = 0x0000000000FF0000
 * GENMASK(31, 24)     = 0x00000000FF000000
 * GENMASK_ULL(39, 32) = 0x000000FF00000000
 * GENMASK_ULL(47, 40) = 0x0000FF0000000000
 * GENMASK_ULL(55, 48) = 0x00FF000000000000
 * GENMASK_ULL(63, 56) = 0xFF00000000000000
 */

/*
 * Bitfield access macros
 *
 * FIELD_{GET,PREP} macros take as first parameter shifted mask
 * from which they extract the base mask and shift amount.
 * Mask must be a compilation time constant.
 *
 * Example:
 *
 *  #define REG_FIELD_A  GENMASK(6, 0)
 *  #define REG_FIELD_B  BIT(7)
 *  #define REG_FIELD_C  GENMASK(15, 8)
 *  #define REG_FIELD_D  GENMASK(31, 16)
 *
 * Get:
 *  a = FIELD_GET(REG_FIELD_A, reg);
 *  b = FIELD_GET(REG_FIELD_B, reg);
 *
 * Prepare:
 *  reg = FIELD_PREP(REG_FIELD_A, 1) |
 *	  FIELD_PREP(REG_FIELD_B, 0) |
 *	  FIELD_PREP(REG_FIELD_C, c) |
 *	  FIELD_PREP(REG_FIELD_D, 0x40);
 *
 * Set:
 *  //reg = 0x12345678
 *  FIELD_SET(GENMASK(31, 24), 0xAA, reg);
 *  //reg = 0xAA345678
 *
 * Clear:
 *  // reg = 0x12345678
 *  FIELD_CLEAR(GENMASK(7, 0), reg);
 *  // reg = 0x12345600
 *
 * Modify:
 *  e.g. Set REG_FIELD_C of reg to 0xC
 *
 *  Method 1:
 *  FIELD_SET(REG_FIELD_C, 0xC, reg);
 *
 *  Method 2:
 *  FIELD_CLEAR(REG_FIELD_C, reg);
 *  reg |= FIELD_PREP(REG_FIELD_C, 0xC);
 */
#ifndef BIT
#define BIT(nr)             (1UL << (nr))
#endif

#ifndef BIT_ULL
#define BIT_ULL(nr)			(1ULL << (nr))
#endif

#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE       8
#endif

#ifndef BITS_PER_LONG_LONG
#define BITS_PER_LONG_LONG  64
#endif

#ifndef BITS_PER_LONG
#if defined(__LP64__) || defined(_LP64)
#define BITS_PER_LONG       64
#else
#define BITS_PER_LONG       32
#endif
#endif

/*
 * Create a contiguous bitmask starting at bit position @l and ending at
 * position @h. For example
 * GENMASK_ULL(39, 21) gives us the 64bit vector 0x000000ffffe00000.
 */
#ifndef GENMASK
#define GENMASK(h, l) \
	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))
#endif

#ifndef GENMASK_ULL
#define GENMASK_ULL(h, l) \
	(((~0ULL) - (1ULL << (l)) + 1) & \
	 (~0ULL >> (BITS_PER_LONG_LONG - 1 - (h))))
#endif

#define __vos_compile_error(message) __attribute__((error(message)))
#define __vos_compile_assert(condition, msg, prefix, suffix)		\
	do {								\
		extern void prefix ## suffix(void) __vos_compile_error(msg); \
		if (!(condition))					\
			prefix ## suffix();				\
	} while (0)
#define vos_compile_assert(condition, msg) \
	__vos_compile_assert(condition, msg, __compiletime_assert_, __COUNTER__)

#define VOS_BUILD_BUG_ON_MSG(cond, msg) vos_compile_assert(!(cond), msg)

#define __VOS_BUILD_BUG_ON_NOT_POWER_OF_2(n)	\
	VOS_BUILD_BUG_ON_MSG(((n) & ((n) - 1)) != 0, "not power of 2")

#define __vos_bf_shf(x) (__builtin_ffsll(x) - 1)

#define __VOS_BF_FIELD_CHECK(_mask, _reg, _val, _pfx) \
	({ \
		VOS_BUILD_BUG_ON_MSG(!__builtin_constant_p(_mask), \
				 _pfx "mask is not constant"); \
		VOS_BUILD_BUG_ON_MSG((_mask) == 0, _pfx "mask is zero"); \
		VOS_BUILD_BUG_ON_MSG(__builtin_constant_p(_val) ? \
				 ~((_mask) >> __vos_bf_shf(_mask)) & (_val) : 0, \
				 _pfx "value too large for the field"); \
		VOS_BUILD_BUG_ON_MSG((_mask) > (typeof(_reg))~0ull, \
				 _pfx "type of reg too small for mask"); \
		__VOS_BUILD_BUG_ON_NOT_POWER_OF_2((_mask) + \
					      (1ULL << __vos_bf_shf(_mask))); \
	})

/**
 * FIELD_FIT() - check if value fits in the field
 * @_mask: shifted mask defining the field's length and position
 * @_val:  value to test against the field
 *
 * Return: true if @_val can fit inside @_mask, false if @_val is too big.
 */
#ifndef FIELD_FIT
#define FIELD_FIT(_mask, _val) \
	({ \
		__VOS_BF_FIELD_CHECK(_mask, 0ULL, 0ULL, "FIELD_FIT: "); \
		!((((typeof(_mask))_val) << __vos_bf_shf(_mask)) & ~(_mask)); \
	})
#endif

/**
 * FIELD_PREP() - prepare a bitfield element
 * @_mask: shifted mask defining the field's length and position
 * @_val:  value to put in the field
 *
 * FIELD_PREP() masks and shifts up the value.  The result should
 * be combined with other fields of the bitfield using logical OR.
 */
#ifndef FIELD_PREP
#define FIELD_PREP(_mask, _val) \
	({ \
		__VOS_BF_FIELD_CHECK(_mask, 0ULL, _val, "FIELD_PREP: "); \
		((typeof(_mask))(_val) << __vos_bf_shf(_mask)) & (_mask); \
	})
#endif

/**
 * FIELD_GET() - extract a bitfield element
 * @_mask: shifted mask defining the field's length and position
 * @_reg:  value of entire bitfield
 *
 * FIELD_GET() extracts the field specified by @_mask from the
 * bitfield passed in as @_reg by masking and shifting it down.
 */
#ifndef FIELD_GET
#define FIELD_GET(_mask, _reg) \
	({ \
		__VOS_BF_FIELD_CHECK(_mask, _reg, 0U, "FIELD_GET: "); \
		(typeof(_mask))(((_reg) & (_mask)) >> __vos_bf_shf(_mask)); \
	})
#endif

/**
 * FIELD_CLEAR() - clear a bitfield element to zero
 * @_mask: shifted mask defining the field's length and position
 * @_reg:  value of entire bitfield
 *
 * FIELD_CLEAR() clear the field of @_reg specified by @_mask
 */
#ifndef FIELD_CLEAR
#define FIELD_CLEAR(_mask, _reg) \
	({ \
		__VOS_BF_FIELD_CHECK(_mask, _reg, 0U, "FIELD_CLEAR: "); \
		((typeof(_mask))((_reg) &= ~(_mask))); \
	})
#endif

/**
 * FIELD_SET() - replace a bitfield element
 * @_mask: shifted mask defining the field's length and position
 * @_reg:  value of entire bitfield
 * @_val:  value to put in the field
 *
 * FIELD_SET() set @_val to the field @_mask of @_reg.
 * The original field is replaced by the new value.
 */
#ifndef FIELD_SET
#define FIELD_SET(_mask, _val, _reg) \
	({ \
		__VOS_BF_FIELD_CHECK(_mask, _reg, _val, "FIELD_SET: "); \
		FIELD_CLEAR(_mask, _reg); \
		((typeof(_mask))((_reg) |= FIELD_PREP(_mask, _val))); \
	})
#endif

#endif /* _VOS_BITFIELD_H_ */

