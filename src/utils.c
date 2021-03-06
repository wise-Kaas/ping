#include <endian.h>
#include "ping.h"
#include "lib.h"

u_int16_t	compute_checksum(u_int16_t *addr, int count)
{
	register u_int32_t sum;

	sum = 0;
	while (count > 1)
	{
		sum += *addr++;
		count -= 2;
	}
	if (count > 0)
		sum += *(unsigned char *)addr;
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return (~sum);
}

u_int16_t	ft_htons(u_int16_t x)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN

	u_int16_t new;

	new = (x & 0xFF) << 8;
	new += (x & 0xFF << 8) >> 8;
	return (new);
#elif __BYTE_ORDER == __BIG_ENDIAN

	return (x);
#endif
}

u_int16_t	ft_ntohs(u_int16_t x)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN

	u_int16_t new;

	new = (x & 0xFF) << 8;
	new += (x & 0xFF << 8) >> 8;
	return (new);
#elif __BYTE_ORDER == __BIG_ENDIAN

	return (x);
#endif
}

u_int64_t ft_htonll(u_int64_t x)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN

	u_int64_t new;

	new = (long) ft_htons(x & 0xFFFF) << (12 * 4);
	new += (long) ft_htons((x & 0xFFFF << (4 * 4)) >> (4 * 4)) << (8 * 4);
	new += (long) ft_htons((x & (long) 0xFFFF << (8 * 4)) >> (8 * 4)) << (4 * 4);
	new += (long) ft_htons((x & (long) 0xFFFF << (12 * 4)) >> (12 * 4));
	return (new);
#elif __BYTE_ORDER == __BIG_ENDIAN

	return (x);
#endif
}
