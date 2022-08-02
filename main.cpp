#include <iostream>
#include <numeric>  // iota range, mpl
#include "foreach.h"
#include "extensive_form.h"
#include <matplotlibcpp.h>
#include <Eigen/Dense>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

using namespace std;
using namespace Eigen;

namespace GAME {
	unsigned long PLAYER_NUM = 2;
	map<player_t, unsigned long> PLAYER_IN = {{"A", 0}, {"B", 1}};
	vector<card_t> UPCARD({1, 1, 1});
	
	// vector<vector<double>> CARD_VALUE({{3.00, 1.00, 2.00, 4.00}, {2.00, 4.00, 6.00, 1.00}});
	vector<vector<double>> CARD_VALUE({ {3.00, 1.00, 2.00}, {2.00, 4.00, 6.00} });

	vector<vector<double>> k = {{0.96, 0.04}, {0.04, 0.96}}; // the multi-attention array of CFR-NZS
	vector<player_t> PLAYER_OUT = {"A", "B"};

	player_t next_player(player_t i) {
		vector<player_t>::iterator j = next(find(PLAYER_OUT.begin(), PLAYER_OUT.end(), i));
		if (j == PLAYER_OUT.end()) j = PLAYER_OUT.begin();
		return *j;
	}

	vector<double> payoff_leaves(const node& node) {
		long UPCARD_NUM = UPCARD.size();
		vector<double> payoff(PLAYER_NUM, 0.0);
		history_t h = node.h();

		for (long i = 0; i < UPCARD_NUM; i++) {
			if (h[PLAYER_NUM * i] > h[PLAYER_NUM * i + 1]) {
				payoff[0] += CARD_VALUE[0][i];
				payoff[1] -= CARD_VALUE[1][i];
			}
			else if (h[PLAYER_NUM * i] < h[PLAYER_NUM * i + 1]) {
				payoff[0] -= CARD_VALUE[0][i];
				payoff[1] += CARD_VALUE[1][i];
			}
		}
		return payoff;
	}

	void tree_generate(node* root) {
		map<player_t, set<card_t>> deck;
		node* p = root;

		for (pair<player_t, int> i : PLAYER_IN) for (int c = 1; c <= UPCARD.size(); c++) deck[i.first].insert(c);
		
		p->player = PLAYER_OUT[0];
		do {
			for (card_t i : deck[p->player]) {
				p->children[i] = make_shared<node>();
				p->children[i]->parent = p;
				p->children[i]->just_happend = i;
				p->children[i]->player = next_player(p->player);
			}
			p->cfv.resize(PLAYER_OUT.size());

			if (p->children.size()) {
				p = p->children.begin()->second.get();
				deck[p->parent->player].erase(p->just_happend);
			}
			else {
				while (p->parent) {
					deck[p->parent->player].insert(p->just_happend);
					if (p != prev(p->parent->children.end())->second.get()) {
						p = next(p->parent->children.find(p->just_happend))->second.get();
						deck[p->parent->player].erase(p->just_happend);
						break;
					}
					p = p->parent;
				}
			}
		} while (p->parent);
		return;
	}
	void leaf_payoff(node* root) {
		node* p = root;
		do {
			if (p->children.size() == 0) p->cfv = GAME::payoff_leaves(*p);
			p = p->next_dfs_circular();
		} while (p->parent);
	}
	void tree_to_vec_transformation(node* root, vector<InformationCollection>* Collect_of_Player) {
		node* n = root;
		do {
			if (PLAYER_IN[n->player] == 0) {
				if (true) {
					n->I = make_shared<InformationSet>(*n);
					(*Collect_of_Player)[PLAYER_IN[n->player]].insert(n->I.get());
				}
			}
			else if (PLAYER_IN[n->player] == 1) {
				if (n->just_happend == n->parent->children.begin()->first) {
					n->I = make_shared<InformationSet>(*n);
					(*Collect_of_Player)[PLAYER_IN[n->player]].insert(n->I.get());
				}
				else {
					n->I = n->parent->children.begin()->second.get()->I;
					n->I->insert(n);
				}
				
			}
			n = n->next_dfs_circular();
		} while (n->parent);
	}
	void non_leaf_pi_cfv(node* root, const vector<StrategyMapping>& strategy_profile) {
		node* n;
		node* head;
		map<card_t, shared_ptr<node>>::iterator n_child_unvisited_begin;
		unsigned long play;
		InformationSet* info;
		unsigned long act;
		bool reverse;
		
		n = root;
		n_child_unvisited_begin = n->children.begin();
		n->pi = 1.0;
		fill(n->cfv.begin(), n->cfv.end(), 0.0);

		do
		{
			if (n_child_unvisited_begin != n->children.end()) {
				reverse = false;
				head = n_child_unvisited_begin->second.get();
				n = head;
				n_child_unvisited_begin = n->children.begin();
			}
			else if (!n->parent) break;
			else {
				reverse = true;
				head = n;
				n = n->parent;
				n_child_unvisited_begin = next(head->parent->children.find(head->just_happend));
			}
			// loop body for ascending edges:
			{
				play = PLAYER_IN[head->parent->player];
				info = head->parent->I.get();
				act = head->parent->I->action_in[head->just_happend];
				if (reverse == false) {
					if (head->children.size() != 0) fill(head->cfv.begin(), head->cfv.end(), 0.0);
					head->pi = head->parent->pi * (const double)strategy_profile[play].find(info)->second[act];
				}
				else {
					for (int i = 0; i < head->parent->cfv.size(); i++) {
						head->parent->cfv[i] += head->cfv[i] * (const double)strategy_profile[play].find(info)->second[act];
					}
				}
			}
			
		} while (true);
	}
};

