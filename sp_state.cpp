#include <stdio.h>
#include <memory.h>
#include <queue>
#include <vector>
#include <unordered_map>
#include <set>
#include <assert.h>

#include "spells.h"



state start;

void dp()
{
	std::unordered_map<long long, double> v;
	v.insert(std::make_pair(start.s, 0));
	std::unordered_map<long long, std::vector<char>> routes;
	routes[start.s] = std::vector<char>();
	auto transistions = get_transistions();
	auto trans_names = get_trans_names();
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
				damage_reset();
				state new_state = transistions[i](current);
				if (new_state.s >= 0)
				{
					long long dest = new_state.s;
					if (v_next.find(dest) == v_next.end() ||
						(v_next[dest] < vertic.second + damage_get()))
					{
						v_next[dest] = vertic.second + damage_get();
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
	//damage_total = 0;
	//mastery_total = 0;
	state s = start;
	for (auto step : route)
	{
		printf("%s(%d)", trans_names[step], s.shadow_orbs);
		printf("->");
		s = transistions[step](s);
	}
	printf("%f\n", damage_get());
	//printf("%f+%f*mastery", damage_total, mastery_total);
}

/*
int main()
{
	static_assert(sizeof(state) == sizeof(long long), "sizeof(state)!=4");
	start.s = 0;
	dp();
	// 92727.112000
	return 0;
}
*/