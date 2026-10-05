/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /G/wwlib/lcw.cpp                                            $* 
 *                                                                                             * 
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 10/04/99 10:25a                                             $*
 *                                                                                             * 
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   LCW_Comp -- Performes LCW compression on a block of data.                                 * 
 *   LCW_Uncomp -- Decompress an LCW encoded data block.                                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"LCW.H"

/***************************************************************************
 * LCW_Uncomp -- Decompress an LCW encoded data block.                     *
 *                                                                         *
 * Uncompress data to the following codes in the format b = byte, w = word *
 * n = byte code pulled from compressed data.                              *
 *                                                                         *
 *   Command code, n        |Description                                   *
 * ------------------------------------------------------------------------*
 * n=0xxxyyyy,yyyyyyyy      |short copy back y bytes and run x+3 from dest *
 * n=10xxxxxx,n1,n2,...,nx+1|med length copy the next x+1 bytes from source*
 * n=11xxxxxx,w1            |med copy from dest x+3 bytes from offset w1   *
 * n=11111111,w1,w2         |long copy from dest w1 bytes from offset w2   *
 * n=11111110,w1,b1         |long run of byte b1 for w1 bytes              *
 * n=10000000               |end of data reached                           *
 *                                                                         *
 *                                                                         *
 * INPUT:                                                                  *
 *      void * source ptr                                                  *
 *      void * destination ptr                                             *
 *      unsigned long length of uncompressed data                          *
 *                                                                         *
 *                                                                         *
 * OUTPUT:                                                                 *
 *     unsigned long # of destination bytes written                        *
 *                                                                         *
 * WARNINGS:                                                               *
 *     3rd argument is dummy. It exists to provide cross-platform          *
 *      compatibility. Note therefore that this implementation does not    *
 *      check for corrupt source data by testing the uncompressed length.  *
 *                                                                         *
 * HISTORY:                                                                *
 *    03/20/1995 IML : Created.                                            *
 *=========================================================================*/
int LCW_Uncomp(void const * source, void * dest, unsigned long )
{
	unsigned char * source_ptr, * dest_ptr, * copy_ptr;
	unsigned char op_code, data;
	unsigned count;
	unsigned * word_dest_ptr;
	unsigned word_data;

	/* Copy the source and destination ptrs. */
	source_ptr = (unsigned char*) source;
	dest_ptr   = (unsigned char*) dest;

	for (;;) {

		/* Read in the operation code. */
		op_code = *source_ptr++;

		if (!(op_code & 0x80)) {

			/* Do a short copy from destination. */
			count = (op_code >> 4) + 3;
			copy_ptr = dest_ptr - ((unsigned) *source_ptr++ + (((unsigned) op_code & 0x0f) << 8));

			while (count--) *dest_ptr++ = *copy_ptr++;

		} else {

			if (!(op_code & 0x40)) {

				if (op_code == 0x80) {

					/* Return # of destination bytes written. */
					return ((unsigned long) (dest_ptr - (unsigned char*) dest));

				} else {

					/* Do a medium copy from source. */
					count = op_code & 0x3f;

					while (count--) *dest_ptr++ = *source_ptr++;
				}

			} else {

				if (op_code == 0xfe) {

					/* Do a long run. */
					count = *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
					word_data = data = *(source_ptr + 2);
					word_data  = (word_data << 24) + (word_data << 16) + (word_data << 8) + word_data;
					source_ptr += 3;

					copy_ptr = dest_ptr + 4 - ((size_t) dest_ptr & 0x3);
					count -= (copy_ptr - dest_ptr);
					while (dest_ptr < copy_ptr) *dest_ptr++ = data;

					word_dest_ptr = (unsigned*) dest_ptr;

					dest_ptr += (count & 0xfffffffc);

					while (word_dest_ptr < (unsigned*) dest_ptr) {
						*word_dest_ptr		= word_data;
						*(word_dest_ptr + 1) = word_data;
						word_dest_ptr += 2;
					}

					copy_ptr = dest_ptr + (count & 0x3);
					while (dest_ptr < copy_ptr) *dest_ptr++ = data;

				} else {

					if (op_code == 0xff) {

						/* Do a long copy from destination. */
						count = *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
						copy_ptr = (unsigned char*) dest + *(source_ptr + 2) + ((unsigned) *(source_ptr + 3) << 8);
						source_ptr += 4;

						while (count--) *dest_ptr++ = *copy_ptr++;

					} else {

						/* Do a medium copy from destination. */
						count = (op_code & 0x3f) + 3;
						copy_ptr = (unsigned char*) dest + *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
						source_ptr += 2;

						while (count--) *dest_ptr++ = *copy_ptr++;
					}
				}
			}
		}
	}
}


#if defined(_MSC_VER)


