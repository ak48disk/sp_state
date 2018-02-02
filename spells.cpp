#include "spells.h"

#define mastery_add 0.8
#define T17


#define return_state(x) {state s;s.s=x;return s;}
#define max(a,b) (a) > (b) ? (a) : (b)
#define min(a,b) (a) < (b) ? (a) : (b)

namespace {
	double damage;
	double damage_total = 0;
	double mastery_total = 0;
}


void damage_init()
{
	damage = 0;
	damage_total = 0;
	mastery_total = 0;
}

void damage_reset()
{
	damage = 0;
}

double damage_get()
{
	return damage;
}

void deal_damage(double dmg, int mastery)
{
	damage += dmg;
	if (mastery) {
		damage += dmg * mastery_add;
		mastery_total += dmg;
	}
	damage_total += dmg;
}

state gcd(state s)
{
	if (s.dp_up_gcds)s.dp_up_gcds--;
	if (s.mb_cd_gcds)s.mb_cd_gcds--;
	if (s.dp_mfi_gcds) s.dp_mfi_gcds--;
	if (s.swp_up_gcds)
	{
		s.swp_up_gcds--;
		s.swp_state = ~s.swp_state;
		if (s.swp_state == 0)
			deal_damage(47.5, 0);
	}
	if (s.vt_up_gcds)
	{
		s.vt_up_gcds--;
		s.vt_state = ~s.vt_state;
		if (s.vt_state == 0)
			deal_damage(58.5, 0);
	}
	return s;
}

state mind_blast(state s)
{
	if (s.mb_cd_gcds || s.mf4_state) return_state(-1);
	deal_damage(200, 1);
	s.mb_cd_gcds = 4;
	s = gcd(s);
	s.shadow_orbs = min(5, s.shadow_orbs + 1);

	return s;
}

state sw_pain(state s)
{
	if (s.mf4_state) return_state(-1);
	//if (s.swp_up_gcds > 2) return_state(-1); //
	if (s.swp_up_gcds == 0) s.swp_state = 0;
	s.swp_up_gcds = min(18, s.swp_up_gcds + 12);
	deal_damage(47.5, 0);
	s = gcd(s);
	return s;
}

state vt(state s)
{
	if (s.mf4_state) return_state(-1);
	//if (s.vt_up_gcds > 2) return_state(-1); //
	s = gcd(s);
	if (s.vt_up_gcds == 0) s.vt_state = 0;
	s.vt_up_gcds = min(15, s.vt_up_gcds + 10);
	return s;
}

state mf(state s)
{
	if (s.mf4_state) return_state(-1);
	int dp = s.dp_mfi_gcds;
	s = gcd(s);
	if (dp)
		deal_damage(2 * 33, 1);
	deal_damage(2 * 33, 1);
	return s;
}

state mf4(state s)
{
	int dp = s.dp_mfi_gcds || (s.mf4_state == 2);
	//if (!dp) { if (!s.mf4_state) return_state(-1); }
	s = gcd(s);
	if (dp)
		deal_damage(2 * 33, 1);
	deal_damage(2 * 33, 1);
	if (!s.mf4_state)
		s.mf4_state = dp ? 2 : 1;
	else
		s.mf4_state = 0;
	return s;
}

state ms(state s)
{
	if (s.mf4_state) return_state(-1);
	if (s.dp_mfi_gcds) return_state(-1);
	s = gcd(s);
	if (s.swp_up_gcds | s.vt_up_gcds) {
		deal_damage(90.8, 1);
	}
	else
	{
		deal_damage(90.8 * 1.4, 1);
	}
	s.dp_up_gcds = 0;
	s.swp_up_gcds = 0;
	s.vt_up_gcds = 0;
	s.swp_state = 0;
	s.vt_state = 0;
	return s;
}


state dp(state s)
{
	if (s.mf4_state) return_state(-1);
	if (s.shadow_orbs < 3) return_state(-1);
	s.shadow_orbs -= 3;
	s.dp_up_gcds = 4;
	s.dp_mfi_gcds += 4;
	deal_damage(600, 0);
#ifdef T17
	if (s.mb_cd_gcds > 0)s.mb_cd_gcds = s.mb_cd_gcds - 1;
#endif
	return gcd(s);
}

void disabled(int* d, state s) {
	d[0] = (s.mb_cd_gcds || s.mf4_state) ? 1 : 0;
	d[1] = s.mf4_state ? 1 : 0;
	d[2] = s.mf4_state ? 1 : 0;
	d[3] = s.mf4_state ? 1 : 0;
	d[4] = (s.mf4_state || s.shadow_orbs < 3) ? 1 : 0;
	d[5] = (s.mf4_state || s.dp_mfi_gcds) ? 1 : 0;
	d[6] = 0;
}

const char* trans_names[] = {
	"mind_blast",
	"sw_pain",
	"vt",
	"mf",
	"dp",
	"ms",
	"mf",
};


const char** get_trans_names()
{
	return trans_names;
}

transistion transistions[] =
{
	mind_blast,
	sw_pain,
	vt,
	mf,
	dp,
	ms,
	mf4,
};

const transistion* get_transistions()
{
	return transistions;
}
