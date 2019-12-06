#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include "File.h"
#include "Ngram.h"      
#include "Vocab.h"
using namespace std;
#define ORDER 2
#define NOTHING -1
#define SMALL -2147483647

/* ./mydisambig $1:input $2:ZBmap $3:LM $4:output */
int main(int argc, char const *argv[])
{
	/* Language Model */
	Vocab voc;
	Ngram lm(voc, ORDER);
	File lmFile(argv[3], "r");
	lm.read(lmFile);
	lmFile.close();

	/* Mapping */
	map<string, vector<string>> ZBmap;
	fstream mapFile;
	mapFile.open(argv[2], ios::in|ios::binary);
	string str;
	while (getline(mapFile, str)) {
		string ZhuYin = str.substr(0, 2);
		vector<string> Big5_word;
		for (int i = 3; i < str.size(); i += 3) 
			Big5_word.push_back(str.substr(i, 2));
		ZBmap[ZhuYin] = Big5_word;
	}
	mapFile.close();

	/* Test Data */
	fstream fin, fout;
	fin.open(argv[1], ios::in|ios::binary);
	fout.open(argv[4], ios::out|ios::binary);
	while (getline(fin, str)) {
		/* input sentance */
		string input;
		for (int i = 0; i < str.size(); i++) 
			if (str[i] != ' ') input += str[i];
		
		/* veterbi algorithm */
		vector<vector<string>> words;
		vector<vector<double>> probs;
		vector<vector<int>> last_Idx;
		vector<string> end_words;
		end_words.push_back("</s>");

		for (int i = 0; i < input.size() / 2; i++) 
			words.push_back(ZBmap[input.substr(i * 2, 2)]);
		words.push_back(end_words);

		/* initial */
		vector<double> prob;
		vector<int> last_idx;
		for (int j = 0; j < words[0].size(); j++) {
			VocabIndex w1 = voc.getIndex(words[0][j].c_str());
			w1 = (w1 == Vocab_None)? voc.getIndex(Vocab_Unknown): w1;
			VocabIndex w2 = voc.getIndex("<s>");
			w2 = (w2 == Vocab_None)? voc.getIndex(Vocab_Unknown): w2;
			VocabIndex context[] = {w2, Vocab_None};
			prob.push_back(lm.wordProb(w1, context));
			last_idx.push_back(NOTHING);
		}
		probs.push_back(prob);
		last_Idx.push_back(last_idx);
		prob.clear();
		last_idx.clear();

		/* recursion */
		for (int i = 1; i < words.size(); i++) {
			for (int j = 0; j < words[i].size(); j++) {
				VocabIndex w1 = voc.getIndex(words[i][j].c_str());
				w1 = (w1 == Vocab_None)? voc.getIndex(Vocab_Unknown): w1;
				double max_p = SMALL;
				int mark = 0;
				for (int k = 0; k < words[i - 1].size(); k++) {
					VocabIndex w2 = voc.getIndex(words[i - 1][k].c_str());
					w2 = (w2 == Vocab_None)? voc.getIndex(Vocab_Unknown): w2;
					VocabIndex context[] = {w2, Vocab_None};
					double p = lm.wordProb(w1, context) + probs[i - 1][k];
					mark = (p > max_p)? k: mark;
					max_p = (p > max_p)? p: max_p;
				}
				prob.push_back(max_p);
				last_idx.push_back(mark);
			}
			probs.push_back(prob);
			last_Idx.push_back(last_idx);
			prob.clear();
			last_idx.clear();
		}

		/* output */
		double max_p = SMALL;
		int mark = 0, len = words.size();
		for (int i = 0; i < words[len - 1].size(); i++) {
			if (probs[len - 1][i] > max_p) {
				max_p = probs[len - 1][i];
				mark = i;
			}
		}
		string out;
		for (int i = len - 1; i >= 0 && mark != NOTHING; i--) {
			out = " " + words[i][mark] + out;
			mark = last_Idx[i][mark];
		}
		fout << "<s>" << out << endl;
	}
	fin.close();
	fout.close();

	return 0;
}
