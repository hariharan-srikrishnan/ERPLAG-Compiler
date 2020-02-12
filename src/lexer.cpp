#include <bits/stdc++.h>
using namespace std;

void removeComments(string testcaseFile, string &cleanFile) {
	std::vector<pair<int, int> > commentMarkerLocations;
	int n = (int) testcaseFile.size();
	bool open = true;
	for(int i=0; i<n; i++) {
		if(testcaseFile[i] == '*') {
			if(i+1 < n && testcaseFile[i+1] == '*') {
				if(open)
					commentMarkerLocations.push_back(std::make_pair(i,0));
				else 
					commentMarkerLocations[commentMarkerLocations.size()-1].second = i;
				open = !open;
				i++;
			}
		}
	}
	for(auto p : commentMarkerLocations) {
		cout << p.first << " " << p.second << endl;	
	}
	int m = (int) commentMarkerLocations.size();
	int i;
	if(m == 0) {
		cleanFile = testcaseFile;
		return;
	}
	cleanFile.clear();
	int j = 0;
	while(i < n && j < m) {
		while(i < commentMarkerLocations[j].first) {
			cleanFile.push_back(testcaseFile[i]);
			i++;
		}
		i = commentMarkerLocations[j].second + 2;
		j++;
	}
	while(i < n) {
		cleanFile.push_back(testcaseFile[i]);
		i++;
	}
}

int main() {
	std::string testcaseFile, cleanFile;
	testcaseFile = "hello ** this is to be removed ** this is after removal";
	removeComments(testcaseFile, cleanFile);
	std::cout << cleanFile;
	std::cout << endl << endl;
}