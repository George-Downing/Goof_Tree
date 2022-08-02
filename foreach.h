#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <set>

using namespace std;

template<typename _Ty1, typename _Ty2>
ostream& operator<< (ostream&, const pair<_Ty1, _Ty2>&);
template<typename _Ty>
ostream& operator<< (ostream&, const vector<_Ty>&);
template<typename _Ty1, typename _Ty2>
ostream& operator<< (ostream&, const map<_Ty1, _Ty2>&);
template<typename _Ty>
ostream& operator<< (ostream&, const set<_Ty>&);

template<typename _Ty1, typename _Ty2>
ostream& operator<< (ostream& os, const pair<_Ty1, _Ty2>& p) {
	os << p.first << ": " << p.second;
	return os;
}

template<typename _Ty>
ostream& operator<< (ostream& os, const vector<_Ty>& V) {
	os << "[";
	if (V.size() != 0) {
		for (auto iter = V.begin(); ; iter++) {
			os << *iter;
			if (iter == prev(V.end())) {
				break;
			}
			os << ", ";
		}
	}
	os << "]";
	return os;
}

template<typename _Ty1, typename _Ty2>
std::ostream& operator<< (std::ostream& os, const map<_Ty1, _Ty2>& M) {
	os << "{";
	if (M.size() != 0) {
		for (auto p = M.begin(); ; p++) {
			os << *p;
			if (p == prev(M.end())) {
				break;
			}
			os << ", ";
		}
	}
	os << "}";
	return os;
}

template<typename _Ty>
ostream& operator<< (ostream& os, const set<_Ty>& S) {
	os << "[";
	if (S.size() != 0) {
		for (auto iter = S.begin(); ; iter++) {
			os << *iter;
			if (iter == prev(S.end())) {
				break;
			}
			os << ", ";
		}
	}
	os << "]";
	return os;
}
