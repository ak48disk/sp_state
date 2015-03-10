#include <stdio.h>
#include <memory.h>
#include <queue>
#include <vector>
#include <unordered_map>
#include <set>
#include <assert.h>

#define T17

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
			unsigned char shadow_word_death_cd : 3;
			unsigned char shadow_word_death_state : 1;
			unsigned char dp_up_gcds : 4;
			unsigned char dp_mfi_gcds : 4;
			//
			unsigned char swp_up_gcds_2 : 5;
			unsigned char swp_state_2 : 1;
			unsigned char vt_up_gcds_2 : 5;
			unsigned char vt_state_2 : 1;
		};
		long long s;
	};
}state;

#define return_state(x) {state s;s.s=x;return s;}
#define max(a,b) (a) > (b) ? (a) : (b)
#define min(a,b) (a) < (b) ? (a) : (b)

typedef state(*transistion)(state s);

double damage;
double damage_total = 0;
double mastery_total = 0;
state start;

#define mastery_add 0.8

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
	if (s.shadow_word_death_cd) s.shadow_word_death_cd--;
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
	//
	if (s.swp_up_gcds_2)
	{
		s.swp_up_gcds_2--;
		s.swp_state_2 = ~s.swp_state_2;
		if (s.swp_state_2 == 0)
			deal_damage(47.5, 0);
	}
	if (s.vt_up_gcds_2)
	{
		s.vt_up_gcds_2--;
		s.vt_state_2 = ~s.vt_state_2;
		if (s.vt_state_2 == 0)
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
	if(s.mf4_state) return_state(-1);
	//if (s.swp_up_gcds > 2) return_state(-1); //
	if (s.swp_up_gcds == 0) s.swp_state = 0;
	s.swp_up_gcds = min(18,s.swp_up_gcds + 12);
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
	s.vt_up_gcds = min(15,s.vt_up_gcds + 10);
	return s;
}

state sw_pain_2(state s)
{
	if (s.mf4_state) return_state(-1);
	if (s.swp_up_gcds_2 > 5) return_state(-1); //
	if (s.swp_up_gcds_2 == 0) s.swp_state_2 = 0;
	s.swp_up_gcds_2 = min(18, s.swp_up_gcds_2 + 12);
	deal_damage(47.5, 0);
	s = gcd(s);
	return s;
}

state vt_2(state s)
{
	if (s.mf4_state) return_state(-1);
	if (s.vt_up_gcds_2 > 5) return_state(-1); //
	s = gcd(s);
	if (s.vt_up_gcds_2 == 0) s.vt_state_2 = 0;
	s.vt_up_gcds_2 = min(15, s.vt_up_gcds_2 + 10);
	return s;
}

state mf(state s)
{
	if (s.mf4_state) return_state(-1);
	int dp = s.dp_mfi_gcds;
	s =gcd(s);
	if (dp)
		deal_damage(2 * 33, 1);
	deal_damage(2 * 33, 1);
	return s;
}

state mf4(state s)
{
	int dp = s.dp_mfi_gcds || (s.mf4_state == 2);
	//if (!dp) { if (!s.mf4_state) return_state(-1); }
	s =gcd(s);
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

state death(state s)
{
	if (s.mf4_state) return_state(-1);
	if (s.shadow_word_death_cd) return_state(-1);
	s.shadow_orbs = min(5, s.shadow_orbs + 1);
	s.shadow_word_death_state = ~s.shadow_word_death_state;
	if (s.swp_up_gcds || s.vt_up_gcds) {
		deal_damage(270, 0);
	}
	else
	{
		deal_damage(270 * 1.4, 0);
	}
	
	if (!s.shadow_word_death_state) s.shadow_word_death_cd = 6;
	return gcd(s);
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
	
	death,
	sw_pain_2,
	vt_2,
};

char* trans_names[] = {
	"mind_blast",
	"sw_pain",
	"vt",
	"mf",
	"dp",
	"ms",
	"mf",
	
	"death",
	"sw_pain_2",
	"vt_2",
};


void dp()
{
	std::unordered_map<long long, double> v;
	v.insert(std::make_pair(start.s, 0));
	std::unordered_map<long long, std::vector<char>> routes;
	routes[start.s] = std::vector<char>();
	for (int step = 0; step < 300; ++step)
	{
		std::unordered_map<long long, double> v_next;
		std::unordered_map<long long, std::vector<char>> r_next;
		for (auto vertic : v)
		{
			state current;
			current.s = vertic.first;
			for (int i = 0; i < 7; ++i)
			{
				damage = 0;
				state new_state = transistions[i](current);
				if (new_state.s >= 0)
				{
					long long dest = new_state.s;
					if (v_next.find(dest) == v_next.end() ||
						(v_next[dest] < vertic.second + damage))
					{
						v_next[dest] = vertic.second + damage;
						r_next[dest] = routes[vertic.first];
						r_next[dest].push_back((char)i);
					}
				}
			}
		}
		printf("step = %d, n_v = %d\n", step, v_next.size());
		v = v_next;
		routes = r_next;
	}
	double max = 0; long long maxv = 0;
	for (auto vertic : v)
	{
		if (vertic.second > max)
		{
			max = vertic.second;
			maxv = vertic.first;
		}
	}
	auto route = routes[maxv];
	damage_total = 0;
	mastery_total = 0;
	state s = start;
	for (auto step : route)
	{
		printf("%s(%d)", trans_names[step], s.shadow_orbs);
		printf("->");
		s = transistions[step](s);
	}
	printf("%f+%f*mastery", damage_total, mastery_total);
}

int main()
{
	static_assert(sizeof(state) == sizeof(long long), "sizeof(state)!=4");
	start.s = 0;
	start.shadow_orbs = 5;
	dp();
	return 0;
}