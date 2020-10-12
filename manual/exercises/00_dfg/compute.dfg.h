#ifndef __compute_H__
#define __compute_H__
#define P_compute_A 16
#define P_compute_B 20
#define P_compute_C 1

#define P_compute_O 12

// CAUTION: This is a serialization-based version
// of the schedule.  (ie. cheating)  It is for simulation only.
// corresponding dfg is in: compute.*.json

#define compute_size 96

char compute_config[96] = "filename:compute";

#endif //compute_H
