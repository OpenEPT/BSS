

#ifdef __cplusplus
extern "C" {
#endif

/* Auto-generated configuration header */
#include "../../Kalman/kalman_config.h"

#ifndef PARAMETERS_CONFIG_H_
#define PARAMETERS_CONFIG_H_

#define PARAMETERS_CONFIG_USE_RAW_OCV 1
#define PARAMETERS_CONFIG_USE_LOOK_UP_TABLE KALMAN_CONFIG_POLY_USE_LOOKUPTABLE

// Default: do not use lookup table unless explicitly enabled (or -D at compile time)
#ifndef PARAMETERS_CONFIG_USE_LOOK_UP_TABLE
#define PARAMETERS_CONFIG_USE_LOOK_UP_TABLE 0
#endif

#define PARAMETERS_CONFIG_OCV_SIZE 1000
#define PARAMETERS_CONFIG_OCV_POLY_ORDER 15
#define PARAMETERS_BATTERY_REGION_NUMBER 22

// Filip: Modified
#define PARAMETERS_BATTERY_REGION_NUMBER_DOD 20
#define PARAMETERS_CONFIG_OCV_POLY_ORDERED 11
#define PARAMETERS_CONFIG_OCV_DERIV_POLY_ORDERED 10

#ifdef __cplusplus
}
#endif

#endif /* PARAMETERS_CONFIG_H_ */
