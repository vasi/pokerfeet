#include <stdio.h>

typedef int Index;
const int IndexMax = 52;
enum Suit { Club = 0, Diamond, Heart, Spade };
typedef int Number;

const char *SuitChar = "CDHS";
const char *NumChar = "A23456789TJQK";
typedef char CardID[3];

struct Card {
	Number num;
	Suit suit;
	
	Card(Index i = 0) : num(i % 13), suit((Suit)(i / 13)) { }
	
	void id(CardID c) const {
		c[0] = NumChar[num];
		c[1] = SuitChar[suit];
	}
};

const int HandSize = 5;
struct Hand {
	Card cards[HandSize];
	
	Hand(Index *ip) {
		for (int i = 0; i < HandSize; ++i)
			cards[i] = Card(ip[i]);
	}
	
	void dump(FILE *f = stderr) const {
		CardID cid;
		cid[2] = '\0';
		for (int i = 0; i < HandSize; ++i) {
			if (i != 0) fprintf(f, ", ");
			cards[i].id(cid);
			fprintf(f, "%s", cid);
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
		
		int next = ++ip[i];
		for (int j = i + 1; j < HandSize; ++j)
			ip[j] = ++next;
		h = Hand(ip);
		return true;
	}
};

int main(void) {
	Index ip[HandSize];
	Hand h = Hand::firstHand(ip);
	do {
		h.dump(stdout);
	} while (Hand::nextHand(ip, h));
	
	return 0;
}
