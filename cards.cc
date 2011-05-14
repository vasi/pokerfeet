#include <cstdio>
#include <set>
#include <vector>
#include <algorithm>
#include <bitset>
#include <cstdarg>
using namespace std;

enum Suit { Club = 0, Diamond, Heart, Spade, SuitMax, SuitNOK };
enum Color { Black = 0, Red, ColorMax };

struct SuitSet {
	bitset<SuitMax> bits;
	
	SuitSet(Suit s1, Suit s2 = SuitNOK, Suit s3 = SuitNOK, Suit s4 = SuitNOK) {
		bits.set(s1);
		if (s2 != SuitNOK) bits.set(s2);
		if (s3 != SuitNOK) bits.set(s3);
		if (s4 != SuitNOK) bits.set(s4);
	}
	SuitSet(unsigned long u) : bits(u) { }
	
	bool include(Suit s) const { return bits.test(s); }
};
const SuitSet BlackSet(Club, Spade);
const SuitSet RedSet(Diamond, Heart);
const SuitSet AllSuits((1 << SuitMax) - 1);

typedef int Index;
const Index IndexMax = 52;
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
		return RedSet.include(suit) ? Red : Black;
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

struct PokerHandSet { // Eg: "Hand x is a flush, and a two-pair"
	bitset<PokerHandsMax> bits;
	void set(PokerHand p) { bits.set(p); }
	bool has(int i) const { return bits.test(i); }
	bool has(PokerHand p) const { return has((int)p); }
};


const int HandSize = 5;

struct NumberSet { // Group cards by face value
	Card ordered[HandSize];
	vector<size_t> offs, counts;
	
	NumberSet(Card *cs) : offs(NumMax), counts(NumMax) {
		int ci = 0;
		for (Number n = 0; n < NumMax; ++n) {
			offs[n] = ci;
			for (int p = 0; p < HandSize; ++p) {
				if (cs[p].num == n) {
					++counts[n];
					ordered[ci++] = cs[p];
				}
			}
		}
	}
	
	size_t count(Number n) const { return counts[n]; }
	const Card *cards(Number n) const { return ordered + offs[n]; }
};


struct Hand {
	Card cards[HandSize];
	PokerHandSet phands;
	
	Hand(Index *ip) {
		for (int i = 0; i < HandSize; ++i)
			cards[i] = Card(ip[i]);
		findPokerHands();
	}
	
	bool is(PokerHand p) { return phands.has(p); }
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
	
	size_t straight(NumberSet &nums, SuitSet suits) {
		size_t best = 0, cur = 0;
		for (Number n = 0; n <= Ace; ++n) {
			Number nidx = (n == Ace) ? 0 : n;
			
			bool found = false;
			const Card *c = nums.cards(nidx);
			for (size_t ct = nums.count(nidx); ct > 0; --ct, ++c) {
				if (suits.include(c->suit)) {
					found = true;
					break;
				}
			}
			
			if (found)
				best = max(best, ++cur);
			else
				cur = 0;
		}
		return best;
	}
	
	void findPokerHands() {
		vector<int> suits(SuitMax), colors(ColorMax);
		for (int i = 0; i < HandSize; ++i) {
			Card &c = cards[i];
			++suits[c.suit];
			++colors[c.color()];
		}
		NumberSet nums(cards);
		
		vector<size_t> snums = nums.counts;
		sort(snums.rbegin(), snums.rend());
		if (snums[0] >= 4)
			addHand(FourKind);
		else if (snums[0] == 3)
			addHand(snums[1] == 2 ? FullHouse : ThreeKind);
		else if (snums[0] == 2)
			addHand(snums[1] == 2 ? TwoPair : Pair);
		
		bool flush = false;
		for (int i = 0; i < SuitMax; ++i) {
			if (suits[i] == HandSize) flush = true;
			if (suits[i] == 4) addHand(Flush4);
		}
		bool colFlush = (colors[Black] == HandSize || colors[Red] == HandSize);
		
		size_t str = straight(nums, AllSuits);
		for (int i = 0; i < SuitMax; ++i) {
			if (straight(nums, SuitSet((Suit)i)) == 4) {
				addHand(StraightFlush4);
				break;
			}
		}
		if (!phands.has(StraightFlush4) &&
			(straight(nums, BlackSet) == 4 || straight(nums, RedSet) == 4))
				addHand(StraightColorFlush4);
		
		if (str == 4) addHand(Straight4);		
		if (str == 5) {
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
//		if (h.is(StraightFlush4) || h.is(StraightFlush)) h.dump(stdout);
		hlist.push_back(h.phands);
	} while (Hand::nextHand(ip, h));
//exit(0);
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
		
		// find best hand: the rarest one
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
		
		// remove best hand
		for (HandsList::iterator i = hlist.begin(); i != hlist.end(); ++i) {
			if (i->has(bestHand))
				i->bits.reset();
		}
	}
}

int main(void) {
#if 0
	Index ip[HandSize] = { 0, 2, 10, 11, 12 };
	Hand h(ip);
	h.dump(stdout);
	return 0;
#endif

	orderTypes();
	return 0;
}
