#include <stdio.h>
#include <memory.h>
#include <hash_set>
#include <queue>
#include <vector>
#include <map>
#include <set>

typedef struct state {
	union {
		struct {
		/*	unsigned char swp_up_gcds:6;
			unsigned char swp_state : 1;
			unsigned char vt_state : 1;
			unsigned char vt_up_gcds : 6;
			unsigned char mf4_state : 2;
			unsigned char dp_up_gcds : 4;
			unsigned char mb_cd_gcds : 4;
			unsigned char dp_mfi_gcds : 4;
			unsigned char shadow_orbs: 4;*/
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
		};
		int s;
	};
}state;

#define return_state(x) {state s;s.s=x;return s;}
#define max(a,b) (a) > (b) ? (a) : (b)
#define min(a,b) (a) < (b) ? (a) : (b)

typedef state(*transistion)(state s);

double damage;

#define mastery_add 0.25

void deal_damage(double dmg, int mastery)
{
	damage += dmg;
	if (mastery)
		damage += dmg * mastery_add;
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
	return s;
}

state mind_blast(state s)
{
	if (s.mb_cd_gcds || s.mf4_state) return_state(-1);
	if (s.swp_up_gcds || s.vt_up_gcds)
	{
		deal_damage(180, 1);
	}
	else
	{
		deal_damage(180*1.4, 1);
	}
	s.mb_cd_gcds = 4;
	s = gcd(s);
	s.shadow_orbs = min(5, s.shadow_orbs + 1);
	
	return s;
}

state sw_pain(state s)
{
	if(s.mf4_state) return_state(-1);
	if (s.swp_up_gcds == 0) s.swp_state = 0;
	s.swp_up_gcds = min(18,s.swp_up_gcds + 12);
	deal_damage(47.5, 0);
	s = gcd(s);
	return s;
}

state vt(state s)
{
	if (s.mf4_state) return_state(-1);
	s = gcd(s);
	if (s.vt_up_gcds == 0) s.vt_state = 0;
	s.vt_up_gcds = min(15,s.vt_up_gcds + 10);
	return s;
}

state mf(state s)
{
	if (s.mf4_state) return_state(-1);
	int dp = s.dp_mfi_gcds;
	s =gcd(s);
	if (dp)
		deal_damage(2 * 30, 1);
	deal_damage(2 * 30, 1);
	return s;
}

state mf4(state s)
{
	int dp = s.dp_mfi_gcds || (s.mf4_state == 2);
	s =gcd(s);
	if (dp)
		deal_damage(2 * 30, 1);
	deal_damage(2 * 30, 1);
	if (!s.mf4_state)
		s.mf4_state = dp ? 2 : 1;
	else
		s.mf4_state = 0;
	return s;
}

state ms(state s)
{
	if (s.mf4_state) return_state(-1);
	s = gcd(s);
	if (s.swp_up_gcds | s.vt_up_gcds) {
		deal_damage(82.5, 1);
	}
	else
	{
		deal_damage(82.5 * 1.4, 1);
	}
	s.dp_up_gcds = 0;
	s.swp_up_gcds = 0;
	s.vt_up_gcds = 0;
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
	return gcd(s);
}

state death(state s)
{
	if (s.mf4_state) return_state(-1);
	if (s.shadow_word_death_cd) return_state(-1);
	s.shadow_orbs = min(5, s.shadow_orbs + 1);
	s.shadow_word_death_state = ~s.shadow_word_death_state;
	if (s.swp_up_gcds || s.vt_up_gcds) {
		deal_damage(161.775, 0);
	}
	else
	{
		deal_damage(161.775 * 1.4, 0);
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
	death
};

char* trans_names[] = {
	"mind_blast",
	"sw_pain",
	"vt",
	"mf",
	"dp",
	"ms",
	"mf",
	"death"
};

typedef struct edge
{
	state start;
	state end;
	double damage;
	char* trans_name;
};

std::map<int, std::vector<edge>> edges;

void bfs()
{
	int n_transitions = 0;
	std::hash_set<int> visited;
	state start; start.s = 0;
	std::queue<state> q;
	q.push(start);
	visited.insert(0);
	while (!q.empty())
	{
		state current = q.front(); q.pop();
		if (edges.find(current.s) == edges.end())
		{
			edges.insert(make_pair(current.s, std::vector<edge>()));
		}
		for (int i = 0; i < 7; ++i)
		{
			damage = 0;
			state new_state = transistions[i](current);
			if (new_state.s >= 0)
			{
				edge e;
				e.start = current;
				e.end = new_state;
				e.damage = damage;
				e.trans_name = trans_names[i];
				edges[current.s].push_back(e);
				n_transitions++;
				if (visited.find(new_state.s) == visited.end())
				{
					q.push(new_state);
					visited.insert(new_state.s);
				}
			}
			else
			{
				if (new_state.s != -1) 
				{
				}
			}
		}
	}
}

void dp()
{
	std::map<int, double> v;
	v.insert(std::make_pair(0, 0));
	std::map<int, std::vector<char*>> routes;
	routes[0] = std::vector<char*>();
	for (int step = 0; step < 200; ++step)
	{
		std::map<int, double> v_next;
		std::map<int, std::vector<char*>> r_next;
		for (auto vertic : v)
		{
			for (auto edge : edges[vertic.first])
			{
				int dest = edge.end.s;
				if (v_next.find(dest) == v_next.end() ||
					(v_next[dest] < vertic.second + edge.damage))
				{
					v_next[dest] = vertic.second + edge.damage;
					r_next[dest] = routes[vertic.first];
					r_next[dest].push_back(edge.trans_name);
				}
			}
		}
		printf("step = %d, n_v = %d\n", step, v_next.size());
		v = v_next;
		routes = r_next;
	}
	double max = 0; int maxv = 0;
	for (auto vertic : v)
	{
		if (vertic.second > max)
		{
			max = vertic.second;
			maxv = vertic.first;
		}
	}
	auto route = routes[maxv];
	for (auto step : route)
	{
		printf("%s", step);
		printf("->");
	}
}

int main()
{
	state s;
	static_assert(sizeof(state) == sizeof(int), "sizeof(state)!=4");
	s.s = 0;
	s.shadow_orbs = 5;
	printf("%d\n", s.s);
	bfs();
	dp();
	return 0;
}