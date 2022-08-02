#include <algorithm>
#include "foreach.h"
#include "extensive_form.h"

// #include <Eigen\Dense>
// #include <numeric>  // iota range
// #include <algorithm> // shuffle, reverse

using namespace std;
// using namespace Eigen;

bool history_less_dfs::operator()(const history_t& a, const history_t& b) const {
	long La = a.size();
	long Lb = b.size();
	for (long i = 0; i < La && i < Lb; i++) {
		if (a[i] < b[i]) {
			return true;
		}
		else if (a[i] > b[i]) {
			return false;
		}
	}
	if (La < Lb) return true;
	else return false;
}

node::~node() {}
node::node() {}
history_t node::h() const {
	history_t h;
	node const* p;
	for (p = this; p->parent; p = p->parent) {
		h.push_back(p->just_happend);
	}
	reverse(h.begin(), h.end());
	return h;
}
node& node::operator() (const history_t& history) {
	node* p = this;
	for (record_t h : history) {
		p = p->children[h].get();
	}
	return *p;
}
node* node::next_dfs_circular() const {
	node* p = (node* const)this;
	if (p->children.size()) {
		p = p->children.begin()->second.get();
		return p;
	}
	while (p->parent) {
		if (p != prev(p->parent->children.end())->second.get()) {
			p = next(p->parent->children.find(p->just_happend))->second.get();
			break;
		}
		p = p->parent;
	}
	return p;
}
bool node::operator<(const node& b) const {
	return this->h() < b.h();
}
bool node_ptr_less_dfs::operator()(const node* a, const node* b) const {
	return *a < *b;
}
ostream& operator<<(ostream& os, const node& n) {
	os << n.h();
	return os;
}

InformationSet::~InformationSet() {}
InformationSet::InformationSet() {}
InformationSet::InformationSet(const node& n)
{
	unsigned long i = 0;
	for (pair<card_t, shared_ptr<node>> q : n.children) {
		action_in[q.second->just_happend] = i;
		action_out.push_back(q.second->just_happend);
		i++;
	}
	this->insert((node*)&n);
}
bool InformationSet::operator<(const InformationSet& J) const {
	return *(*this->begin()) < *(*J.begin());
	// return *this->begin() < *J.begin(); // pointer itself is the source of randomness
}
bool InformationSet_ptr_less_dfs::operator()(const InformationSet* I, const InformationSet* J) const {
	return *I < *J;
}
ostream& operator <<(ostream& os, const InformationSet& I) {
	os << "I: {";
	if (I.size() != 0) {
		for (auto iter = I.begin(); ; iter++) {
			os << (*iter)->h();
			if (iter == prev(I.end())) {
				break;
			}
			os << ", ";
		}
	}
	os << "}";
	os << ", pi: {";
	if (I.size() != 0) {
		for (auto iter = I.begin(); ; iter++) {
			os << (*iter)->pi;
			if (iter == prev(I.end())) {
				break;
			}
			os << ", ";
		}
	}
	os << "}";
	os << ", action: " << I.action_out;
	// os << ", sigma: " << I.sigma;
	return os;
}

ostream& operator <<(ostream& os, const InformationCollection& Collect) {
	bool print;
	bool printed = false;
	os << "{";
	for (InformationSet* I: Collect) {
		print = I->action_out.size() > 1 ? true : false;
		if (print) {
			if (printed) {
				os << ";" << endl;
			}
			printed = true;
			os << *I;
		}
	}
	os << "}";
	return os;
}

ostream& operator <<(ostream& os, const StrategyMapping& Sigma) {
	bool print;
	bool printed = false;
	os << "{";
	for (pair<InformationSet*, vector<double>> IS_P : Sigma) {
		print = IS_P.first->action_out.size() > 1 ? true : false;
		if (print) {
			if (printed) {
				os << ";" << endl;
			}
			printed = true;
			os << *IS_P.first << ", distribuiton: " << IS_P.second;
		}
	}
	os << "}";
	return os;
}
vector<vector<double>> random_strategy(long N, long D) {
	vector<vector<double>> y(N, vector<double>(D));
	long i;
	long d;
	if (D <= 0) return y;
	for (i = 0; i < N; i++) {
		for (d = 0; d < D - 1; d++) y[i][d] = rand() / (double)RAND_MAX;
		y[i][D - 1] = 1.0;
		sort(y[i].begin(), y[i].end());
		for (d = D - 1; d >= 1; d--) {
			y[i][d] -= y[i][d - 1];
		}
	}
	return y;
}