int main_goofspiel(int argc, char** argv)
{
	node root;
	node* n = nullptr;
	long T = 10; // both 0 and T are included
	bool _print = false;
	

	cout << "tree generating..." << endl;
	GAME::tree_generate(&root);
	GAME::leaf_payoff(&root);

	cout << "TREE-TO-VEC-TRANSFORMATION: convert TREE searching problem into QUADRATIC/POLYNOMIAL optimizing problem:" << endl;
	vector<InformationCollection> InfoCollect(GAME::PLAYER_NUM);
	// double sigma_updated[long play][InformationSet* I][long act]
	vector<map<InformationSet*, vector<double>, InformationSet_ptr_less_dfs>> sigma_updated(GAME::PLAYER_NUM);
	// double sigma_history = [long play][InformationSet* I][long act][long t]
	vector<map<InformationSet*, vector<vector<double>>, InformationSet_ptr_less_dfs>> sigma_history(GAME::PLAYER_NUM);
	vector<map<InformationSet*, vector<double>, InformationSet_ptr_less_dfs>> CFR_CFD_Sum_Cumul(GAME::PLAYER_NUM);

	GAME::tree_to_vec_transformation(&root, &InfoCollect);
	// init(sigma)
	for (int play = 0; play < GAME::PLAYER_NUM; play++) {
		for (InformationSet* I : InfoCollect[play]) {
			sigma_updated[play][I] = random_strategy(1, I->action_out.size())[0];
			sigma_history[play][I].resize(I->action_out.size());
			for (vector<double>& x : sigma_history[play][I]) x.resize(1 + T);
			CFR_CFD_Sum_Cumul[play][I] = vector<double>(GAME::PLAYER_NUM, 0.0);
		}
	}
	// 20220706: load(root, sigma)

	
	
	for (int t = 0; ; t++) {
		_print = false;
		if (t < 10) {
			if (t % 1 == 0) _print = true;
		}
		else if (t < 20) {
			if (t % 2 == 0) _print = true;
		}
		else if (t < 50) {
			if (t % 5 == 0) _print = true;
		}
		else if (t < 100) {
			if (t % 10 == 0) _print = true;
		}
		else {
			if (t % 100 == 0) _print = true;
		}

		if (_print) {
			cout << "t: " << t << endl;
			cout << "sigma:" << endl;
			for (int i = 0; i < GAME::PLAYER_NUM; i++) cout << "sigma[" << GAME::PLAYER_OUT[i] << "]:" << endl << sigma_updated[i] << endl;
			cout << endl;
		}

		if (_print) cout << "coping INPUT: sigma..." << endl;
		for (int play = 0; play < GAME::PLAYER_NUM; play++) {
			for (InformationSet* I : InfoCollect[play]) {
				for (int act = 0; act < I->action_out.size(); act++) {
					sigma_history[play][I][act][t] = sigma_updated[play][I][act];
				}
			}
		}
		
		if (_print) cout << "calculating OUTPUT: [pi, cfv]..." << endl;
		GAME::non_leaf_pi_cfv(&root, sigma_updated);

		if (false) {
			n = &root;
			do {
				if (n->children.size() > 1) cout << n->h() << ", pi: " << n->pi << ", cfv: " << n->cfv << ", action: " << n->I->action_out << endl;
				n = n->next_dfs_circular();
			} while (n->parent);
			cout << endl;
		}
		
		if (t == T) break;
		cout << "#---=---=---=---#---=---=---=---#---=---=---=---#---=---=---=---" << endl;
		
		
		for (long i = 0; i < GAME::PLAYER_NUM; i++) {
			for (InformationSet* I : InfoCollect[i]) {
				if (I->action_out.size() <= 1) continue;
				
				vector<double> feed(I->action_out.size(), 0.0);
				double weight;
				for (long j = 0; j < GAME::PLAYER_NUM; j++) {
					// original definition of feed, and weight = sum(feed)
					for (long act = 0; act < I->action_out.size(); act++) {
						feed[act] = accumulate(I->begin(), I->end(), 0.0,
							[&j, &act](double x, node* n) {return x + (n->pi) * (n->children[n->I->action_out[act]]->cfv[j] - n->cfv[j]);});
					}
					if (j != i) for (double& x : feed) x = -x;
					for (double& x : feed) if (x < 0) x = 0;
					weight = accumulate(feed.begin(), feed.end(), 0.0);
					
					// make weight/CFR_CFD_Sum_Cumul <= 1; still, 0.0/0.0 should be dedicatedly handled (set as 1.0)
					CFR_CFD_Sum_Cumul[i][I][j] += weight;
					
					// feed / weight: Zero Division Prevention
					if (weight != 0) for (double& x : feed) x /= weight;
					else for (double& x : feed) x = 1.0 / feed.size();
					
					// weight / CFR_CFD_Sum_Cumul[i][I][j]: Zero Division Prevention
					if (CFR_CFD_Sum_Cumul[i][I][j] != 0) weight /= CFR_CFD_Sum_Cumul[i][I][j];
					else weight = 1;
					
					// CFR-NZS update
					for (long act = 0; act < feed.size(); act++) sigma_updated[i][I][act] += weight * GAME::k[i][j] * (feed[act] - sigma_history[i][I][act][t]);
					
					// If you want to converge faster, enable it.
					// If you want to stick strictly to CFR-NZS, disable it.
					// The annealing coefficient can be adjusted.
					CFR_CFD_Sum_Cumul[i][I][j] *= 0.96;
				}
				// eliminate some really tiny floating number precision errors
				for (double& x : sigma_updated[i][I]) x = x >= 0 ? x : 0;
			}
		}
	}

	// 20220706: save(root, sigma)

	
	// (OPTIONAL) If the server does not have matplotlib installed on BASE python interpreter, DISABLE this part of code:
	namespace plt = matplotlibcpp;
	
	vector<vector<double>>(*vec_d_stack)(const vector<vector<double>>&) = [](const vector<vector<double>>& f)->vector<vector<double>> {
		vector<vector<double>> F = f;
		for (long act = 1; act < f.size(); act++) {
			for (long t = 0; t < F[0].size(); t++) {
				F[act][t] += F[act - 1][t];
			}
		}
		return F;
	};

	vector<double> t(1 + T);
	iota(t.begin(), t.end(), 0.0);

	long fig_num = 1;
	for (long play = 0; play < GAME::PLAYER_NUM; play++) {
		for (pair<InformationSet*, vector<vector<double>>> IS_P: sigma_history[play]) {
			InformationSet* I = IS_P.first;
			vector<vector<double>> F;
			stringstream fig_title;

			if (I->action_out.size() <= 1) continue;
			plt::figure(fig_num);
			F = vec_d_stack(sigma_history[play][I]);
			F.insert(F.begin(), vector<double>(1 + T, 0.0));

			for (long act = 0; act < I->action_out.size(); act++) plt::fill_between(t, F[act], F[1 + act], { {"label", to_string(I->action_out[act])} });

			fig_title << "play: " << GAME::PLAYER_OUT[play] << ", (*I.begin())->h(): " << (*I->begin())->h();
			plt::title(fig_title.str(), { {"fontweight", "1"}, {"fontsize", "10"} });
			plt::legend();
			plt::savefig("PERCENTS/" + to_string(fig_num - 1) + ".png");
			plt::close();
			fig_num++;
		}
	}
	// (OPTIONAL) If the server does not have matplotlib installed on BASE python interpreter, DISABLE this part of code.

	return 0;
}

int main_mpl(int argc, char** argv) {
	namespace plt = matplotlibcpp;
	vector<double> t = { 0.00, 1.00, 2.00, 3.00 };
	vector<double> set0_act0 = { 0.12, 0.15, 0.26, 0.25 };
	vector<double> set0_act1 = { 0.16, 0.18, 0.24, 0.25 };
	
	return 0;
}

int main(int argc, char** argv)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	srand(133484);
	main_goofspiel(0, {});
	// main_mpl(0, {});
	return 0;
}

