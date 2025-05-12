/*============================================================================*/
/*
 
/*============================================================================*/

#include "ringbuffer.h"
/**
 * @file
 * Implementation of ring ac_buf functions.
 */

void ringb_init( s_ringb_t *ps_rb, ringb_atom_t* p_buf, uint16_t size)
{
    ps_rb->p_buf    = p_buf;
    ps_rb->mask     = size - 1;
    ps_rb->sz_tail  = 0;
    ps_rb->sz_head  = 0;
}

void ringb_pusha( s_ringb_t *ps_rb, ringb_atom_t data )
{
    /* Is ac_buf full? */
    if( ringb_full( ps_rb ) )
    {
        /* Is going to overwrite the oldest byte */
        /* Increase tail index */
        ps_rb->sz_tail = ( ( ps_rb->sz_tail + 1 ) & ps_rb->mask );
    }

    /* Place data in ac_buf */
    ps_rb->p_buf[ps_rb->sz_head] = data;
    ps_rb->sz_head = ( ( ps_rb->sz_head + 1 ) & ps_rb->mask );
}

void ringb_push( s_ringb_t *ps_rb, const ringb_atom_t* p_data,
        ringb_size_t size )
{
    /* Add bytes; one by one */
    ringb_size_t i;
    for( i = 0; i < size; i++ )
    {
        ringb_pusha( ps_rb, p_data[i] );
    }
}

//void ringb_push_b( l_ringb_t *ps_rb, const ringb_atom_t* p_data,
//        ringb_size_t size )
//{
//    /* Add bytes; one by one */
//    ringb_size_t i;
//    for( i = 0; i < size; i++ )
//    {
//        ringb_pusha( ps_rb, p_data[i] );
//    }
//}

uint8_t ringb_pulla( s_ringb_t *ps_rb, ringb_atom_t* p_data )
{
    if( ringb_empty( ps_rb ) )
    {
        /* No items */
        return 0;
    }

    *p_data = ps_rb->p_buf[ps_rb->sz_tail];
    ps_rb->sz_tail = ( ( ps_rb->sz_tail + 1 ) & ps_rb->mask );
    return 1;
}

ringb_size_t ringb_pull( s_ringb_t *ps_rb, ringb_atom_t* p_data,
                         ringb_size_t len )
{
    if( ringb_empty( ps_rb ) )
    {
        /* No items */
        return 0;
    }

    ringb_atom_t *data_ptr = p_data;
    ringb_size_t cnt = 0;
    while( cnt < len && ringb_pulla( ps_rb, data_ptr ))
    {
        cnt++;
        if(cnt==8)
            cnt=8;
        data_ptr++;
    }
    return cnt;
}


//ringb_size_t ringb_pull_b( l_ringb_t *ps_rb, ringb_atom_t* p_data,
//                         ringb_size_t len )
//{
//    if( ringb_empty( ps_rb ) )
//    {
//        /* No items */
//        return 0;
//    }
//
//    ringb_atom_t *data_ptr = p_data;
//    ringb_size_t cnt = 0;//has been modified to be 1 instead of 0
//    while( cnt < len && ringb_pulla( ps_rb, data_ptr ))
//    {
//        cnt++;
//        data_ptr++;
//    }
//    return cnt;
//}


uint8_t ringb_peek( s_ringb_t *ps_rb, ringb_atom_t *data,
                         ringb_size_t index )
{
    if( index >= ringb_items( ps_rb ) )
    {
        /* No items at index */
        return 0;
    }

    /* Add index to pointer */
    ringb_size_t data_index = ( ( ps_rb->sz_tail + index ) & ps_rb->mask );
    *data = ps_rb->p_buf[data_index];
    return 1;
}

extern inline uint8_t ringb_empty( s_ringb_t *ps_rb );
extern inline uint8_t ringb_full( s_ringb_t *ps_rb );
extern inline uint8_t ringb_items( s_ringb_t *ps_rb );
