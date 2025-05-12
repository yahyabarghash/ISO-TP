/*============================================================================*/

/*============================================================================*/
#include <inttypes.h>

/**
 * @file
 * Prototypes and structures for the ring buffer module.
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

/**
 * The size of a ring buffer.
 * Due to the design only <tt> RB_SIZE-1 </tt> items
 * can be contained in the buffer.
 * The buffer size must be a power of two.
 */
//#define RB_BLOCKS       8
//#define RB_CLOCK_SIZE   128
//#define RB_SIZE         RB_BLOCKS * RB_CLOCK_SIZE
//#define RB_SIZE2 4096

//#if (RB_SIZE & (RB_SIZE - 1)) != 0
//#error "RB_SIZE must be a power of two"
//#endif

/**
 * The type which is used to hold the size and the indicies of the buffer.
 * Must be able to fit \c RB_SIZE .
 */
typedef uint16_t ringb_size_t;

/**
 * The type which is used to hold atomic size of the ring buffer.
 * Usually it's int8_t
 */
typedef uint8_t ringb_atom_t;

///**
// * Used as a modulo operator as <tt> a % b = (a & (b - 1)) </tt>
// * where \c a is a positive index in the buffer and b is the (power of two)
// * size of the buffer.
// */
//#define RB_MASK (RB_SIZE-1)
static uint8_t rbmask = 0;

/**
 * Structure which holds a ring buffer. The buffer contains a buffer array
 * as well as metadata for the ring buffer.
 */

typedef struct s_ringb
{
    /** Buffer memory. */
    ringb_atom_t* p_buf;
    /**
     * Used as a modulo operator as <tt> a % b = (a & (b - 1)) </tt>
     * where \c a is a positive index in the buffer and b is the (power of two)
     * size of the buffer.
     */
    uint16_t       mask;
    /** Index of tail. */
    ringb_size_t  sz_tail;
    /** Index of head. */
    ringb_size_t  sz_head;
} s_ringb_t;

//typedef struct l_ringb
//{
//    /** Buffer memory. */
//    ringb_atom_t a_buf[RB_SIZE2];
//    /** Index of tail. */
//    ringb_size_t sz_tail;
//    /** Index of head. */
//    ringb_size_t sz_head;
//} l_ringb_t;
/**
 * Initializes the ring buffer pointed to by <em>buffer</em>.
 * This function can also be used to empty/reset the buffer.
 * @param ps_buf The ring buffer to initialize.
 * @param p_buf  pointer to the actual data buffer
 * @param size   size of the allocated buffer
 */
void ringb_init( s_ringb_t *ps_rb, ringb_atom_t* p_buf, uint16_t size);

/**
 * Adds a atomic to a ring buffer.
 * @param ps_buf    The buffer in which the data should be placed.
 * @param data      The atomic to place.
 */
void ringb_pusha( s_ringb_t *ps_rb, ringb_atom_t data );

/**
 * Adds an array of atomics to a ring buffer.
 * @param ps_buf    The buffer in which the data should be placed.
 * @param p_data    A pointer to the array of atomics to place in the queue.
 * @param sz_len    The size of the array.
 */
void ringb_push( s_ringb_t *ps_rb, const ringb_atom_t* p_data,
        ringb_size_t sz_len );

///**
// * Adds an array of atomics to a ring buffer with size of 4096.
// * @param ps_buf    The buffer in which the data should be placed.
// * @param p_data    A pointer to the array of atomics to place in the queue.
// * @param sz_len    The size of the array.
// */
//void ringb_push_b( l_ringb_t *ps_rb, const ringb_atom_t* p_data,
//        ringb_size_t sz_len );


/**
 * Returns the oldest atomic in a ring buffer.
 * @param ps_buf The buffer from which the data should be returned.
 * @param p_data A pointer to the location at which the data should be placed.
 * @return 1 if data was returned; 0 otherwise.
 */
uint8_t ringb_pulla( s_ringb_t *ps_rb, ringb_atom_t* p_data );

/**
 * Returns the <em>len</em> oldest atomics from a ring buffer.
 * @param ps_buf The buffer from which the data should be returned.
 * @param p_data A pointer to the array at which the data should be placed.
 * @param sz_len The maximum number of atomics to return.
 * @return The number of atomics returned.
 */
ringb_size_t ringb_pull( s_ringb_t *ps_rb, ringb_atom_t* p_data,
                         ringb_size_t sz_len );

///**
// * Returns the <em>len</em> oldest atomics from a ring buffer.
// * @param ps_buf The buffer from which the data should be returned.
// * @param p_data A pointer to the array at which the data should be placed.
// * @param sz_len The maximum number of atomics to return.
// * @return The number of atomics returned.
// */
//ringb_size_t ringb_pull_b( l_ringb_t *ps_rb, ringb_atom_t* p_data,
//                         ringb_size_t sz_len );


/**
 * Peeks a ring buffer, i.e. returns an element without removing it.
 * @param ps_buf The buffer from which the data should be returned.
 * @param p_data A pointer to the location at which the data should be placed.
 * @param index The index to peek.
 * @return 1 if data was returned; 0 otherwise.
 */
uint8_t ringb_peek( s_ringb_t *ps_rb, ringb_atom_t *data, ringb_size_t index );

/**
 * Returns whether a ring buffer is empty.
 * @param ps_buf The buffer for which it should be returned whether it is empty.
 * @return 1 if empty; 0 otherwise.
 */
inline uint8_t ringb_empty( s_ringb_t* ps_rb )
{
    return ( ps_rb->sz_head == ps_rb->sz_tail );
}

/**
 * Returns the number of items in a ring buffer.
 * @param ps_buf The buffer for which the number of items should be returned.
 * @return The number of items in the ring buffer.
 */
inline uint8_t ringb_full( s_ringb_t* ps_rb )
{
    return ( ( ps_rb->sz_head - ps_rb->sz_tail ) & ps_rb->mask ) == ps_rb->mask;
}

/**
 * Returns whether a ring buffer is full.
 * @param ps_buf The buffer for which it should be returned whether it is full.
 * @return 1 if full; 0 otherwise.
 */
inline uint8_t ringb_items( s_ringb_t* ps_rb )
{
    return ( ( ps_rb->sz_head - ps_rb->sz_tail ) & ps_rb->mask );
}

#endif /* RINGBUFFER_H */
