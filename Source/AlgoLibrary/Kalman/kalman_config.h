/*
 * kalman_config.h
 *
 *  Created on: Oct 6, 2025
 *      Author: elektronika
 */

#ifndef KALMAN_KALMAN_CONFIG_H_
#define KALMAN_KALMAN_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define KALMAN_CONFIG_POLY_USE_LOOKUPTABLE 0 //If 1 is set use lookup table for poly. Othervise, use math calculation for poly
#define KALMAN_CONFIG_POLY_USE_INTERNAL_POW 1	//If 1 is set use interl pow, othervise use math.h pow

#ifdef __cplusplus
}
#endif


#endif /* KALMAN_KALMAN_CONFIG_H_ */
