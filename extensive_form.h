#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <set>

using namespace std;

typedef long card_t;
typedef string player_t;
typedef long record_t;
typedef unsigned long long virtaddr_t;

typedef vector<record_t> history_t;
class history_less_dfs;
class node;
class node_ptr_less_dfs;
class InformationSet;
class InformationSet_ptr_less_dfs;

class history_less_dfs : less<history_t> {
public:
	bool operator()(const history_t& a, const history_t& b) const;
};

class node {
public:
	// edges
	node* parent = nullptr;
	card_t just_happend = 0;
	map<card_t, shared_ptr<node>> children;

	// nodes
	double pi = 1.0;
	vector<double> cfv;
	player_t player;
	shared_ptr<InformationSet> I;
	virtaddr_t idx = NULL;

	~node();
	node();
	node(const node& n) = delete; // (temporary, will provide node-only and subtree-included)

	history_t h() const;
	node& operator() (const history_t& history); // n-generation descendants
	node* next_dfs_circular() const;
	virtual bool operator <(const node& b) const; // dfs is virtual, bfs is not yet deployed
};
class node_ptr_less_dfs {
public:
	bool operator()(const node* a, const node* b) const;
};
ostream& operator<<(ostream& os, const node& n);

class InformationSet : public set<node*, node_ptr_less_dfs> {
public:
	map<card_t, unsigned long> action_in;
	vector<card_t> action_out;
	// deprecated:
	// vector<double> sigma;
	// vector<double> cfr_cfd_sum_cumulate;

	~InformationSet();
	InformationSet();
	InformationSet(const node& n);
	bool operator <(const InformationSet& J) const;
};
class InformationSet_ptr_less_dfs {
public:
	virtual bool operator()(const InformationSet* a, const InformationSet* b) const;
};
ostream& operator <<(ostream& os, const InformationSet& I);

typedef set<InformationSet*, InformationSet_ptr_less_dfs> InformationCollection;
typedef map<InformationSet*, vector<double>, InformationSet_ptr_less_dfs> StrategyMapping;
ostream& operator <<(ostream& os, const InformationCollection& Collect);
ostream& operator <<(ostream& os, const StrategyMapping& Strategy);
vector<vector<double>> random_strategy(long N, long D);