/*********************************************************************************************** 
 * LCW_Comp -- Performes LCW compression on a block of data.                                   * 
 *                                                                                             * 
 *    This routine will compress a block of data using the LCW compression method. LCW has     * 
 *    the primary characteristic of very fast uncompression at the expense of very slow        * 
 *    compression times.                                                                       * 
 *                                                                                             * 
 * INPUT:   source   -- Pointer to the source data to compress.                                * 
 *                                                                                             * 
 *          dest     -- Pointer to the destination location to store the compressed data       * 
 *                      to.                                                                    * 
 *                                                                                             * 
 *          datasize -- The size (in bytes) of the source data to compress.                    * 
 *                                                                                             * 
 * OUTPUT:  Returns with the number of bytes of output data stored into the destination        * 
 *          buffer.                                                                            * 
 *                                                                                             * 
 * WARNINGS:   Be sure that the destination buffer is big enough. The maximum size required    * 
 *             for the destination buffer is (datasize + datasize/128).                        * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/20/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
/*ARGSUSED*/
int LCW_Comp(void const * source, void * dest, int datasize)
{
	int retval = 0;

	unsigned char * dptr = (unsigned char *)dest;
	unsigned char const * sptr = (unsigned char const *)source;
	unsigned char const * const end_of_data = sptr + datasize;
	unsigned char const * const a1stsrc = sptr;
	unsigned char * const a1stdest = dptr;

	int inlen = 1;
	unsigned char * lenoff = dptr;
	unsigned char * ndest = nullptr;
	int count = 0;
	unsigned char const * matchoff = nullptr;

	*dptr++ = 0x81;
	*dptr++ = *sptr++;

	for (;;) {
		ndest = dptr;
		unsigned char const * search_start = a1stsrc;
		count = 1;

		for (;;) {
			if (sptr + 64 < end_of_data && *sptr == *(sptr + 64)) {
				unsigned char const * run_end = sptr;
				while (run_end < end_of_data && *run_end == *sptr) {
					run_end++;
				}
				int run_length = (int)(run_end - sptr);

				if (run_length >= 65) {
					inlen = 0;
					sptr = run_end;
					dptr = ndest;

					*dptr++ = 0xFE;
					*dptr++ = (unsigned char)(run_length & 0xFF);
					*dptr++ = (unsigned char)((run_length >> 8) & 0xFF);
					*dptr++ = *sptr;

					ndest = dptr;
					continue;
				}
			}

			if (sptr <= search_start) {
				break;
			}

			unsigned char const * search_ptr = search_start;
			while (search_ptr < sptr && *search_ptr != *sptr) {
				search_ptr++;
			}
			if (search_ptr >= sptr) {
				break;
			}

			if (sptr + count - 1 < end_of_data && search_ptr + count - 1 < sptr) {
				if (*(sptr + count - 1) != *(search_ptr + count - 1)) {
					search_start = search_ptr + 1;
					continue;
				}
			}

			int match_length = 0;
			unsigned char const * tmp_s = sptr;
			unsigned char const * tmp_d = search_ptr;
			while (tmp_s < end_of_data && *tmp_s == *tmp_d) {
				match_length++;
				tmp_s++;
				tmp_d++;
			}

			if (match_length > count) {
				count = match_length;
				matchoff = search_ptr;
			}

			search_start = search_ptr + 1;
		}

		dptr = ndest;

		if (count > 2) {
			if (count <= 10) {
				int offset = (int)(sptr - matchoff);
				if (offset <= 0xFFF) {
					*dptr++ = (unsigned char)(((count - 3) << 4) | ((offset >> 8) & 0x0F));
					*dptr++ = (unsigned char)(offset & 0xFF);
					sptr += count;
					inlen = 0;
					continue;
				}
			}

			if (count <= 64) {
				*dptr++ = (unsigned char)(0xC0 | (count - 3));
				int offset = (int)(matchoff - a1stsrc);
				*dptr++ = (unsigned char)(offset & 0xFF);
				*dptr++ = (unsigned char)((offset >> 8) & 0xFF);
				sptr += count;
				inlen = 0;
				continue;
			}

			*dptr++ = 0xFF;
			*dptr++ = (unsigned char)(count & 0xFF);
			*dptr++ = (unsigned char)((count >> 8) & 0xFF);
			int offset = (int)(matchoff - a1stsrc);
			*dptr++ = (unsigned char)(offset & 0xFF);
			*dptr++ = (unsigned char)((offset >> 8) & 0xFF);
			sptr += count;
			inlen = 0;
			continue;
		}

		if (inlen == 0) {
			lenoff = dptr;
			*dptr++ = 0x80;
		}

		if (*lenoff == 0xBF) {
			lenoff = dptr;
			*dptr++ = 0x80;
		}

		(*lenoff)++;
		*dptr++ = *sptr++;
		inlen = 1;

		if (sptr >= end_of_data) {
			break;
		}
	}

	*dptr++ = 0x80;
	retval = (int)(dptr - a1stdest);

	return(retval);
}
#endif


