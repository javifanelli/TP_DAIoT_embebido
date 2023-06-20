/*
 * sntp_time.h
 *
 *  Created on: 2021-04-17
 *      Author: Leopoldo Zimperz
 */

#ifndef SNTP_TIME_H_
#define SNTP_TIME_H_

char formatted_time[20];

void obtain_time(void);
void initialize_sntp(void);

#endif /* SNTP_TIME_H_ */
