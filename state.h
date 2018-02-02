#pragma once
typedef struct state {
	union {
		struct {
			unsigned char swp_up_gcds : 5; //max 31
			unsigned char shadow_orbs : 3;
			unsigned char vt_up_gcds : 5;
			unsigned char mf4_state : 2;
			unsigned char swp_state : 1;
			unsigned char mb_cd_gcds : 3;
			unsigned char vt_state : 1;
			unsigned char dp_up_gcds : 4;
			unsigned char dp_mfi_gcds : 4;
		};
		long long s;
	};
}state;

#define num_states 9
