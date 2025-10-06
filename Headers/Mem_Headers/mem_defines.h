#ifndef MEM_DEFINES_H
#define MEM_DEFINES_H

#define persist         static
#define internal        static

#define kb(x)           (x * 1024)
#define mb(x)           (kb(x) * 1024)
#define gb(x)           (mb(x) * 1024)

#define unused(x)       (void)(x)
#define packed          __attribute__((__packed__))

#define file            __FILE__
#define line            __LINE__

// MEM_ALLOC_H
#define MEM_SUCCESS     (result)0X00 // Operation succesful.
#define MEM_NOISSUE     (result)0X01 // No overwrite issues detected.
#define MEM_OUTOFBOUNDS (result)0X02 // Memory overwrite detected.
#define MEM_NOALLOC     (result)0X03 // Unable to allocate memory.
#define MEM_NOFREE      (result)0X04 // Unable to free memory.
#define MEM_NOOUTPUT    (result)0X05 // No output buffer provided.
#define MEM_NOINPUT     (result)0X06 // Input field missing.
#define MEM_UNUSED      (result)0X07 // No allocations made as of yet.
#define MEM_NOPREV      (result)0X08 // No previous allocation to iterate to.
#define MEM_NONEXT      (result)0X09 // No next allocation to iterate to.

// MEM_POOL_H
#define POOL_SUCCESS    (result)0X0A // memory manager success.
#define POOL_NOSPACE    (result)0X0B // No space available.
#define POOL_NOPOOL     (result)0X0C // No avaolable pools, call Pool_Build().
#define POOL_NOSIZE     (result)0X0D // No size provided.
#define POOL_NOALLOC    (result)0X0E // No allocator provided.
#define POOL_NOFREE     (result)0X0F // No free provided.
#define POOL_NOBUILD    (result)0X10 // Unable to allocate space.
#define POOL_NOACTIVE   (result)0X11 // Pool has been freed already.
#define POOL_NORESIZE   (result)0X12 // Pool is not resizeable.
#define POOL_NOOUTPUT   (result)0X13 // No output buffer provided.
#define POOL_INVPLHND   (result)0X14 // Invalid pool handle.
#define POOL_INVREHND   (result)0X15 // Invalid reservation handle.
#define POOL_RESFREE    (result)0X16 // Reservation has been released.
#define POOL_DBLFREE    (result)0X17 // Release has already been called on this reservation.
#define POOL_RESNOFIT   (result)0X18 // Reservatioin isn't large enough, data has been truncated.
#define POOL_INVOFFST   (result)0X19 // Invalid buffer offset on write attempt.
#define POOL_RESFAULT   (result)0x1A // Unable to obtain the reservation.
#define POOL_FAILURE    (result)0X1B // Memory manager critical failure.

#endif // MEM_DEFINES_H