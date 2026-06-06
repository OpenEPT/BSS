/*
 * profiler.h
 *
 *  Created on: Oct 6, 2025
 *      Author: elektronika
 */

#ifndef PROFILER_PROFILER_H_
#define PROFILER_PROFILER_H_

#ifdef __cplusplus
extern "C" {
#endif

void PROFILER_Reset();

void PROFILER_Start();

void PROFILER_Stop();

unsigned int PROFILER_GetValue();

#ifdef __cplusplus
}
#endif

#endif /* PROFILER_PROFILER_H_ */
