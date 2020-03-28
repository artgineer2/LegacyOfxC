/*
 * structs.h
 *
 *  Created on: Mar 11, 2020
 *      Author: mike
 */

#ifndef CUSTOMTYPES_H_
#define CUSTOMTYPES_H_


typedef struct LongBytes
{
	union
	{
		uint32_t long_var;
		struct
		{
			uint8_t const pad;
			uint8_t high_byte;
			uint8_t mid_byte;
			uint8_t low_byte;
		};
	};
} LongBytes_t;





#endif /* CUSTOMTYPES_H_ */
