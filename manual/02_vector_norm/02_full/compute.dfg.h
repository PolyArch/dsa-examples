#ifndef __compute_H__
#define __compute_H__
#define P_compute_A 14
#define P_compute_SIGNAL 10
#define P_compute_NORM 16
#define P_compute_V 1
#define P_compute_COEF 20

#define P_compute_O 12
#define P_compute_INV 1
#define P_compute_RES 0

// CAUTION: This is a serialization-based version
// of the schedule.  (ie. cheating)  It is for simulation only.
// corresponding dfg is in: compute.*.json

#define compute_size 96

char compute_config[96] = "filename:compute";

#endif //compute_H
