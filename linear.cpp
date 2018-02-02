#include <limits>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <algorithm>

#include "spells.h"


struct params {
	float w[num_spells][num_states];
	float b[num_spells];
};


int select_spell(const params* p, state s)
{
	static int spell_disabled[num_spells] = { 0 };
	static float spell_vals[num_spells] = { 0 };
	disabled(spell_disabled, s);
	for (int i = 0; i < num_spells; ++i) {
		spell_vals[i] =
			p->w[i][0] * s.dp_mfi_gcds +
			p->w[i][1] * s.dp_up_gcds +
			p->w[i][2] * s.mb_cd_gcds +
			p->w[i][3] * s.mf4_state +
			p->w[i][4] * s.shadow_orbs +
			p->w[i][5] * s.swp_state +
			p->w[i][6] * s.swp_up_gcds +
			p->w[i][7] * s.vt_state +
			p->w[i][8] * s.vt_up_gcds + p->b[i];
	}
	float max_val = -FLT_MAX; int max = -1;
	for (int i = 0; i < num_spells; ++i) {
		if (!spell_disabled[i] && spell_vals[i] > max_val) {
			max_val = spell_vals[i];
			max = i;
		}
	}
	return max;
}

double run(const params* p, bool print_route=false)
{
	state s; s.s = 0;
	auto spells = get_transistions(); damage_init();
	auto trans_names = get_trans_names();
	for (int step = 0; step < 300; ++step) {
		int spell = select_spell(p, s);
		if (spell < 0 || spell > num_spells) {
			printf("err\n");
			exit(0);
		}
		if (print_route)
		{
			printf("%s(%d)", trans_names[spell], s.shadow_orbs);
			printf("->");
		}
		s = spells[spell](s);
	}
	return damage_get();
}

void rp_init()
{
	srand(time(NULL));
}

int get_rp(int limit)
{
	return (rand() * rand()) % limit;
}

bool rp(int chance/*1000-100%*/)
{
	return get_rp(1000) < chance;
}

void rand_p(params* p)
{ 
	for (int i = 0; i < num_spells; ++i)
	{
		for (int j = 0; j < num_states; ++j) {
			p->w[i][j] = get_rp(10000);
		}
		p->b[i] = get_rp(10000);
	}
}

double test_run()
{
	params p;
	rand_p(&p);
	return run(&p);
}

float new_value(float a, float b)
{
	int rp = get_rp(1000);
	if (rp < 300) return a;
	if (rp < 600) return b;
	float f = (rp - 600) / 400.0;
	return a * f + b * (1 - f);
}

float maybe_mutate(float v)
{
	int rp = get_rp(10000);
	if (rp < 7000) return v;
	if (rp < 9000) return v + (rp - 7000);
	float f = (rp - 9000) / 1000.0;
	if (f < 0.5) {
		f = 1 + 4 * (0.5 - f);
		return v * f;
	}
	else {
		f = 1 + 4 * (f - 0.5);
		return v / f;
	}
}

void new_p(params* p, const params* pa, const params* pb)
{
	for (int i = 0; i < num_spells; ++i)
	{
		for (int j = 0; j < num_states; ++j) {
			p->w[i][j] = maybe_mutate(new_value(pa->w[i][j], pb->w[i][j]));
		}
		p->b[i] = maybe_mutate(new_value(pa->b[i], pb->b[i]));
	}
}

params dna()
{
	params* ps = new params[100000];
	params* good_ps = new params[1000];

	std::vector<std::pair<int, float>> vs(100000);

	for (int i = 0; i < 100000; ++i) rand_p(&ps[i]);

	for (int it = 0; it < 1000; ++it)
	{
		for (int i = 0; i < 100000; ++i) {
			vs[i] = std::make_pair(i, run(&ps[i]));
		}
		std::sort(vs.begin(), vs.end(), [](auto a, auto b)->bool {return a.second > b.second; });


		printf("it=%d val=%f\n", it, vs[0].second);

		for (int i = 0; i < 1000; ++i) {
			good_ps[i] = ps[vs[i].first];
		}

		for (int i = 0; i < 100000; ++i) {
			if (i < 1000) {
				ps[i] = good_ps[i];
			}
			else if (i < 90000) {
				auto pa = &good_ps[get_rp(1000)];
				auto pb = &good_ps[get_rp(1000)];
				new_p(&ps[i], pa, pb);
			}
			else {
				rand_p(&ps[i]);
			}
		}
	}
	params retval = good_ps[0];
	delete[] ps;
	return retval;
}

int main()
{
	static_assert(sizeof(state) == sizeof(long long), "sizeof(state)!=4");
	rp_init();
	float m = 0; int l = 1;
	for (int k = 0; k < 6; ++k)
	{
		for (int i = 0; i < l; ++i) {
			double d = test_run();
			if (d > m) m = d;
		}
		printf("%d %f\n",l, m);
		l = l * 10;
	}
	params max_p = dna();
	run(&max_p, true);
	// 92727.112000
	return 0;
}