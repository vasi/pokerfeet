#include <cstdio>
#include <set>
#include <vector>
#include <algorithm>
#include <bitset>
using namespace std;

typedef int Index;
const Index IndexMax = 52;
enum Suit { Club = 0, Diamond, Heart, Spade, SuitMax };
enum Color { Black = 0, Red, ColorMax };
typedef int Number;
const Number NumMax = IndexMax / SuitMax;
const Number Ace = NumMax;

const char *SuitChar = "CDHS";
const char *NumChar = "A23456789TJQK";
typedef char CardID[3];

struct Card {
	Number num;
	Suit suit;
	
	Card(Index i = 0) : num(i % NumMax), suit((Suit)(i / NumMax)) { }
	
	void id(CardID c) const {
		c[0] = NumChar[num];
		c[1] = SuitChar[suit];
	}
	
	Color color() const {
		return (suit == Diamond || suit == Heart) ? Red : Black;
	}
};


enum PokerHand {
	#define T(X) X,
	#include "pokerHands.def"
};
const char *PokerHandNames[] = {
	#define T(X) #X,
	#include "pokerHands.def"
};

struct PokerHandSet {
	bitset<PokerHandsMax> bits;
	void set(PokerHand p) { bits.set(p); }
	bool has(int i) const { return bits.test(i); }
	bool has(PokerHand p) const { return has((int)p); }
};

const int HandSize = 5;
struct Hand {
	Card cards[HandSize];
	PokerHandSet phands;
	
	Hand(Index *ip) {
		for (int i = 0; i < HandSize; ++i)
			cards[i] = Card(ip[i]);
		findPokerHands();
	}
	
	void addHand(PokerHand p) { phands.set(p); }
	
	void dump(FILE *f = stderr) const {
		CardID cid;
		cid[2] = '\0';
		for (int i = 0; i < HandSize; ++i) {
			if (i != 0) fprintf(f, ", ");
			cards[i].id(cid);
			fprintf(f, "%s", cid);
		}
		fprintf(f, ": ");
		bool first = true;
		for (size_t i = 0; i < PokerHandsMax; ++i) {
			if (phands.has(i)) {
				if (!first) fprintf(f, ", ");
				first = false;
				fprintf(f, "%s", PokerHandNames[i]);
			}
		}
		fprintf(f, "\n");
	}
	
	static Hand firstHand(Index *ip) {
		for (int i = 0; i < HandSize; ++i)
			ip[i] = i;
		return Hand(ip);
	}
	static bool nextHand(Index *ip, Hand &h) {
		Index max = IndexMax;
		int i = HandSize - 1;
		for (; ; --i) { // find first changeable
			if (i < 0) return false;
			if (ip[i] < max - 1)
				break;
			max = ip[i];
		}
		
		Index next = ++ip[i];
		for (int j = i + 1; j < HandSize; ++j)
			ip[j] = ++next;
		h = Hand(ip);
		return true;
	}
	
	void findPokerHands() {
		vector<int> nums(NumMax), suits(SuitMax), colors(ColorMax);
		for (int i = 0; i < HandSize; ++i) {
			Card &c = cards[i];
			++nums[c.num];
			++suits[c.suit];
			++colors[c.color()];
		}
		
		vector<int> snums = nums;
		sort(snums.rbegin(), snums.rend());
		if (snums[0] >= 4)
			addHand(FourKind);
		else if (snums[0] == 3)
			addHand(snums[1] == 2 ? FullHouse : ThreeKind);
		else if (snums[0] == 2)
			addHand(snums[1] == 2 ? TwoPair : Pair);
		
		bool flush = false, flush4 = false;
		for (int i = 0; i < SuitMax; ++i) {
			if (suits[i] == HandSize) flush = true;
			if (suits[i] == 4) flush4 = true;
		}
		if (flush4) addHand(Flush4);
		bool colFlush = (colors[Black] == HandSize || colors[Red] == HandSize);
		
		bool straight = false;
		int strCount = 0;
		for (Number n = 0; n <= Ace; ++n) {
			Number ni = (n == Ace) ? 0 : n;
			strCount = nums[ni] ? (strCount + 1) : 0;
			if (strCount == 5) {
				straight = true;
				break;
			}
		}
		
		if (straight) {
			if (flush) addHand(StraightFlush);
			else if (colFlush) addHand(StraightColorFlush);
			else addHand(Straight);
		} else if (flush) addHand(Flush);
		else if (colFlush) addHand(ColorFlush);
		
		if (phands.bits.none())
			addHand(HighCard);
	}
};

typedef vector<PokerHandSet> HandsList;
void allHands(HandsList &hlist) {
	Index ip[HandSize];
	Hand h = Hand::firstHand(ip);
//	Index p = ip[0];
	do {
		// if (p != ip[0]) {
		// 	fprintf(stderr, "%d\n", ip[0]);
		// 	p = ip[0];
		// }
//		h.dump(stdout);
		hlist.push_back(h.phands);
	} while (Hand::nextHand(ip, h));
}

void orderTypes() {
	HandsList hlist;
	allHands(hlist);
	
	int remain = hlist.size();
	vector<int> counts(PokerHandsMax);
	
	while (remain) {
		// count
		fill(counts.begin(), counts.end(), 0);
		for (HandsList::const_iterator s = hlist.begin(); s != hlist.end(); ++s) {
			if (s->bits.any()) {
				for (int j = 0; j < PokerHandsMax; ++j) {
					if (s->has(j))
						++counts[j];
				}
			}
		}
		
		// find best
		PokerHand bestHand = PokerHandsMax;
		int bestCount = remain + 1;
		for (size_t i = 0; i < counts.size(); ++i) {
			PokerHand h = (PokerHand)i;
			if (counts[h] && counts[h] < bestCount) {
				bestHand = h;
				bestCount = counts[h];
			}
		}
		fprintf(stderr, "%-20s: %7d\n", PokerHandNames[bestHand], bestCount);
		remain -= bestCount;
		
		// remove those hands
		for (HandsList::iterator i = hlist.begin(); i != hlist.end(); ++i) {
			if (i->has(bestHand))
				i->bits.reset();
		}
	}
}

int main(void) {
#if 0
	Index ip[HandSize] = { 0, 9, 10, 11, 12 };
	Hand h(ip);
	h.dump(stdout);
#else
	orderTypes();
#endif
	return 0;
}
