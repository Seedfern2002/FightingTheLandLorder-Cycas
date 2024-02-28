#include<iostream>
#include<set>
#include<cmath>
#include<cstring>
#include<algorithm>
#include<unordered_map>
#include<string>
#include<ctime>
#include<vector>
#include "jsoncpp/json.h"
using namespace std;
constexpr int PLAYER_COUNT = 3;

enum class Stage
{
	BIDDING, 
	PLAYING	 
};

enum class CardComboType
{
	PASS,		
	SINGLE,		
	PAIR,		
	STRAIGHT,	
	STRAIGHT2,	
	TRIPLET,	
	TRIPLET1,	
	TRIPLET2,	
	BOMB,		
	QUADRUPLE2, 
	QUADRUPLE4,
	PLANE,		
	PLANE1,		
	PLANE2,		
	SSHUTTLE,	
	SSHUTTLE2,	
	SSHUTTLE4,	
	ROCKET,		
	INVALID		
};

int cardComboScores[] = {
	0,	
	1,	
	2,	
	6,	
	6,	
	4,	
	4,	
	4,	
	10, 
	8,	
	8,	
	8,	
	8,	
	8,	
	10, 
	10, 
	10, 
	16, 
	0	
};

#ifndef _BOTZONE_ONLINE
string cardComboStrings[] = {
	"PASS",
	"SINGLE",
	"PAIR",
	"STRAIGHT",
	"STRAIGHT2",
	"TRIPLET",
	"TRIPLET1",
	"TRIPLET2",
	"BOMB",
	"QUADRUPLE2",
	"QUADRUPLE4",
	"PLANE",
	"PLANE1",
	"PLANE2",
	"SSHUTTLE",
	"SSHUTTLE2",
	"SSHUTTLE4",
	"ROCKET",
	"INVALID" };
#endif
using Card = short;
using Level = short;
constexpr Level MAX_LEVEL = 15;
constexpr Level MAX_STRAIGHT_LEVEL = 11;
constexpr Level level_joker = 13;
constexpr Level level_JOKER = 14;
constexpr Card card_joker = 52;
constexpr Card card_JOKER = 53;
constexpr Level card2level(Card card)
{
	return card / 4 + card / 53;
}

struct CardPack
{
	Level level;
	short count;

	bool operator<(const CardPack& b) const
	{
		if (count == b.count)
			return level > b.level;
		return count > b.count;
	}
};

set<Card> myCards;
set<Card> landlordPublicCards;
vector<vector<Card>> whatTheyPlayed[PLAYER_COUNT];

bool eyes_open = 0;

short cardRemaining[PLAYER_COUNT] = { 17, 17, 17 };
int playertopos[PLAYER_COUNT] = { 0 };

int myPosition;

int landlordPosition = -1;

int landlordBid = -1;

Stage stage = Stage::BIDDING;

vector<int> bidInput;

short card_remain[15] = { 0 };
short a[15] = { 0 };
short played[15] = { 0 };
double MAXN[17] = { 0 };
double pos_bom = 0;

Level leveltocard(Level level)
{
	if (level <= 12 && level >= 0)
	{
		for (int i = 4 * level; i < 4 + 4 * level; i++)
		{
			if (myCards.find(i) != myCards.end())
			{
				myCards.erase(i);
				return i;
			}
		}
	}
	else if (level == 13)
	{
		myCards.erase(52);
		return 52;
	}
	else if (level == 14)
	{
		myCards.erase(53);
		return 53;
	}
}
long long CC[40][40] = { 0 };
long long C(int m, int n)
{
	if (m == 0) return 1;
	if (m < n - m) m = n - m;
	if (CC[m][n])
	{
		return CC[m][n];
	}
	long long ans = 1;
	for (int i = m + 1; i <= n; i++)
	{
		ans *= i;
		ans /= (i - m);
	}
	CC[m][n] = ans;
	return ans;
}
double possibility_single(short* b, short card, int position);
double possibility_pair(short* b, short card, int position);
double possibility_triplet(short* b, short card, int position, int type);
double possibility_bomb(short* b, short card, int position, int type);
double possibility_rocket(short* b, int position);
double possibility_straight(short* b, short card, int length, int position);
double possibility_straight2(short* b, short card, int length, int position);
double possivility_plane(short* b, short card, int length, int position, int type);

double antisingle(short* b, short card, int position);
double antipair(short* b, short card, int position);
double antitriplet(short* b, short card, int position, int type);
double antibomb(short* b, short card, int position, int type);
double antistraight(short* b, short card, int length, int position);
double antistraight2(short* b, short card, int length, int position);
double antiplane(short* b, short card, int length, int position, int type);

void strategy();
void renew_action(int position);

unordered_map<int, double>mark;
unordered_map<int, double>antipos;
unordered_map<int, double>pos;
class cardcombotype {
public:
	double value;
	double possb;
	double antipossb;
	int number;
	string name;
	cardcombotype() {
		antipossb = 0.0;
		possb = 0.0;
	}
	virtual ~cardcombotype() {}
	virtual short find_card() {
		return -1;
	}
	virtual int find_length() {
		return -1;
	}
	virtual vector<short> find_remain()
	{
		vector<short> so;
		return so;
	}
	virtual short find_another()
	{
		return -1;
	}
	virtual double getvalue() = 0;
};
class Single :public cardcombotype {
public:
	short card;
	Single(short _card, double v = 0) :card(_card) {
		value = v;
		name = "SINGLE";
		number = 1;
		if(eyes_open) getvalue();
	}
	~Single() {}
	double getvalue()
	{
		if (mark[card])
		{
			value = mark[card];
			antipossb = antipos[card];
			possb = pos[card];
			return mark[card];
		}
		int flag = 0;
		if (MAXN[1] == 0) flag = 1;
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possibility_single(card_remain, card, p1) + possibility_single(card_remain, card, p2);
			antipossb = antisingle(card_remain, card, p1) + antisingle(card_remain, card, p2);
			if (flag)
			{
				MAXN[1] = antisingle(card_remain, 14, p1) + antisingle(card_remain, 14, p2);
				if (MAXN[1] == 0) MAXN[1] = 0.00001;
			}
		}
		else
		{
			possb = possibility_single(card_remain, card, landlordPosition);
			antipossb = antisingle(card_remain, card, landlordPosition);
			if (flag)
			{
				MAXN[1] = antisingle(card_remain, 14, landlordPosition);
				if (MAXN[1] == 0) MAXN[1] = 0.00001;
			}
		}
		value = log(possb + 1)/ exp(2*antipossb/MAXN[1]);
		antipos[card] = antipossb;
		mark[card] = value;
		pos[card] = possb;
		return value;
	}
	short find_card()
	{
		return card;
	}
	operator double() {
		return value;
	}
};
class Pair :public cardcombotype {
public:
	short two;
	Pair(short _card, double v = 0) :two(_card) {
		value = v;
		name = "PAIR";
		number = 2;
		if (eyes_open) getvalue();
	}
	~Pair() {}
	double getvalue()
	{
		if (mark[two + 15])
		{
			antipossb = antipos[two + 15];
			value = mark[two + 15];
			possb = pos[two + 15];
			return value;
		}
		int flag = 0;
		if (MAXN[2] == 0) flag = 1;
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possibility_pair(card_remain, two, p1) + possibility_pair(card_remain, two, p2);
			antipossb = antipair(card_remain, two, p1) + antipair(card_remain, two, p2);
			antipossb += (antisingle(card_remain, two, p1) + antisingle(card_remain, two, p2));
			if (flag)
			{
				MAXN[2]+= antipair(card_remain, 12, p1) + antipair(card_remain, 12, p2);
				MAXN[2]+=(antisingle(card_remain, 12, p1) + antisingle(card_remain, 12, p2));
				if (MAXN[2] == 0) MAXN[2] = 0.00001;
			}
		}
		else
		{
			possb = possibility_pair(card_remain, two, landlordPosition);
			antipossb = antipair(card_remain, two, landlordPosition);
			antipossb += antisingle(card_remain, two, landlordPosition);
			if (flag)
			{
				MAXN[2] += antipair(card_remain, 12, landlordPosition);
				MAXN[2] += antisingle(card_remain, 12, landlordPosition);
				if (MAXN[2] == 0) MAXN[2] = 0.00001;
			}
		}
		value = log(possb + 1) / exp(2*antipossb/MAXN[2]);
		antipos[two + 15] = antipossb;
		mark[two + 15] = value;
		pos[two + 15] = possb;
		return value;
	}
	short find_card()
	{
		return two;
	}
	operator double()
	{
		return value;
	}
};
class Straight :public cardcombotype {
public:
	short start;
	int length;
	Straight(short _start, int _length, double v = 0) :start(_start), length(_length)
	{
		value = v;
		name = "STRAIGHT";
		number = 3;
		if (eyes_open) getvalue();
	}
	~Straight() {}
	double getvalue()
	{
		if (mark[1000 + start * 100 + length])
		{
			antipossb = antipos[1000 + start * 100 + length];
			value = mark[1000 + start * 100 + length];
			possb = pos[1000 + start * 100 + length];
			return value;
		}
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possibility_straight(card_remain, start, length, p1) + possibility_straight(card_remain, start, length, p2);
			antipossb = antistraight(card_remain, start, length, p1) + antistraight(card_remain, start, length, p2);
		}
		else
		{
			possb = possibility_straight(card_remain, start, length, landlordPosition);
			antipossb = antistraight(card_remain, start, length, landlordPosition);
		}
		value = log(possb + 1) / exp(antipossb+1);
		antipos[1000 + start * 100 + length] = antipossb;
		mark[1000 + start * 100 + length] = value;
		pos[1000 + start * 100 + length] = possb;
		return value;
	}
	short find_card()
	{
		return start;
	}
	int find_length()
	{
		return length;
	}
	operator double()
	{
		return value;
	}
};
class Straight2 :public cardcombotype {
public:
	short start;
	int length;
	Straight2(short _s, int _l, double v = 0) :start(_s), length(_l)
	{
		value = v;
		name = "STRAIGHT2";
		number = 4;
		if (eyes_open) getvalue();
	}
	~Straight2() {}
	double getvalue()
	{
		if (mark[3000 + start * 100 + length])
		{
			antipossb = antipos[3000 + start * 100 + length];
			value = mark[3000 + start * 100 + length];
			possb = pos[3000 + start * 100 + length];
			return value;
		}
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possibility_straight2(card_remain, start, length, p1) + possibility_straight2(card_remain, start, length, p2);
			antipossb = antistraight2(card_remain, start, length, p1) + antistraight2(card_remain, start, length, p2);
			for (int j = 0; j < length; j++)
			{
				if (start + length >= 10) antipossb += (antipair(card_remain, start + j, p1) + antipair(card_remain, start + j, p2));
			}
		}
		else
		{
			possb = possibility_straight2(card_remain, start, length, landlordPosition);
			antipossb = antistraight2(card_remain, start, length, landlordPosition);
			for (int j = 0; j < length; j++)
			{
				if (start + length >= 10) antipossb += antipair(card_remain, start + j, landlordPosition);
			}
		}
		value = log(possb + 1) / exp(antipossb+1);
		antipos[3000 + start * 100 + length] = antipossb;
		mark[3000 + start * 100 + length] = value;
		pos[3000 + start * 100 + length] = possb;
		return value;
	}
	short find_card()
	{
		return start;
	}
	int find_length()
	{
		return length;
	}
	operator double()
	{
		return value;
	}
};
class Triplet :public cardcombotype {
public:
	short three;
	Triplet(short _c, double v = 0) :three(_c)
	{
		value = v;
		name = "TRIPLET";
		number = 5;
		if (eyes_open) getvalue();
	}
	~Triplet() {}
	double getvalue()
	{
		if (mark[three + 30])
		{
			antipossb = antipos[three + 30];
			value = mark[three + 30];
			possb = pos[three + 30];
			return value;
		}
		int flag = 0;
		double at = 0;
		if (MAXN[5] == 0) flag = 1;
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possibility_triplet(card_remain, three, p1, 0) + possibility_triplet(card_remain, three, p2, 0);
			antipossb = antitriplet(card_remain, three, p1, 0) + antitriplet(card_remain, three, p2, 0);
			at = antipossb;
			if (flag)
			{
				MAXN[5] = antitriplet(card_remain, 12, p1, 0) + antitriplet(card_remain, 12, p2, 0);
				if (MAXN[5] == 0) MAXN[5] = 0.00001;
			}
			if (three >= 10)
			{
				antipossb += (antipair(card_remain, three, p1) + antipair(card_remain, three, p2));
				antipossb += (antisingle(card_remain, three, p1) + antisingle(card_remain, three, p2));
			}
		}
		else
		{
			possb = possibility_triplet(card_remain, three, landlordPosition, 0);
			antipossb = antitriplet(card_remain, three, landlordPosition, 0);
			at = antipossb;
			if (flag)
			{
				MAXN[5] = antitriplet(card_remain, 12, landlordPosition, 0);
				if (MAXN[5] == 0) MAXN[5] = 0.00001;
			}
			if (three >= 10)
			{
				antipossb += antipair(card_remain, three, landlordPosition);
				antipossb += antisingle(card_remain, three, landlordPosition);
			}
		}
		value = log(possb + 1) / exp(at/MAXN[5]);
		antipos[three + 30] = antipossb;
		mark[three + 30] = value;
		pos[three + 30] = possb;
		return value;
	}
	short find_card()
	{
		return three;
	}
	operator double()
	{
		return value;
	}
};
class Triplet1 :public cardcombotype {
public:
	short three, one;
	Triplet1(short _t, short _o, double v = 0) :three(_t), one(_o)
	{
		value = v;
		name = "TRIPLET1";
		number = 6;
		if (eyes_open) getvalue();
	}
	~Triplet1() {}
	double getvalue()
	{
		if (mark[three + 45])
		{
			antipossb = antipos[three + 45];
			value = mark[three + 45];
			possb = pos[three + 45];
			return value;
		}
		int flag = 0;
		double at = 0;
		if (MAXN[6] == 0) flag = 1;
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possibility_triplet(card_remain, three, p1, 1) + possibility_triplet(card_remain, three, p2, 1);
			antipossb = antitriplet(card_remain, three, p1, 1) + antitriplet(card_remain, three, p2, 1);
			at = antipossb;
			if (flag)
			{
				MAXN[6] = antitriplet(card_remain, 12, p1, 1) + antitriplet(card_remain, 12, p2, 1);
				if (MAXN[6] == 0) MAXN[6] = 0.00001;
			}
			if (three >= 10)
			{
				antipossb += (antipair(card_remain, three, p1) + antipair(card_remain, three, p2));
				antipossb += (antisingle(card_remain, three, p1) + antisingle(card_remain, three, p2));
			}
		}
		else
		{
			possb = possibility_triplet(card_remain, three, landlordPosition, 1);
			antipossb = antitriplet(card_remain, three, landlordPosition, 1);
			at = antipossb;
			if (flag)
			{
				MAXN[6] = antitriplet(card_remain, 12, landlordPosition, 1);
				if (MAXN[6] == 0) MAXN[6] = 0.00001;
			}
			if (three >= 10)
			{
				antipossb += antipair(card_remain, three, landlordPosition);
				antipossb += antisingle(card_remain, three, landlordPosition);
			}
		}
		value = log(possb + 1) / exp(at/MAXN[6]);
		antipos[three + 45] = antipossb;
		mark[three + 45] = value;
		pos[three + 45] = possb;
		return value;
	}
	short find_card()
	{
		return three;
	}
	short find_another()
	{
		return one;
	}
	operator double()
	{
		return value;
	}
};
class Triplet2 :public cardcombotype {
public:
	short three, two;
	Triplet2(short _th, short _tw, double v = 0) :three(_th), two(_tw)
	{
		value = v;
		name = "TRIPLET2";
		number = 7;
		if (eyes_open) getvalue();
	}
	~Triplet2() {}
	double getvalue()
	{
		if (mark[three + 60])
		{
			antipossb = antipos[three + 60];
			value = mark[three + 60];
			possb = pos[three + 60];
			return value;
		}
		int flag = 0;
		double at = 0;
		if (MAXN[7] == 0) flag = 1;
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possibility_triplet(card_remain, three, p1, 2) + possibility_triplet(card_remain, three, p2, 2);
			antipossb = antitriplet(card_remain, three, p1, 2) + antitriplet(card_remain, three, p2, 2);
			at = antipossb;
			if (flag)
			{
				MAXN[7] = antitriplet(card_remain, 12, p1, 2) + antitriplet(card_remain, 12, p2, 2);
				if (MAXN[7] == 0) MAXN[7] = 0.00001;
			}
			if (three >= 10)
			{
				antipossb += (antipair(card_remain, three, p1) + antipair(card_remain, three, p2));
				antipossb += (antisingle(card_remain, three, p1) + antisingle(card_remain, three, p2));
			}
		}
		else
		{
			possb = possibility_triplet(card_remain, three, landlordPosition, 2);
			antipossb = antitriplet(card_remain, three, landlordPosition, 2);
			at = antipossb;
			if (flag)
			{
				MAXN[7] = antitriplet(card_remain, 12, landlordPosition, 2);
				if (MAXN[7] == 0) MAXN[7] = 0.00001;
			}
			if (three >= 10)
			{
				antipossb += antipair(card_remain, three, landlordPosition);
				antipossb += antisingle(card_remain, three, landlordPosition);
			}
		}
		value = log(possb + 1) / exp(at/MAXN[7]);
		antipos[three + 60] = antipossb;
		mark[three + 60] = value;
		pos[three + 60] = possb;
		return value;
	}
	short find_card()
	{
		return three;
	}
	short find_another()
	{
		return two;
	}
	operator double()
	{
		return value;
	}
};
class Bomb :public cardcombotype {
public:
	short four;
	Bomb(short _f, double v = 0) :four(_f)
	{
		value = v;
		name = "BOMB";
		number = 8;
	}
	~Bomb() {}
	double getvalue()
	{
		if (mark[four + 75])
		{
			possb = pos[four + 75];
			antipossb = antipos[four + 75];
			value = mark[four + 75];
			return value;
		}
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possibility_bomb(card_remain, four, p1, 0) + possibility_bomb(card_remain, four, p2, 0);
			antipossb = antibomb(card_remain, four, p1, 0) + antibomb(card_remain, four, p2, 0);
		}
		else
		{
			possb = possibility_bomb(card_remain, four, landlordPosition, 0);
			antipossb = antibomb(card_remain, four, landlordPosition, 0);
		}
		value =-0.19+ log(possb + 1) / (antipossb + 1);
		pos[four + 75] = possb;
		antipos[four + 75] = antipossb;
		mark[four + 75] = value;
		return value;
	}
	short find_card()
	{
		return four;
	}
	operator double()
	{
		return value;
	}
};
class Quadruple2 :public cardcombotype {
public:
	short four;
	vector<short> single;
	Quadruple2(short _f, vector<short> vec, double v = 0) :four(_f)
	{
		value = v;
		for (short c : vec)
			single.push_back(c);
		name = "QUADRUPLE2";
		number = 9;
	}
	~Quadruple2() {}
	double getvalue()
	{
		if (mark[four + 90])
		{
			possb = pos[four + 90];
			antipossb = antipos[four + 90];
			value = mark[four + 90];
			return value;
		}
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possibility_bomb(card_remain, four, p1, 1) + possibility_bomb(card_remain, four, p2, 1);
			antipossb = antibomb(card_remain, four, p1, 1) + antibomb(card_remain, four, p2, 1);
		}
		else
		{
			possb = possibility_bomb(card_remain, four, landlordPosition, 1);
			antipossb = antibomb(card_remain, four, landlordPosition, 1);
		}
		value = log(possb + 1) / (antipossb + 1);
		pos[four + 90] = possb;
		antipos[four + 90] = antipossb;
		mark[four + 90] = value;
		return value;
	}
	short find_card()
	{
		return four;
	}
	vector<short> find_remain()
	{
		return single;
	}
	operator double()
	{
		return value;
	}
};
class Quadruple4 :public cardcombotype
{
public:
	short four;
	vector<short> pair;
	Quadruple4(short _f, vector<short>vec, double v = 0) :four(_f)
	{
		value = v;
		for (short c : vec)
			pair.push_back(c);
		name = "QUADRUPLE4";
		number = 10;
	}
	~Quadruple4() {}
	double getvalue()
	{
		if (mark[four + 105])
		{
			possb = pos[four + 105];
			antipossb = antipos[four + 105];
			value = mark[four + 105];
			return value;
		}
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possibility_bomb(card_remain, four, p1, 2) + possibility_bomb(card_remain, four, p2, 2);
			antipossb = antibomb(card_remain, four, p1, 2) + antibomb(card_remain, four, p2, 2);
		}
		else
		{
			possb = possibility_bomb(card_remain, four, landlordPosition, 2);
			antipossb = antibomb(card_remain, four, landlordPosition, 2);
		}
		value = log(possb + 1) / (antipossb + 1);
		pos[four + 105] = possb;
		antipos[four + 105] = antipossb;
		mark[four + 105] = value;
		return value;
	}
	short find_card()
	{
		return four;
	}
	vector<short> find_remain()
	{
		return pair;
	}
	operator double()
	{
		return value;
	}
};
class Plane :public cardcombotype {
public:
	short three;
	int length;
	Plane(short _t, int _l, double v = 0) :three(_t), length(_l)
	{
		value = v;
		name = { "PLANE" };
		number = 11;
		if (eyes_open) getvalue();
	}
	~Plane() {}
	double getvalue()
	{
		if (mark[5000 + three * 100 + length])
		{
			possb = pos[5000 + three * 100 + length];
			antipossb = antipos[5000 + three * 100 + length];
			value = mark[5000 + three * 100 + length];
			return value;
		}
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possivility_plane(card_remain, three, length, p1, 0) + possivility_plane(card_remain, three, length, p2, 0);
			antipossb = antiplane(card_remain, three, length, p1, 0) + antiplane(card_remain, three, length, p2, 0);
			for (int j = 0; j < length; j++)
			{
				antipossb += (antitriplet(card_remain, three + j, p1, 0) + antitriplet(card_remain, three + j, p2, 0));
				/*if (three + j >= 10)
				{
					antipossb += (antipair(card_remain, three + j, p1) + antipair(card_remain, three + j, p2));
					antipossb += (antisingle(card_remain, three + j, p1) + antisingle(card_remain, three + j, p2));
				}*/
			}
		}
		else
		{
			possb = possivility_plane(card_remain, three, length, landlordPosition, 0);
			antipossb = antiplane(card_remain, three, length, landlordPosition, 0);
			for (int j = 0; j < length; j++)
			{
				antipossb += antitriplet(card_remain, three + j, landlordPosition, 0);
				/*if (three + j >= 10)
				{
					antipossb += antisingle(card_remain, three + j, landlordPosition);
					antipossb += antipair(card_remain, three + j, landlordPosition);
				}*/
			}
		}
		value = log(possb + 1) / (antipossb + 1);
		mark[5000 + three * 100 + length] = value;
		antipos[5000 + three * 100 + length] = antipossb;
		pos[5000 + three * 100 + length] = possb;
		return value;
	}
	short find_card()
	{
		return three;
	}
	int find_length()
	{
		return length;
	}
	operator double()
	{
		return value;
	}
};
class Plane1 :public cardcombotype {
public:
	short three;
	int length;
	vector<short>single;
	Plane1(short _t, int _l, vector<short>vec, double v = 0) :three(_t), length(_l)
	{
		value = v;
		for (short c : vec)
			single.push_back(c);
		name = "PLANE1";
		number = 12;
		if (eyes_open) getvalue();
	}
	~Plane1() {}
	double getvalue()
	{
		if (mark[5000 + three * 100 + length + 20])
		{
			antipossb = antipos[5000 + three * 100 + length + 20];
			value = mark[5000 + three * 100 + length + 20];
			possb = pos[5000 + three * 100 + length + 20];
			return value;
		}
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possivility_plane(card_remain, three, length, p1, 1) + possivility_plane(card_remain, three, length, p2, 1);
			antipossb = antiplane(card_remain, three, length, p1, 1) + antiplane(card_remain, three, length, p2, 1);
			for (int j = 0; j < length; j++)
			{
				antipossb += (antitriplet(card_remain, three + j, p1, 1) + antitriplet(card_remain, three + j, p2, 1));
				/*if (three + j >= 10)
				{
					antipossb += (antipair(card_remain, three + j, p1) + antipair(card_remain, three + j, p2));
					antipossb += (antisingle(card_remain, three + j, p1) + antisingle(card_remain, three + j, p2));
				}*/
			}
		}
		else
		{
			possb = possivility_plane(card_remain, three, length, landlordPosition, 1);
			antipossb = antiplane(card_remain, three, length, landlordPosition, 1);
			for (int j = 0; j < length; j++)
			{
				antipossb += antitriplet(card_remain, three + j, landlordPosition, 1);
				/*if (three + j >= 10)
				{
					antipossb += antisingle(card_remain, three + j, landlordPosition);
					antipossb += antipair(card_remain, three + j, landlordPosition);
				}*/
			}
		}
		value = log(possb + 1) / (antipossb + 1);
		antipos[5000 + three * 100 + length + 20] = antipossb;
		mark[5000 + three * 100 + length + 20] = value;
		pos[5000 + three * 100 + length + 20] = possb;
		return value;
	}
	short find_card()
	{
		return three;
	}
	int find_length()
	{
		return length;
	}
	vector<short> find_remain()
	{
		return single;
	}
	operator double()
	{
		return value;
	}
};
class Plane2 :public cardcombotype {
public:
	short three;
	int length;
	vector<short> pair;
	Plane2(short _t, int _l, vector<short>vec, double v = 0) :three(_t), length(_l)
	{
		value = v;
		for (short c : vec)
			pair.push_back(c);
		name = "PLANE2";
		number = 13;
		if (eyes_open) getvalue();
	}
	~Plane2() {}
	double getvalue()
	{
		if (mark[5000 + three * 100 + length + 40])
		{
			antipossb = antipos[5000 + three * 100 + length + 40];
			value = mark[5000 + three * 100 + length + 40];
			possb = pos[5000 + three * 100 + length + 40];
			return value;
		}
		if (myPosition == landlordPosition || stage == Stage::BIDDING)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			possb = possivility_plane(card_remain, three, length, p1, 2) + possivility_plane(card_remain, three, length, p2, 2);
			antipossb = antiplane(card_remain, three, length, p1, 2) + antiplane(card_remain, three, length, p2, 2);
			for (int j = 0; j < length; j++)
			{
				antipossb += (antitriplet(card_remain, three + j, p1, 2) + antitriplet(card_remain, three + j, p2, 2));
				/*if (three + j >= 10)
				{
					antipossb += (antipair(card_remain, three + j, p1) + antipair(card_remain, three + j, p2));
					antipossb += (antisingle(card_remain, three + j, p1) + antisingle(card_remain, three + j, p2));
				}*/
			}
		}
		else
		{
			possb = possivility_plane(card_remain, three, length, landlordPosition, 2);
			antipossb = antiplane(card_remain, three, length, landlordPosition, 2);
			for (int j = 0; j < length; j++)
			{
				antipossb += antitriplet(card_remain, three + j, landlordPosition, 2);
				/*if (three + j >= 10)
				{
					antipossb += antisingle(card_remain, three + j, landlordPosition);
					antipossb += antipair(card_remain, three + j, landlordPosition);
				}*/
			}
		}
		value = log(possb + 1) / (antipossb + 1);
		antipos[5000 + three * 100 + length + 40] = antipossb;
		mark[5000 + three * 100 + length + 40] = value;
		pos[5000 + three * 100 + length + 40] = possb;
		return value;
	}
	short find_card()
	{
		return three;
	}
	int find_length()
	{
		return length;
	}
	vector<short> find_remain()
	{
		return pair;
	}
	operator double()
	{
		return value;
	}
};
class Sshuttle :public cardcombotype {
public:
	short four;
	int length;
	Sshuttle(short _f, int _l, double v = 0) :four(_f), length(_l)
	{
		value = v;
		name = "SSHUTTLE";
		number = 14;
	}
	~Sshuttle() {}
	double getvalue()
	{
		possb = 0.0;
		value = 0.0;
		return value;
	}
	short find_card()
	{
		return four;
	}
	int find_length()
	{
		return length;
	}
	operator double()
	{
		return value;
	}
};
class Sshuttle2 :public cardcombotype {
public:
	short four;
	int length;
	vector<short>single;
	Sshuttle2(short _f, int _l, vector<short>vec, double v = 0) :four(_f), length(_l)
	{
		value = v;
		for (short c : vec)
			single.push_back(c);
		name = "SSHUTTLE2";
		number = 15;
	}
	~Sshuttle2() {}
	double getvalue()
	{
		possb = 0.0;
		value = 0.0;
		return value;
	}
	short find_card()
	{
		return four;
	}
	int find_length()
	{
		return length;
	}
	vector<short> find_remain()
	{
		return single;
	}
	operator double()
	{
		return value;
	}
};
class Sshuttle4 :public cardcombotype {
public:
	short four;
	int length;
	vector<short>pair;
	Sshuttle4(short _f, int _l, vector<short>vec, double v = 0) :four(_f), length(_l)
	{
		value = v;
		for (short c : vec)
			pair.push_back(c);
		name = "SSHUTTLE4";
		number = 16;
	}
	~Sshuttle4() {}
	double getvalue()
	{
		possb = 0.0;
		value = 0.0;
		return value;
	}
	short find_card()
	{
		return four;
	}
	int find_length()
	{
		return length;
	}
	vector<short> find_remain()
	{
		return pair;
	}
	operator double()
	{
		return value;
	}
};
class Rocket :public cardcombotype {
public:
	short joker, JOKER;
	Rocket() :joker(13), JOKER(14)
	{
		value = -0.19;
		name = "ROCKET";
		number = 17;
	}
	~Rocket() {}
	double getvalue()
	{
		possb = 0.0;
		value = -0.19;
		return value;
	}
	operator double()
	{
		return value;
	}
};
class Pass :public cardcombotype {
public:
	Pass()
	{
		name = "PASS";
		number = 0;
	}
	~Pass() {}
	double getvalue()
	{
		value = 0.0;
		return value;
	}
};
typedef cardcombotype* pcards;
bool cmp(pcards p1, pcards p2)
{
	return p1->getvalue() > p2->getvalue();
}
class cardstack {
public:
	vector<pcards> st[20];
	vector<pcards> vc;
	int num[20];
	double mark;
	double mark_renew;
	cardstack()
	{
		mark = 0.0;
	}
	void pop()
	{
		if (vc.empty()) return;
		pcards p = vc.back();
		int i = p->number;
		vc.pop_back();
		st[i].pop_back();
		return;
	}
	int findMaxSeq(vector<CardPack> packs)const 
	{
		for (unsigned c = 1; c < packs.size(); c++)
			if (packs[c].count != packs[0].count ||
				packs[c].level != packs[c - 1].level - 1)
				return c;
		return packs.size();
	}
	void push_back(pcards p)
	{
		if (p)
		{
			st[p->number].push_back(p);
			vc.push_back(p);
		}
	}
	void push_back(vector<Card> vec)
	{
		vector<Card>::iterator begin = vec.begin();
		vector<Card>::iterator end = vec.end();
		Level comboLevel = 0;
		if (vec.empty())
		{
			cardcombotype* p = new Pass();
			st[0].push_back(p);
			vc.push_back(p);
			return;
		}

		short counts[MAX_LEVEL + 1] = {};

		short countOfCount[5] = {};

		vector<CardPack> packs;
		for (Card c : vec)
			counts[card2level(c)]++;
		for (Level l = 0; l <= MAX_LEVEL; l++)
			if (counts[l])
			{
				packs.push_back(CardPack{ l, counts[l] });
				countOfCount[counts[l]]++;
			}
		sort(packs.begin(), packs.end());

		comboLevel = packs[0].level;

		vector<int> kindOfCountOfCount;
		for (int i = 0; i <= 4; i++)
			if (countOfCount[i])
				kindOfCountOfCount.push_back(i);
		sort(kindOfCountOfCount.begin(), kindOfCountOfCount.end());

		int curr, lesser;

		switch (kindOfCountOfCount.size())
		{
		case 1:
			curr = countOfCount[kindOfCountOfCount[0]];
			switch (kindOfCountOfCount[0])
			{
			case 1:
				if (curr == 1)
				{
					cardcombotype* p = new Single(packs[0].level);
					st[1].push_back(p);
					vc.push_back(p);
					return;
				}
				if (curr == 2 && packs[1].level == level_joker)
				{
					cardcombotype* p = new Rocket();
					st[17].push_back(p);
					vc.push_back(p);
					return;
				}
				if (curr >= 5 && findMaxSeq(packs) == curr &&
					packs.begin()->level <= MAX_STRAIGHT_LEVEL)
				{
					cardcombotype* p = new Straight(packs.back().level, packs.size());
					st[3].push_back(p);
					vc.push_back(p);
					return;
				}
				break;
			case 2:
				if (curr == 1)
				{
					cardcombotype* p = new Pair(packs[0].level);
					st[2].push_back(p);
					vc.push_back(p);
					return;
				}
				if (curr >= 3 && findMaxSeq(packs) == curr &&
					packs.begin()->level <= MAX_STRAIGHT_LEVEL)
				{
					cardcombotype* p = new Straight2(packs.back().level, packs.size());
					st[4].push_back(p);
					vc.push_back(p);
					return;
				}
				break;
			case 3:
				if (curr == 1)
				{
					cardcombotype* p = new Triplet(packs[0].level);
					st[5].push_back(p);
					vc.push_back(p);
					return;
				}
				if (findMaxSeq(packs) == curr &&
					packs.begin()->level <= MAX_STRAIGHT_LEVEL)
				{
					cardcombotype* p = new Plane(packs.back().level, packs.size());
					st[11].push_back(p);
					vc.push_back(p);
					return;
				}
				break;
			case 4:
				if (curr == 1)
				{
					cardcombotype* p = new Bomb(packs[0].level);
					st[8].push_back(p);
					vc.push_back(p);
					return;
				}
				if (findMaxSeq(packs) == curr &&
					packs.begin()->level <= MAX_STRAIGHT_LEVEL)
				{
					cardcombotype* p = new Sshuttle(packs.back().level, packs.size());
					st[14].push_back(p);
					vc.push_back(p);
					return;
				}
			}
			break;
		case 2: 
			curr = countOfCount[kindOfCountOfCount[1]];
			lesser = countOfCount[kindOfCountOfCount[0]];
			if (kindOfCountOfCount[1] == 3)
			{
				if (kindOfCountOfCount[0] == 1)
				{
					if (curr == 1 && lesser == 1)
					{
						cardcombotype* p = new Triplet1(packs[0].level, packs[1].level);
						st[6].push_back(p);
						vc.push_back(p);
						return;
					}
					if (findMaxSeq(packs) == curr && lesser == curr &&
						packs.begin()->level <= MAX_STRAIGHT_LEVEL)
					{
						vector<short>single;
						for (short c : vec)
						{
							int flag = 1;
							int ans = card2level(c);
							for (int i = 0; i < curr; i++)
							{
								if (ans == packs[i].level)
								{
									flag = 0;
								}
							}
							if (flag) single.push_back(ans);
						}
						cardcombotype* p = new Plane1(packs[curr - 1].level, curr, single);
						st[11].push_back(p);
						vc.push_back(p);
						return;
					}
				}
				if (kindOfCountOfCount[0] == 2)
				{
					if (curr == 1 && lesser == 1)
					{
						cardcombotype* p = new Triplet2(packs[0].level, packs[1].level);
						st[7].push_back(p);
						vc.push_back(p);
						return;
					}
					if (findMaxSeq(packs) == curr && lesser == curr &&
						packs.begin()->level <= MAX_STRAIGHT_LEVEL)
					{
						vector<short>pair;
						for (short c : vec)
						{
							int flag = 1;
							int ans = card2level(c);
							for (int i = 0; i < curr; i++)
							{
								if (ans == packs[i].level)
								{
									flag = 0;
								}
							}
							if (flag) pair.push_back(ans);
						}
						cardcombotype* p = new Plane2(packs[curr - 1].level, curr, pair);
						st[12].push_back(p);
						vc.push_back(p);
						return;
					}
				}
			}
			if (kindOfCountOfCount[1] == 4)
			{
				if (kindOfCountOfCount[0] == 1)
				{
					if (curr == 1 && lesser == 2)
					{
						vector<short>single;
						single.push_back(packs[1].level);
						single.push_back(packs[2].level);
						cardcombotype* p = new Quadruple2(packs[0].level, single);
						st[9].push_back(p);
						vc.push_back(p);
						return;
					}
					if (findMaxSeq(packs) == curr && lesser == curr * 2 &&
						packs.begin()->level <= MAX_STRAIGHT_LEVEL)
					{
						vector<short>pair;
						for (short c : vec)
						{
							int flag = 1;
							int ans = card2level(c);
							for (int i = 0; i < curr; i++)
							{
								if (ans == packs[i].level)
								{
									flag = 0;
								}
							}
							if (flag) pair.push_back(ans);
						}
						cardcombotype* p = new Sshuttle2(packs[0].level - curr + 1, curr, pair);
						st[15].push_back(p);
						vc.push_back(p);
						return;
					}
				}
				if (kindOfCountOfCount[0] == 2)
				{
					if (curr == 1 && lesser == 2)
					{
						vector<short>pair;
						pair.push_back(packs[1].level);
						pair.push_back(packs[2].level);
						cardcombotype* p = new Quadruple4(packs[0].level, pair);
						st[10].push_back(p);
						vc.push_back(p);
						return;
					}
					if (findMaxSeq(packs) == curr && lesser == curr * 2 &&
						packs.begin()->level <= MAX_STRAIGHT_LEVEL)
					{
						vector<short>pair;
						for (short c : vec)
						{
							int flag = 1;
							int ans = card2level(c);
							for (int i = 0; i < curr; i++)
							{
								if (ans == packs[i].level)
								{
									flag = 0;
								}
							}
							if (flag) pair.push_back(ans);
						}
						cardcombotype* p = new Sshuttle4(packs[0].level - curr + 1, curr, pair);
						st[16].push_back(p);
						vc.push_back(p);
						return;
					}
				}
			}
		}

	}
	double renew_mark()
	{
		double value = 0;
		int offensive = 0;
		if (!st[8].empty())
		{
			offensive += st[8].size();
		}
		if (!st[17].empty())
		{
			offensive += st[17].size();
		}
		int vvva = 0;
		for (pcards p : vc)
		{
			if (p->getvalue() > 0.01) vvva++;
			if (vvva-offensive >= 2) break;
		}
		if (vvva-offensive <= 1&&pos_bom<0.005) offensive += 1;
		vector<double>vect;
		double mark2[17];
		for (int i = 1; i <= 17; i++)
		{
			if (i==3||i==4||i>=8)
			{
				for (pcards p : st[i])
				{
					vect.push_back(p->value);
					value += p->value;	
				}
			}
			int size = st[i].size();
			value += size * 0.10;
			int half = size / 2;
			if (size % 2 == 0)
			{
				for (int j = 0; j < half; j++)
				{
					double vvv=min(st[i][j]->value, st[i][j + half]->value * 2);
					vect.push_back(vvv);
					value += vvv;
				}
				for (int j = half; j < size; j++)
				{
					vect.push_back(st[i][j]->value);
					value += st[i][j]->value;
				}
			}
			else if (size % 2 == 1)
			{
				if (size == 1)
				{
					value += st[i][0]->value;
					vect.push_back(st[i][0]->value);
				}
				else {
					for (int j = 0; j < half; j++)
					{
						double vvv = min(st[i][j]->value, st[i][j + half+1]->value * 2);
						vect.push_back(vvv);
						value += vvv;
					}
					for (int j = half + 1; j < size; j++)
					{
						vect.push_back(st[i][j]->value);
						value += st[i][j]->value;
					}
					vect.push_back(st[i][half]->value);
					value += st[i][half]->value;
				}
			}
		}
		sort(vect.begin(), vect.end(),greater<int>());
		for (double b : vect)
		{
			if (offensive)
			{
				value -= b;
				offensive--;
			}
		}
		/*for (pcards p : vect)
		{
			if (mark2[p->number] > 0)
			{
				if (offensive)
				{
					offensive--;
					mark2[p->number] -= min(mark2[p->number], p->getvalue());
					value -= min(mark2[p->number], p->getvalue());
				}
			}
		}*/
		mark_renew = value;
		return value;
	}
	cardstack& operator=(const cardstack& cd)
	{
		/*mark = cd.mark;
		if (!vc.empty())
		{
			for (pcards p : vc)
			{
				delete p;
			}
		}*/
		vc.clear();
		mark_renew = cd.mark_renew;
		mark = cd.mark;
		for (int i = 0; i <= 17; i++)
		{
			/*if (!st[i].empty())
			{
				for (pcards p : st[i])
				{
					delete p;
				}
			}*/
			st[i].clear();
			for (pcards c : cd.st[i])
			{
				pcards p = NULL;
				switch (i)
				{
				case 1:
					p = new Single(c->find_card(), c->value);
					break;
				case 2:
					p = new Pair(c->find_card(), c->value);
					break;
				case 3:
					p = new Straight(c->find_card(), c->find_length(), c->value);
					break;
				case 4:
					p = new Straight2(c->find_card(), c->find_length(), c->value);
					break;
				case 5:
					p = new Triplet(c->find_card(), c->value);
					break;
				case 6:
					p = new Triplet1(c->find_card(), c->find_another(), c->value);
					break;
				case 7:
					p = new Triplet2(c->find_card(), c->find_another(), c->value);
					break;
				case 8:
					p = new Bomb(c->find_card(), c->value);
					break;
				case 9:
					p = new Quadruple2(c->find_card(), c->find_remain(), c->value);
					break;
				case 10:
					p = new Quadruple4(c->find_card(), c->find_remain(), c->value);
					break;
				case 11:
					p = new Plane(c->find_card(), c->find_length(), c->value);
					break;
				case 12:
					p = new Plane1(c->find_card(), c->find_length(), c->find_remain(), c->value);
					break;
				case 13:
					p = new Plane2(c->find_card(), c->find_length(), c->find_remain(), c->value);
					break;
				case 14:
					p = new Sshuttle(c->find_card(), c->find_length(), c->value);
					break;
				case 15:
					p = new Sshuttle2(c->find_card(), c->find_length(), c->find_remain(), c->value);
					break;
				case 16:
					p = new Sshuttle4(c->find_card(), c->find_length(), c->find_remain(), c->value);
					break;
				case 0:
					p = new Pass();
					break;
				case 17:
					p = new Rocket();
					break;
				default:
					break;
				}
				if (p != NULL)
				{
					st[i].push_back(p);
					vc.push_back(p);
				}
			}
		}
		return *this;
	}
};
int findMaxSeq(vector<CardPack> packs)
{
	for (unsigned c = 1; c < packs.size(); c++)
		if (packs[c].count != packs[0].count ||
			packs[c].level != packs[c - 1].level - 1)
			return c;
	return packs.size();
}
pcards judge_type(vector<Card> vec)
{
	vector<Card>::iterator begin = vec.begin();
	vector<Card>::iterator end = vec.end();
	Level comboLevel = 0;
	if (begin == end)
	{
		cardcombotype* p = new Pass();
		return p;
	}

	short counts[MAX_LEVEL + 1] = {};

	short countOfCount[5] = {};

	vector<CardPack> packs;
	for (Card c : vec)
		counts[card2level(c)]++;
	for (Level l = 0; l <= MAX_LEVEL; l++)
		if (counts[l])
		{
			packs.push_back(CardPack{ l, counts[l] });
			countOfCount[counts[l]]++;
		}
	sort(packs.begin(), packs.end());

	comboLevel = packs[0].level;

	vector<int> kindOfCountOfCount;
	for (int i = 0; i <= 4; i++)
		if (countOfCount[i])
			kindOfCountOfCount.push_back(i);
	sort(kindOfCountOfCount.begin(), kindOfCountOfCount.end());

	int curr, lesser;

	switch (kindOfCountOfCount.size())
	{
	case 1:
		curr = countOfCount[kindOfCountOfCount[0]];
		switch (kindOfCountOfCount[0])
		{
		case 1:
			if (curr == 1)
			{
				cardcombotype* p = new Single(packs[0].level);
				return p;
			}
			if (curr == 2 && packs[1].level == level_joker)
			{
				cardcombotype* p = new Rocket();
				return p;
			}
			if (curr >= 5 && findMaxSeq(packs) == curr &&
				packs.begin()->level <= MAX_STRAIGHT_LEVEL)
			{
				cardcombotype* p = new Straight(packs.back().level, packs.size());
				return p;
			}
			break;
		case 2:
			if (curr == 1)
			{
				cardcombotype* p = new Pair(packs[0].level);
				return p;
			}
			if (curr >= 3 && findMaxSeq(packs) == curr &&
				packs.begin()->level <= MAX_STRAIGHT_LEVEL)
			{
				cardcombotype* p = new Straight2(packs.back().level, packs.size());
				return p;
			}
			break;
		case 3:
			if (curr == 1)
			{
				cardcombotype* p = new Triplet(packs[0].level);
				return p;
			}
			if (findMaxSeq(packs) == curr &&
				packs.begin()->level <= MAX_STRAIGHT_LEVEL)
			{
				cardcombotype* p = new Plane(packs.back().level, packs.size());
				return p;
			}
			break;
		case 4:
			if (curr == 1)
			{
				cardcombotype* p = new Bomb(packs[0].level);
				return p;
			}
			if (findMaxSeq(packs) == curr &&
				packs.begin()->level <= MAX_STRAIGHT_LEVEL)
			{
				cardcombotype* p = new Sshuttle(packs.back().level, packs.size());
				return p;
			}
		}
		break;
	case 2: 
		curr = countOfCount[kindOfCountOfCount[1]];
		lesser = countOfCount[kindOfCountOfCount[0]];
		if (kindOfCountOfCount[1] == 3)
		{
			if (kindOfCountOfCount[0] == 1)
			{
				if (curr == 1 && lesser == 1)
				{
					cardcombotype* p = new Triplet1(packs[0].level, packs[1].level);
					return p;
				}
				if (findMaxSeq(packs) == curr && lesser == curr &&
					packs.begin()->level <= MAX_STRAIGHT_LEVEL)
				{
					vector<short>single;
					for (short c : vec)
					{
						int flag = 1;
						int ans = card2level(c);
						for (int i = 0; i < curr; i++)
						{
							if (ans == packs[i].level)
							{
								flag = 0;
							}
						}
						if (flag) single.push_back(ans);
					}
					cardcombotype* p = new Plane1(packs[curr - 1].level, curr, single);
					return p;
				}
			}
			if (kindOfCountOfCount[0] == 2)
			{
				if (curr == 1 && lesser == 1)
				{
					cardcombotype* p = new Triplet2(packs[0].level, packs[1].level);
					return p;
				}
				if (findMaxSeq(packs) == curr && lesser == curr &&
					packs.begin()->level <= MAX_STRAIGHT_LEVEL)
				{
					vector<short>pair;
					for (short c : vec)
					{
						int flag = 1;
						int ans = card2level(c);
						for (int i = 0; i < curr; i++)
						{
							if (ans == packs[i].level)
							{
								flag = 0;
							}
						}
						if (flag) pair.push_back(ans);
					}
					cardcombotype* p = new Plane2(packs[curr - 1].level, curr, pair);
					return p;
				}
			}
		}
		if (kindOfCountOfCount[1] == 4)
		{
			if (kindOfCountOfCount[0] == 1)
			{
				if (curr == 1 && lesser == 2)
				{
					vector<short>single;
					single.push_back(packs[1].level);
					single.push_back(packs[2].level);
					cardcombotype* p = new Quadruple2(packs[0].level, single);
					return p;
				}
				if (findMaxSeq(packs) == curr && lesser == curr * 2 &&
					packs.begin()->level <= MAX_STRAIGHT_LEVEL)
				{
					vector<short>pair;
					for (short c : vec)
					{
						int flag = 1;
						int ans = card2level(c);
						for (int i = 0; i < curr; i++)
						{
							if (ans == packs[i].level)
							{
								flag = 0;
							}
						}
						if (flag) pair.push_back(ans);
					}
					cardcombotype* p = new Sshuttle2(packs[0].level - curr + 1, curr, pair);
					return p;
				}
			}
			if (kindOfCountOfCount[0] == 2)
			{
				if (curr == 1 && lesser == 2)
				{
					vector<short>pair;
					pair.push_back(packs[1].level);
					pair.push_back(packs[2].level);
					cardcombotype* p = new Quadruple4(packs[0].level, pair);
					return p;
				}
				if (findMaxSeq(packs) == curr && lesser == curr * 2 &&
					packs.begin()->level <= MAX_STRAIGHT_LEVEL)
				{
					vector<short>pair;
					for (short c : vec)
					{
						int flag = 1;
						int ans = card2level(c);
						for (int i = 0; i < curr; i++)
						{
							if (ans == packs[i].level)
							{
								flag = 0;
							}
						}
						if (flag) pair.push_back(ans);
					}
					cardcombotype* p = new Sshuttle4(packs[0].level - curr + 1, curr, pair);
					return p;
				}
			}
		}
	}
}
void ptoa(cardcombotype* p);



bool check_straight(short* b, int start, int size)
{
	int flag = 1;
	for (int i = 1; i <= size; i++)
	{
		if (start + size > 12)
		{
			return false;
		}
		if (b[start + i] == 0)
		{
			start = start + i;
			i = 0;
			continue;
		}
	}
	return true;
}
short* remain;

cardstack whatplayed[PLAYER_COUNT];

cardstack whattheypassed[PLAYER_COUNT];

cardcombotype* lastcombo = NULL;
bool landpush = 0;
bool nobody_push = 1;
int loadpos = -1;

cardstack mystrategy;
double min_value = 10000.0;

double renew_value = 10000.0;

cardstack median;
vector<Card>myaction;

namespace BotzoneIO
{
	using namespace std;
	void read()
	{
		string line;
		getline(cin, line);
		Json::Value input;
		Json::Reader reader;
		reader.parse(line, input);

		{
			auto firstRequest = input["requests"][0u]; 
			auto own = firstRequest["own"];
			for (unsigned i = 0; i < own.size(); i++)
			{
				myCards.insert(own[i].asInt());

				a[card2level(own[i].asInt())]++;
			}
			if (!firstRequest["bid"].isNull())
			{
				auto bidHistory = firstRequest["bid"];
				myPosition = bidHistory.size();
				for (unsigned i = 0; i < bidHistory.size(); i++)
					bidInput.push_back(bidHistory[i].asInt());
			}
		}

		int whoInHistory[] = { (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT, (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT };

		int turn = input["requests"].size();
		for (int i = 0; i < turn; i++)
		{
			auto request = input["requests"][i];
			auto llpublic = request["publiccard"];
			if (!llpublic.isNull())
			{
				landlordPosition = request["landlord"].asInt();
				landlordBid = request["finalbid"].asInt();
				myPosition = request["pos"].asInt();
				if ((myPosition - landlordPosition + 3) % 3 == 2)
				{
					playertopos[whoInHistory[0]] = landlordPosition;
					playertopos[whoInHistory[1]] = (landlordPosition + 1) % 3;
					loadpos = whoInHistory[0];
				}
				else if ((myPosition - landlordPosition + 3) % 3 == 1)
				{
					playertopos[whoInHistory[1]] = landlordPosition;
					playertopos[whoInHistory[0]] = (landlordPosition + 2) % 3;
					loadpos = whoInHistory[1];
				}
				else {
					playertopos[whoInHistory[1]] = whoInHistory[1];
					playertopos[whoInHistory[0]] = whoInHistory[0];
				}
				cardRemaining[landlordPosition] += llpublic.size();
				for (unsigned i = 0; i < llpublic.size(); i++)
				{
					landlordPublicCards.insert(llpublic[i].asInt());
					if (landlordPosition == myPosition)
					{
						myCards.insert(llpublic[i].asInt());

						a[card2level(llpublic[i].asInt())]++;
					}
				}
			}

			auto history = request["history"]; 
			if (history.isNull())
				continue;
			stage = Stage::PLAYING;

			int howManyPass = 0;
			for (int p = 0; p < 2; p++)
			{
				int player = whoInHistory[p];
				auto playerAction = history[p];
				vector<Card> playedCards;
				for (unsigned _ = 0; _ < playerAction.size(); _++) 
				{
					int card = playerAction[_].asInt(); 

					played[card2level(card)]++;

					playedCards.push_back(card);
				}
				whatTheyPlayed[playertopos[player]].push_back(playedCards); 

				whatplayed[playertopos[player]].push_back(playedCards);

				cardRemaining[playertopos[player]] -= playerAction.size();

				if (playerAction.size() == 0)
				{
					whattheypassed[playertopos[player]].push_back(lastcombo);
					howManyPass++;
				}
				else
				{
					lastcombo = judge_type(playedCards);
					nobody_push = 0;
					if (player == loadpos)
					{
						landpush = true;
					}
					else
					{
						landpush = false;
					}
				}
			}

			if (howManyPass == 2)
			{
				lastcombo = new Pass();
				nobody_push = 1;
			}

			if (i < turn - 1)
			{
				auto playerAction = input["responses"][i]; 
				vector<Card> playedCards;
				for (unsigned _ = 0; _ < playerAction.size(); _++) 
				{
					int card = playerAction[_].asInt(); 

					played[card2level(card)]++;
					a[card2level(card)]--;

					myCards.erase(card);			
					playedCards.push_back(card);
				}
				whatTheyPlayed[myPosition].push_back(playedCards); 

				whatplayed[myPosition].push_back(playedCards);

				cardRemaining[myPosition] -= playerAction.size();
			}
		}
	}

	void bid(int value)
	{
		Json::Value result;
		result["response"] = value;

		Json::FastWriter writer;
		cout << writer.write(result) << endl;
	}

	template <typename CARD_ITERATOR>
	void play(CARD_ITERATOR begin, CARD_ITERATOR end)
	{
		Json::Value result, response(Json::arrayValue);
		for (; begin != end; begin++)
			response.append(*begin);
		result["response"] = response;

		Json::FastWriter writer;
		cout << writer.write(result) << endl;
	}
}


clock_t time1;
clock_t time2;
bool no_time = 0;

int main()
{
	BotzoneIO::read();
	eyes_open = 1;

	time1 = clock();
	if (stage == Stage::BIDDING)
	{

		auto maxBidIt = std::max_element(bidInput.begin(), bidInput.end());
		int maxBid = maxBidIt == bidInput.end() ? -1 : *maxBidIt;

		for (int i = 0; i <= 12; i++)
		{
			card_remain[i] = 4 - played[i] - a[i];
		}
		for (int i = 13; i <= 14; i++)
		{
			card_remain[i] = 1 - played[i] - a[i];
		}
		int bidValue = 0;
		strategy();
		mystrategy;

		if (renew_value < 2.9) bidValue = 3;
		else if (renew_value >= 2.9 && renew_value < 3.4) bidValue = 2;
		else if (renew_value >= 3.4 && renew_value < 4.0) bidValue = 1;
		else bidValue = 0;

		if (bidValue <= maxBid)
		{
			bidValue = 0;
		}

		BotzoneIO::bid(bidValue);
	}
	else if (stage == Stage::PLAYING)
	{
		for (int i = 0; i <= 12; i++)
		{
			card_remain[i] = 4 - played[i] - a[i];
		}
		for (int i = 13; i <= 14; i++)
		{
			card_remain[i] = 1 - played[i] - a[i];
		}

		if (myPosition == landlordPosition)
		{
			int p1 = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
			int p2 = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
			pos_bom = possibility_bomb(card_remain, -1, p1, 0) + possibility_bomb(card_remain, -1, p2, 0);
			short num_per1 = cardRemaining[p1];
			short num_per2 = cardRemaining[p2];
			short num_all = num_per1 + num_per2;
			if (card_remain[13] && card_remain[14])
			{
				pos_bom += ((double)C(num_per1 - 2, num_all - 2) / C(num_per1, num_all));
				pos_bom += ((double)C(num_per2 - 2, num_all - 2) / C(num_per2, num_all));
			}
		}
		else
		{
			pos_bom = possibility_bomb(card_remain, -1, landlordPosition, 0);
			short another_pos;
			for (int i = 0; i <= 2; i++)
			{
				if (i != landlordPosition && i != myPosition)
					another_pos = i;
			}
			short num_per = cardRemaining[landlordPosition];
			short num_all = cardRemaining[landlordPosition] + cardRemaining[another_pos];
			if (card_remain[13] && card_remain[14])
				pos_bom += ((double)C(num_per - 2, num_all - 2) / C(num_per, num_all));
		}
		renew_action(myPosition);
		BotzoneIO::play(myaction.begin(), myaction.end());
	}
	system("pause");
	return 0;
}
double possibility_single(short* b, short card, int position)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	short min_cannot = 14;
	if (!whattheypassed[position].st[1].empty())
	{
		for (pcards c : whattheypassed[position].st[1])
		{
			if (c->find_card() < min_cannot)
				min_cannot = c->find_card();
		}
	}
	if (myPosition == landlordPosition)
	{
		for (int i = card + 1; i <= 14; i++)
		{
			ans += ((double)1 - (double)C(num_per, num_all - b[i]) / C(num_per, num_all));
		}
	}
	else {

		for (int i = card + 1; i <= min_cannot; i++)
		{
			ans += ((double)1 - (double)C(num_per, num_all - b[i]) / C(num_per, num_all));
		}
		for (int i = max(card + 1, (int)min_cannot); i <= 12; i++)
		{
			if (b[i] == 1) ans += 0.36787944*((double)C(num_per - 1, num_all - 1) / C(num_per, num_all));
			else
			{
				ans += 0.36787944 * ((double)C(num_per - 1, num_all - 1) / C(num_per, num_all));
				if (b[i] == 2)
				{
					ans += ((double)C(num_per - 2, num_all - 2) / C(num_per, num_all));
				}
				else if (b[i] == 3)
				{
					ans += ((double)(C(2, 3) * C(num_per - 2, num_all - 3)) / C(num_per, num_all) + (double)C(num_per - 3, num_all - 3) / C(num_per, num_all));
				}
				else if (b[i] == 4)
				{
					ans += ((double)(C(2, 4) * C(num_per - 2, num_all - 4)) / C(num_per, num_all)
						+ (double)(C(3, 4) * C(num_per - 3, num_all - 4)) / C(num_per, num_all)
						+ (double)C(num_per - 4, num_all - 4) / C(num_per, num_all));
				}
			}
		}
	}
	return ans;
}
double possibility_pair(short* b, short card, int position)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	short min_cannot = 12;
	if (!whattheypassed[position].st[2].empty())
	{
		for (pcards c : whattheypassed[position].st[2])
		{
			if (c->find_card() < min_cannot)
				min_cannot = c->find_card();
		}
	}
	if (myPosition == landlordPosition)
	{
		for (int i = card + 1; i <= 12; i++)
		{
			if (b[i] <= 1) ans += 0.0;
			else
			{
				if (b[i] == 2)
				{
					ans += ((double)C(num_per - 2, num_all - 2) / C(num_per, num_all));
				}
				else if (b[i] == 3)
				{
					ans += ((double)(C(2, 3) * C(num_per - 2, num_all - 3)) / C(num_per, num_all) + (double)C(num_per - 3, num_all - 3) / C(num_per, num_all));
				}
				else if (b[i] == 4)
				{
					ans += ((double)(C(2, 4) * C(num_per - 2, num_all - 4)) / C(num_per, num_all)
						+ (double)(C(3, 4) * C(num_per - 3, num_all - 4)) / C(num_per, num_all)
						+ (double)C(num_per - 4, num_all - 4) / C(num_per, num_all));
				}
			}
		}
	}
	else
	{
		for (int i = card + 1; i <= min_cannot; i++)
		{
			if (b[i] <= 1) ans += 0.0;
			else
			{
				if (b[i] == 2)
				{
					ans += ((double)C(num_per - 2, num_all - 2) / C(num_per, num_all));
				}
				else if (b[i] == 3)
				{
					ans += ((double)(C(2, 3) * C(num_per - 2, num_all - 3)) / C(num_per, num_all) + (double)C(num_per - 3, num_all - 3) / C(num_per, num_all));
				}
				else if (b[i] == 4)
				{
					ans += ((double)(C(2, 4) * C(num_per - 2, num_all - 4)) / C(num_per, num_all)
						+ (double)(C(3, 4) * C(num_per - 3, num_all - 4)) / C(num_per, num_all)
						+ (double)C(num_per - 4, num_all - 4) / C(num_per, num_all));
				}
			}
		}
	}
	return ans;
}
double possibility_triplet(short* b, short card, int position, int type)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	if (num_per < 3 + type) return 0.0;
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	short min_cannot = 12;
	if (!whattheypassed[position].st[5 + type].empty())
	{
		for (pcards c : whattheypassed[position].st[5 + type])
		{
			if (c->find_card() < min_cannot)
				min_cannot = c->find_card();
		}
	}
	for (int i = card + 1; i <= min_cannot; i++)
	{
		if (b[i] <= 2) ans += 0.0;
		if (b[i] == 3)
		{
			ans += (double)C(num_per - 3, num_all - 3) / C(num_per, num_all);
		}
		else if (b[i] == 4)
		{
			ans += ((double)C(3, 4) * C(num_per - 3, num_all - 4) / C(num_per, num_all)
				+ (double)C(num_per - 4, num_all - 4) / C(num_per, num_all));
		}
	}
	return ans;
}
double possibility_bomb(short* b, short card, int position, int type)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	if (num_per < 4 + type * 2) return 0.0;
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	short min_cannot = 12;
	if (!whattheypassed[position].st[8 + type].empty())
	{
		for (pcards c : whattheypassed[position].st[8 + type])
		{
			if (c->find_card() < min_cannot)
				min_cannot = c->find_card();
		}
	}
	for (int i = card + 1; i <= min_cannot; i++)
	{
		if (b[i] <= 3) ans += 0.0;
		else if (b[i] == 4)
		{
			ans += (double)C(num_per - 4, num_all - 4) / C(num_per, num_all);
		}
	}
	return ans;
}
double possibility_rocket(short* b, int position)
{
	return 0.0;
}
double possibility_straight(short* b, short card, int length, int position)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	if (num_per < length) return 0.0;
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	short min_cannot = 12 - length;
	if (!whattheypassed[position].st[3].empty())
	{
		for (pcards c : whattheypassed[position].st[3])
		{
			if (c->find_length() == length)
			{
				if (c->find_card() < min_cannot)
					min_cannot = c->find_card();
			}
		}
	}
	for (int j = card + 1; j <= min_cannot; j++)
	{
		int num = 0;
		bool flag = 0;
		double ans2 = 1.0;
		for (int i = j; i < j + length; i++)
		{
			num += b[i];
			if (b[i] == 0)
			{
				flag = 1;
				j = i;
				break;
			}
		}
		if (flag) continue;
		for (int i = j; i < j + length; i++)
		{
			ans2 *= ((double)1 - (double)C(num_per, num_all - b[i]) / C(num_per, num_all));
		}
		ans += ans2;
	}
	return ans;
}
double possibility_straight2(short* b, short card, int length, int position)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	if (num_per < length * 2) return 0.0;
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	short min_cannot = 12 - length;
	if (!whattheypassed[position].st[2].empty())
	{
		for (pcards c : whattheypassed[position].st[2])
		{
			if (c->find_card() < min_cannot)
				min_cannot = c->find_card();
		}
	}

	if (!whattheypassed[position].st[4].empty())
	{
		for (pcards c : whattheypassed[position].st[4])
		{
			if (c->find_length() == length)
			{
				if (c->find_card() < min_cannot)
					min_cannot = c->find_card();
			}
		}
	}
	for (int j = card + 1; j <= min_cannot; j++)
	{
		double ans2 = 1.0;
		bool flag = 0;
		int num = 0;
		for (int i = j; i < j + length; i++)
		{
			num += b[i];
			if (b[i] < 2)
			{
				flag = 1;
				j = i;
				break;
			}
		}
		if (flag) continue;
		for (int i = j; i < j + length; i++)
		{
			if (b[i] == 2)
			{
				ans2 *= 1;
			}
			else if (b[i] == 3)
			{
				ans2 *= 4;
			}
			else if (b[i] == 4)
			{
				ans2 *= 11;
			}
		}
		ans2 *= (double)C(num_per - length * 2, num_all - num);
		ans2 /= (double)C(num_per, num_all);
		ans += ans2;
	}
	return ans;
}
double possivility_plane(short* b, short card, int length, int position, int type)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	if (length > 2) return 0.0;
	if (num_per < length * 3 + type) return 0.0;
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	short min_cannot = 12 - length;
	if (!whattheypassed[position].st[11 + type].empty())
	{
		for (pcards c : whattheypassed[position].st[11 + type])
		{
			if (c->find_length() == length)
			{
				if (c->find_card() < min_cannot)
					min_cannot = c->find_card();
			}
		}
	}
	if (!whattheypassed[position].st[5 + type].empty())
	{
		for (pcards c : whattheypassed[position].st[5 + type])
		{
			if (c->find_card() < min_cannot)
				min_cannot = c->find_card();
		}
	}
	for (int i = card + 1; i <= min_cannot; i++)
	{
		double ans2 = 0.0;
		int num = 0;
		if (b[i] < 3 || b[i + 1] < 3)
		{
			continue;
		}
		if (b[i] == 3 && b[i + 1] == 3)
		{
			ans2 += (double)C(6, num_per) / C(6, num_all);
		}
		if (b[i] == 3 && b[i + 1] == 4)
		{
			ans2 += (double)C(3, 4) * C(num_per - 6, num_all - 7) / C(num_per, num_all);
		}
		if (b[i] == 4 && b[i + 1] == 3)
		{
			ans2 += (double)C(3, 4) * C(num_per - 6, num_all - 7) / C(num_per, num_all);
		}
		if (b[i] == 4 && b[i + 1] == 4)
		{
			ans2 += (double)C(3, 4) * C(3, 4) * C(num_per - 6, num_all - 8) / C(num_per, num_all);
		}
		ans += ans2;
	}
	return ans;
}
double antisingle(short* b, short card, int position)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	if (myPosition == landlordPosition)
	{
		for (int i = 0; i <= card - 1; i++)
		{
			ans += ((double)1 - (double)C(num_per, num_all - b[i]) / C(num_per, num_all));
		}
	}
	else {

		for (int i = 0; i <= card - 1; i++)
		{
			ans += ((double)1 - (double)C(num_per, num_all - b[i]) / C(num_per, num_all));
		}
	}
	return ans;
}
double antipair(short* b, short card, int position)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	if (myPosition == landlordPosition)
	{
		for (int i = 0; i <= card - 1; i++)
		{
			if (b[i] <= 1) ans += 0.0;
			else
			{
				if (b[i] == 2)
				{
					ans += ((double)C(num_per - 2, num_all - 2) / C(num_per, num_all));
				}
				else if (b[i] == 3)
				{
					ans += ((double)(C(2, 3) * C(num_per - 2, num_all - 3)) / C(num_per, num_all) + (double)C(num_per - 3, num_all - 3) / C(num_per, num_all));
				}
				else if (b[i] == 4)
				{
					ans += ((double)(C(2, 4) * C(num_per - 2, num_all - 4)) / C(num_per, num_all)
						+ (double)(C(3, 4) * C(num_per - 3, num_all - 4)) / C(num_per, num_all)
						+ (double)C(num_per - 4, num_all - 4) / C(num_per, num_all));
				}
			}
		}
	}
	else
	{
		for (int i = 0; i <= card - 1; i++)
		{
			if (b[i] <= 1) ans += 0.0;
			else
			{
				if (b[i] == 2)
				{
					ans += ((double)C(num_per - 2, num_all - 2) / C(num_per, num_all));
				}
				else if (b[i] == 3)
				{
					ans += ((double)(C(2, 3) * C(num_per - 2, num_all - 3)) / C(num_per, num_all) + (double)C(num_per - 3, num_all - 3) / C(num_per, num_all));
				}
				else if (b[i] == 4)
				{
					ans += ((double)(C(2, 4) * C(num_per - 2, num_all - 4)) / C(num_per, num_all)
						+ (double)(C(3, 4) * C(num_per - 3, num_all - 4)) / C(num_per, num_all)
						+ (double)C(num_per - 4, num_all - 4) / C(num_per, num_all));
				}
			}
		}
	}
	return ans;
}
double antitriplet(short* b, short card, int position, int type)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	if (num_per < 3 + type) return 0.0;
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	for (int i = 0; i <= card - 1; i++)
	{
		if (b[i] <= 2) ans += 0.0;
		if (b[i] == 3)
		{
			ans += (double)C(num_per - 3, num_all - 3) / C(num_per, num_all);
		}
		else if (b[i] == 4)
		{
			ans += ((double)C(3, 4) * C(num_per - 3, num_all - 4) / C(num_per, num_all)
				+ (double)C(num_per - 4, num_all - 4) / C(num_per, num_all));
		}
	}
	return ans;
}
double antibomb(short* b, short card, int position, int type)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	if (num_per < 4 + type) return 0.0;
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	for (int i = 0; i <= card - 1; i++)
	{
		if (b[i] <= 3) ans += 0.0;
		if (b[i] == 4)
		{
			ans += (double)C(num_per - 4, num_all - 4) / C(num_per, num_all);
		}

	}
	return ans;
}
double antistraight(short* b, short card, int length, int position)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	if (num_per < length) return 0.0;
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	for (int j = 0; j <= card - 1; j++)
	{
		int num = 0;
		bool flag = 0;
		double ans2 = 1.0;
		for (int i = j; i < j + length; i++)
		{
			num += b[i];
			if (b[i] == 0)
			{
				flag = 1;
				j = i;
				break;
			}
		}
		if (flag) continue;
		for (int i = j; i < j + length; i++)
		{
			ans2 *= ((double)1 - (double)C(num_per, num_all - b[i]) / C(num_per, num_all));
		}
		ans += ans2;
	}
	return ans;
}
double antistraight2(short* b, short card, int length, int position)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	if (num_per < length * 2) return 0.0;
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	for (int j = 0; j <= card - 1; j++)
	{
		double ans2 = 1.0;
		bool flag = 0;
		int num = 0;
		for (int i = j; i < j + length; i++)
		{
			num += b[i];
			if (b[i] < 2)
			{
				flag = 1;
				j = i;
				break;
			}
		}
		if (flag) continue;
		for (int i = j; i < j + length; i++)
		{
			if (b[i] == 2)
			{
				ans2 *= 1;
			}
			else if (b[i] == 3)
			{
				ans2 *= 4;
			}
			else if (b[i] == 4)
			{
				ans2 *= 11;
			}
		}
		ans2 *= (double)C(num_per - length * 2, num_all - num);
		ans2 /= (double)C(num_per, num_all);
		ans += ans2;
	}
	return ans;
}
double antiplane(short* b, short card, int length, int position, int type)
{
	double ans = 0.0;
	short num_per = cardRemaining[position];
	if (length > 2) return 0.0;
	if (num_per < length * 3 + type) return 0.0;
	short another_pos;
	for (int i = 0; i <= 2; i++)
	{
		if (i != position && i != myPosition)
			another_pos = i;
	}
	short num_all = cardRemaining[another_pos] + num_per;
	for (int i = 0; i <= card - 1; i++)
	{
		double ans2 = 0.0;
		int num = 0;
		if (b[i] < 3 || b[i + 1] < 3)
		{
			continue;
		}
		if (b[i] == 3 && b[i + 1] == 3)
		{
			ans2 += (double)C(6, num_per) / C(6, num_all);
		}
		if (b[i] == 3 && b[i + 1] == 4)
		{
			ans2 += (double)C(3, 4) * C(num_per - 6, num_all - 7) / C(num_per, num_all);
		}
		if (b[i] == 4 && b[i + 1] == 3)
		{
			ans2 += (double)C(3, 4) * C(num_per - 6, num_all - 7) / C(num_per, num_all);
		}
		if (b[i] == 4 && b[i + 1] == 4)
		{
			ans2 += (double)C(3, 4) * C(3, 4) * C(num_per - 6, num_all - 8) / C(num_per, num_all);
		}
		ans += ans2;
	}
	return ans;
}

void strategy()
{

	time2 = clock();
	if (time2 - time1 > 959000)
	{
		no_time = 1;
		return;
	}
	int sum = 0;
	bool only_single = true;
	for (int i = 0; i <= 14; i++)
	{
		sum += a[i];
	}
	if (sum == 0)
	{
		median.renew_mark();
		double mark = median.mark_renew;
		if (min_value > median.mark)
		{
			min_value = median.mark;
		}
		if (renew_value > mark)
		{
			renew_value = mark;
			mystrategy = median;
			min_value = median.mark;
		}
		return;
	}
	if (a[13] && a[14])
	{
		a[13]--;
		a[14]--;
		cardcombotype* p = new Rocket();
		only_single = false;
		median.push_back(p);
		double value = p->getvalue();
		median.mark += value;
		if (median.mark < min_value && !no_time)
			strategy();
		median.mark -= value;
		median.pop();
		delete p;
		a[13]++;
		a[14]++;
	}
	for (int i = 0; i <= 7; i++)
	{
		if (sum < 5) break;
		int length = 0;
		for (int j = i; j <= 11; j++)
		{
			if (a[j])
			{
				length++;
			}
			else if (a[j] == 0)
			{
				length = 0;
				i = j;
				break;
			}
			if (length >= 5)
			{
				only_single = false;
				for (int k = j; k >= j - length + 1; k--)
				{
					a[k]--;
				}
				cardcombotype* p = new Straight(j - length + 1, length);
				median.push_back(p);
				double value = p->getvalue();
				median.mark += value;
				if (median.mark < min_value && !no_time)
					strategy();
				median.mark -= value;
				median.pop();
				delete p;
				for (int k = j; k >= j - length + 1; k--)
				{
					a[k]++;
				}
			}
		}
	}
	for (int i = 0; i <= 9; i++)
	{
		if (sum < 6) break;
		int length = 0;
		for (int j = i; j <= 11; j++)
		{
			if (a[j] >= 2)
			{
				length++;
			}
			else if (a[j] < 2)
			{
				length = 0;
				i = j;
				break;
			}
			if (length >= 3)
			{
				only_single = false;
				for (int k = j; k >= j - length + 1; k--)
				{
					a[k] -= 2;
				}
				cardcombotype* p = new Straight2(j - length + 1, length);
				median.push_back(p);
				double value = p->getvalue();
				median.mark += value;
				if (median.mark < min_value && !no_time)
					strategy();
				median;
				median.mark -= value;
				median.pop();
				delete p;
				for (int k = j; k >= j - length + 1; k--)
				{
					a[k] += 2;
				}
			}
		}
	}
	for (int i = 0; i <= 10; i++)
	{
		if (sum < 6) break;
		int length = 0;
		for (int j = i; j <= 11; j++)
		{
			if (a[j] >= 3)
			{
				length++;
			}
			else if (a[j] < 3)
			{
				length = 0;
				i = j;
				break;
			}
			if (length >= 2)
			{
				only_single = false;
				for (int k = j; k >= j - length + 1; k--)
				{
					a[k] -= 3;
				}

				cardcombotype* p = new Plane(j - length + 1, length);
				median.push_back(p);
				double value = p->getvalue();
				median.mark += value;
				if (median.mark < min_value && !no_time)
					strategy();
				median.mark -= value;
				median.pop();
				delete p;

				bool ans_got = 0;
				vector<short> vec;
				if (length == 2)
				{
					for (int k = 0; k <= 13; k++)
					{
						if (sum < 8) break;
						if (k <= j && k >= j - length + 1)
						{
							continue;
						}
						if (a[k])
						{
							a[k]--;
							vec.push_back(k);
							for (int l = k + 1; l <= 14; l++)
							{
								if (l <= j && l >= j - length + 1)
								{
									continue;
								}
								if (a[l])
								{
									a[l]--;
									vec.push_back((short)l);
									cardcombotype* p = new Plane1(j - length + 1, length, vec);
									ans_got = 1;									median.push_back(p);
									double value = p->getvalue();
									median.mark += value;
									if (median.mark < min_value && !no_time)
										strategy();
									median.mark -= value;
									median.pop();
									delete p;
									vec.pop_back();
									a[l]++;
									break;
								}
							}
							vec.pop_back();
							a[k]++;
							if (ans_got) break;
						}
					}
				}
				else if (length == 3)
				{
					for (int k = 0; k <= 14; k++)
					{
						if (sum < 12) break;
						if (k <= j && k >= j - length + 1)
						{
							continue;
						}
						if (a[k])
						{
							a[k]--;
							vec.push_back(k);
							for (int l = k + 1; l <= 14; l++)
							{
								if (l <= j && l >= j - length + 1)
								{
									continue;
								}
								if (a[l])
								{
									a[l]--;
									vec.push_back(l);
									for (int m = l + 1; m <= 14; m++)
									{
										if (m <= j && m >= j - length + 1)
										{
											continue;
										}
										if (a[m])
										{
											a[m]--;
											vec.push_back(m);
											cardcombotype* p = new Plane1(j - length + 1, length, vec);
											ans_got = 1;
											median.push_back(p);
											double value = p->getvalue();
											median.mark += value;
											if (median.mark < min_value && !no_time)
												strategy();
											median.mark -= value;
											median.pop();
											delete p;
											vec.pop_back();
											a[m]++;
											if (ans_got) break;
										}
									}
									vec.pop_back();
									a[l]++;
									if (ans_got) break;
								}
							}
							vec.pop_back();
							a[k]++;
							if (ans_got) break;
						}
					}
				}
				else if (length == 4)
				{
					for (int k = 0; k <= 14; k++)
					{
						if (sum < 16) break;
						if (k <= j && k >= j - length + 1)
						{
							continue;
						}
						if (a[k])
						{
							a[k]--;
							vec.push_back(k);
							for (int l = k + 1; l <= 14; l++)
							{
								if (l <= j && l >= j - length + 1)
								{
									continue;
								}
								if (a[l])
								{
									a[l]--;
									vec.push_back(l);
									for (int m = l + 1; m <= 14; m++)
									{
										if (m <= j && m >= j - length + 1)
										{
											continue;
										}
										if (a[m])
										{
											a[m]--;
											vec.push_back(m);
											for (int n = m + 1; n <= 14; n++)
											{
												if (n <= j && m >= j - length + 1)
												{
													continue;
												}
												if (a[n])
												{
													a[n]--;
													vec.push_back(n);
													cardcombotype* p = new Plane1(j - length + 1, length, vec);
													ans_got = 1;
													median.push_back(p);
													double value = p->getvalue();
													median.mark += value;
													if (median.mark < min_value && !no_time)
														strategy();
													median.mark -= value;
													median.pop();
													delete p;
													vec.pop_back();
													a[n]++;
													if (ans_got) break;
												}
											}
											vec.pop_back();
											a[m]++;
											if (ans_got) break;
										}
									}
									vec.pop_back();
									a[l]++;
									if (ans_got) break;
								}
							}
							vec.pop_back();
							a[k]++;
							if (ans_got) break;
						}
					}
				}
				else if (length == 5)
				{
					if (sum >= 20)
					{
						int used[5];
						int num = 0;
						for (int k = 0; k <= 14; k++)
						{
							if (k <= j && k >= j - length + 1)
							{
								continue;
							}
							if (a[k] == 1)
							{
								used[num++] = k;
								a[k]--;
								vec.push_back(k);
							}
						}
						if (vec.size() == 5)
						{
							cardcombotype* p = new Plane1(j - length + 1, length, vec);
							median.push_back(p);
							double value = p->getvalue();
							median.mark += value;
							if (median.mark < min_value && !no_time)
								strategy();
							median.mark -= value;
							median.pop();
							delete p;
						}
						for (int k = 0; k < num; k++)
						{
							a[used[k]]++;
						}
						vec.clear();
					}
				}

				ans_got = 0;
				if (length == 2)
				{
					for (int k = 0; k <= 14; k++)
					{
						if (sum < 10) break;
						if (k <= j && k >= j - length + 1)
						{
							continue;
						}
						if (a[k] >= 2)
						{
							a[k] -= 2;
							vec.push_back(k);
							for (int l = k + 1; l <= 14; l++)
							{
								if (l <= j && l >= j - length + 1)
								{
									continue;
								}
								if (a[l] >= 2)
								{
									a[l] -= 2;
									vec.push_back(l);
									cardcombotype* p = new Plane2(j - length + 1, length, vec);
									ans_got = 1;
									median.push_back(p);
									double value = p->getvalue();
									median.mark += value;
									if (median.mark < min_value && !no_time)
										strategy();
									median.mark -= value;
									median.pop();
									delete p;
									vec.pop_back();
									a[l] += 2;
									break;
								}
							}
							vec.pop_back();
							a[k] += 2;
							if (ans_got) break;
						}
					}
				}
				else if (length == 3)
				{
					for (int k = 0; k <= 14; k++)
					{
						if (sum < 15) break;
						if (k <= j && k >= j - length + 1)
						{
							continue;
						}
						if (a[k] >= 2)
						{
							a[k] -= 2;
							vec.push_back(k);
							for (int l = k + 1; l <= 14; l++)
							{
								if (l <= j && l >= j - length + 1)
								{
									continue;
								}
								if (a[l] >= 2)
								{
									a[l] -= 2;
									vec.push_back(l);
									for (int m = l + 1; m <= 14; m++)
									{
										if (m <= j && m >= j - length + 1)
										{
											continue;
										}
										if (a[m] >= 2)
										{
											a[m] -= 2;
											vec.push_back(m);
											cardcombotype* p = new Plane2(j - length + 1, length, vec);
											ans_got = 1;
											median.push_back(p);
											double value = p->getvalue();
											median.mark += value;
											if (median.mark < min_value && !no_time)
												strategy();
											median.mark -= value;
											median.pop();
											delete p;
											vec.pop_back();
											a[m] += 2;
											if (ans_got) break;
										}
									}
									vec.pop_back();
									a[l] += 2;
									if (ans_got) break;
								}
							}
							vec.pop_back();
							a[k] += 2;
							if (ans_got) break;
						}
					}
				}
				else if (length == 4)
				{
					if (sum >= 20)
					{
						int used[5];
						int num = 0;
						for (int k = 0; k <= 14; k++)
						{
							if (k <= j && k >= j - length + 1)
							{
								continue;
							}
							if (a[k] == 2)
							{
								used[num++] = k;
								a[k] -= 2;
								vec.push_back(k);
							}
						}
						if (vec.size() == 4)
						{
							cardcombotype* p = new Plane2(j - length + 1, length, vec);
							median.push_back(p);
							double value = p->getvalue();
							median.mark += value;
							if (median.mark < min_value && !no_time)
								strategy();
							median.mark -= value;
							median.pop();
							delete p;
						}
						for (int k = 0; k < num; k++)
						{
							a[used[k]] += 2;
						}
						vec.clear();
					}
				}

				for (int k = j; k >= j - length + 1; k--)
				{
					a[k] += 3;
				}
			}
		}
	}
	for (int i = 0; i <= 10; i++)
	{
		if (sum < 8) break;
		int length = 0;
		for (int j = i; j <= 11; j++)
		{
			if (a[j] == 4)
			{
				length++;
			}
			else if (a[j] < 4)
			{
				length = 0;
				i = j;
				break;
			}
			if (length >= 2)
			{
				only_single = false;
				for (int k = j; k >= j - length + 1; k--)
				{
					a[k] -= 4;
				}
				bool ans_got = 0;
				cardcombotype* p = new Sshuttle(j - length + 1, length);
				median.push_back(p);
				double value = p->getvalue();
				median.mark += value;
				if (median.mark < min_value && !no_time)
					strategy();
				median.mark -= value;
				median.pop();
				delete p;

				vector<short>vec;
				if (length == 2)
				{
					for (int k = 0; k <= 14; k++)
					{
						if (sum < 12) break;
						if (k <= j && k >= j - length + 1)
						{
							continue;
						}
						if (a[k])
						{
							a[k]--;
							vec.push_back(k);
							for (int l = k + 1; l <= 14; l++)
							{
								if (l <= j && l >= j - length + 1)
								{
									continue;
								}
								if (a[l])
								{
									a[l]--;
									vec.push_back(l);
									for (int m = l + 1; m <= 14; m++)
									{
										if (m <= j && m >= j - length + 1)
										{
											continue;
										}
										if (a[m])
										{
											a[m]--;
											vec.push_back(m);
											for (int n = m + 1; n <= 14; n++)
											{
												if (n <= j && m >= j - length + 1)
												{
													continue;
												}
												if (a[n])
												{
													a[n]--;
													vec.push_back(n);
													cardcombotype* p = new Sshuttle2(j - length + 1, length, vec);
													ans_got = 1;
													median.push_back(p);
													double value = p->getvalue();
													median.mark += value;
													if (median.mark < min_value && !no_time)
														strategy();
													median.mark -= value;
													median.pop();
													delete p;
													vec.pop_back();
													a[n]++;
													break;
												}
											}
											vec.pop_back();
											a[m]++;
											if (ans_got) break;
										}
									}
									vec.pop_back();
									a[l]++;
									if (ans_got) break;
								}
							}
							vec.pop_back();
							a[k]++;
							if (ans_got) break;
						}
					}
				}
				else if (length == 3)
				{
					for (int k = 0; k <= 14; k++)
					{
						if (sum < 18) break;
						if (k <= j && k >= j - length + 1)
						{
							continue;
						}
						if (a[k])
						{
							a[k]--;
							vec.push_back(k);
							for (int l = k + 1; l <= 14; l++)
							{
								if (l <= j && l >= j - length + 1)
								{
									continue;
								}
								if (a[l])
								{
									a[l]--;
									vec.push_back(l);
									for (int m = l + 1; m <= 14; m++)
									{
										if (m <= j && m >= j - length + 1)
										{
											continue;
										}
										if (a[m])
										{
											a[m]--;
											vec.push_back(m);
											for (int n = m + 1; n <= 14; n++)
											{
												if (n <= j && m >= j - length + 1)
												{
													continue;
												}
												if (a[n])
												{
													a[n]--;
													vec.push_back(n);
													for (int o = n + 1; o <= 14; o++)
													{
														if (o <= j && o >= j - length + 1)
														{
															continue;
														}
														if (a[o])
														{
															a[o]--;
															vec.push_back(o);
															for (int q = n + 1; q <= 14; q++)
															{
																if (q <= j && q >= j - length + 1)
																{
																	continue;
																}
																if (a[q])
																{
																	a[q]--;
																	vec.push_back(q);
																	cardcombotype* p = new Sshuttle2(j - length + 1, length, vec);
																	ans_got = 1;
																	median.push_back(p);
																	double value = p->getvalue();
																	median.mark += value;
																	if (median.mark < min_value && !no_time)
																		strategy();
																	median.mark -= value;
																	median.pop();
																	delete p;
																	vec.pop_back();
																	a[q]++;
																	break;
																}
															}
															vec.pop_back();
															a[o]++;
															if (ans_got) break;
														}
													}
													vec.pop_back();
													a[n]++;
													if (ans_got) break;
												}
											}
											vec.pop_back();
											a[m]++;
											if (ans_got) break;
										}
									}
									vec.pop_back();
									a[l]++;
									if (ans_got) break;
								}
							}
							vec.pop_back();
							a[k]++;
							if (ans_got) break;
						}
					}
				}

				ans_got = 0;
				if (length == 2)
				{
					for (int k = 0; k <= 14; k++)
					{
						if (sum < 16) break;
						if (k <= j && k >= j - length + 1)
						{
							continue;
						}
						if (a[k] >= 2)
						{
							a[k] -= 2;
							vec.push_back(k);
							for (int l = k + 1; l <= 14; l++)
							{
								if (l <= j && l >= j - length + 1)
								{
									continue;
								}
								if (a[l] >= 2)
								{
									a[l] -= 2;
									vec.push_back(l);
									for (int m = l + 1; m <= 14; m++)
									{
										if (m <= j && m >= j - length + 1)
										{
											continue;
										}
										if (a[m] >= 2)
										{
											a[m] -= 2;
											vec.push_back(m);
											for (int n = m + 1; n <= 14; n++)
											{
												if (n <= j && m >= j - length + 1)
												{
													continue;
												}
												if (a[n] >= 2)
												{
													a[n] -= 2;
													vec.push_back(n);
													cardcombotype* p = new Sshuttle4(j - length + 1, length, vec);
													ans_got = 1;
													median.push_back(p);
													double value = p->getvalue();
													median.mark += value;
													if (median.mark < min_value && !no_time)
														strategy();
													median.mark -= value;
													median.pop();
													delete p;
													vec.pop_back();
													a[n] += 2;
													break;
												}
											}
											vec.pop_back();
											a[m] += 2;
											if (ans_got) break;
										}
									}
									vec.pop_back();
									a[l] += 2;
									if (ans_got) break;
								}
							}
							vec.pop_back();
							a[k] += 2;
							if (ans_got) break;
						}
					}
				}

				for (int k = j; k >= j - length + 1; k--)
				{
					a[k] += 4;
				}
			}
		}
	}
	for (int i = 0; i <= 12; i++)
	{
		if (sum < 4) break;
		if (a[i] == 4)
		{
			if (i <= 11)
			{
				only_single = false;
			}
			a[i] -= 4;

			cardcombotype* p = new Bomb(i);
			median.push_back(p);
			double value = p->getvalue();
			median.mark += value;
			if (median.mark < min_value && !no_time)
				strategy();
			median.mark -= value;
			median.pop();
			delete p;

			vector<short>vec;
			bool ans_got = 0;
			if (sum >= 6)
				for (int j = 0; j <= 14; j++)
				{
					if (a[j])
					{
						a[j]--;
						vec.push_back(j);
						for (int k = j + 1; k <= 14; k++)
						{
							if (a[k])
							{
								a[k]--;
								vec.push_back(k);
								cardcombotype* p = new Quadruple2(i, vec);
								ans_got = 1;
								median.push_back(p);
								double value = p->getvalue();
								median.mark += value;
								if (median.mark < min_value && !no_time)
									strategy();
								median.mark -= value;
								median.pop();
								delete p;
								a[k]++;
								vec.pop_back();
								break;
							}
						}
						vec.pop_back();
						a[j]++;
						if (ans_got) break;
					}
				}

			ans_got = 0;
			if (sum >= 8)
				for (int j = 0; j <= 11; j++)
				{
					if (a[j] >= 2)
					{
						a[j] -= 2;
						vec.push_back(j);
						for (int k = j + 1; k <= 12; k++)
						{
							if (a[k] >= 2)
							{
								a[k] -= 2;
								vec.push_back(k);
								cardcombotype* p = new Quadruple4(i, vec);
								ans_got = 1;
								median.push_back(p);
								double value = p->getvalue();
								median.mark += value;
								if (median.mark < min_value && !no_time)
									strategy();
								median.mark -= value;
								median.pop();
								delete p;
								a[k] += 2;
								vec.pop_back();
								break;
							}
						}
						vec.pop_back();
						a[j] += 2;
						if (ans_got) break;
					}
				}

			a[i] += 4;
		}
	}
	for (int i = 0; i <= 12; i++)
	{
		if (sum < 3) break;
		if (a[i] >= 3)
		{
			if (i != 12)
				only_single = false;
			a[i] -= 3;

			cardcombotype* p = new Triplet(i);
			median.push_back(p);
			double value = p->getvalue();
			median.mark += value;
			if (median.mark < min_value && !no_time)
				strategy();
			median.mark -= value;
			median.pop();
			delete p;

			for (int j = 0; j <= 14; j++)
			{
				if (j == i)
				{
					continue;
				}
				if (sum < 4) break;
				if (a[j])
				{
					a[j]--;
					cardcombotype* p = new Triplet1(i, j);
					median.push_back(p);
					double value = p->getvalue();
					median.mark += value;
					if (median.mark < min_value && !no_time)
						strategy();
					median.mark -= value;
					median.pop();
					delete p;
					a[j]++;
					break;
				}
			}

			for (int j = 0; j <= 12; j++)
			{
				if (j == i)
				{
					continue;
				}
				if (sum < 5) break;
				if (a[j] >= 2)
				{
					a[j] -= 2;
					cardcombotype* p = new Triplet2(i, j);
					median.push_back(p);
					double value = p->getvalue();
					median.mark += value;
					if (median.mark < min_value && !no_time)
						strategy();
					median.mark -= value;
					median.pop();
					delete p;
					a[j] += 2;
					break;
				}
			}
			a[i] += 3;
		}
	}
	for (int i = 0; i <= 12; i++)
	{
		if (sum < 2) break;
		if (a[i] >= 2)
		{
			if (i != 12)
				only_single = false;
			cardcombotype* p = new Pair(i);
			a[i] -= 2;
			median.push_back(p);
			double value = p->getvalue();
			median.mark += value;
			if (median.mark < min_value && !no_time)
				strategy();
			median.mark -= value;
			median.pop();
			delete p;
			a[i] += 2;
		}
	}
	if (only_single)
	{
		vector<pcards> vec;
		for (int i = 0; i <= 14; i++)
		{
			if (a[i] != 0)
			{
				for (int j = 1; j <= a[i]; j++)
				{
					cardcombotype* p = new Single(i);
					vec.push_back(p);
				}
			}
		}
		int size = vec.size();
		if (size != 0)
		{
			for (pcards c : vec)
			{
				median.mark += c->getvalue();
			}
			for (pcards c : vec)
			{
				median.push_back(c);
			}
			median.renew_mark();
			median.mark_renew;
			double mark1 = median.mark_renew;
			if (min_value > median.mark)
			{
				min_value = median.mark;
			}
			if (renew_value > mark1)
			{
				renew_value = mark1;
				mystrategy = median;
				min_value = median.mark;
			}
			for (pcards c : vec)
			{
				median.pop();
			}
			for (pcards c : vec)
			{
				median.mark -= c->getvalue();
			}
		}
	}
}
void ptoa(cardcombotype* p)
{
	if (p == NULL)
	{
		return;
	}
	switch (p->number) {
	case 1: {
		short ans = leveltocard(p->find_card());
		myaction.push_back(ans);
		break;
	}
	case 2: {
		short ans = leveltocard(p->find_card());
		myaction.push_back(ans);
		ans = leveltocard(p->find_card());
		myaction.push_back(ans);
		break;
	}
	case 3: {
		short start = p->find_card();
		int length = p->find_length();
		for (int i = start; i < start + length; i++)
		{
			myaction.push_back(leveltocard(i));
		}
		break;
	}
	case 4: {
		short start = p->find_card();
		int length = p->find_length();
		for (int i = start; i < start + length; i++)
		{
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
		}
		break;
	}
	case 5: {
		short ans = p->find_card();
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		break;
	}
	case 6: {
		short ans = p->find_card();
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		ans = p->find_another();
		myaction.push_back(leveltocard(ans));
		break;
	}
	case 7: {
		short ans = p->find_card();
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		ans = p->find_another();
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		break;
	}
	case 8: {
		short ans = p->find_card();
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		break;
	}
	case 9: {
		short ans = p->find_card();
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		vector<short> vec = p->find_remain();
		for (short s : vec)
		{
			myaction.push_back(leveltocard(s));
		}
		break;
	}
	case 10: {
		short ans = p->find_card();
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		myaction.push_back(leveltocard(ans));
		vector<short> vec = p->find_remain();
		for (short s : vec)
		{
			myaction.push_back(leveltocard(s));
			myaction.push_back(leveltocard(s));
		}
		break;
	}
	case 11: {
		short start = p->find_card();
		int length = p->find_length();
		for (int i = start; i < start + length; i++)
		{
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
		}
		break;
	}
	case 12: {
		short start = p->find_card();
		int length = p->find_length();
		for (int i = start; i < start + length; i++)
		{
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
		}
		vector<short> vec = p->find_remain();
		for (short s : vec)
		{
			myaction.push_back(leveltocard(s));
		}
		break;
	}
	case 13: {
		short start = p->find_card();
		int length = p->find_length();
		for (int i = start; i < start + length; i++)
		{
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
		}
		vector<short> vec = p->find_remain();
		for (short s : vec)
		{
			myaction.push_back(leveltocard(s));
			myaction.push_back(leveltocard(s));
		}
		break;
	}
	case 14: {
		short start = p->find_card();
		int length = p->find_length();
		for (int i = start; i < start + length; i++)
		{
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
		}
		break;
	}
	case 15: {
		short start = p->find_card();
		int length = p->find_length();
		for (int i = start; i < start + length; i++)
		{
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
		}
		vector<short> vec = p->find_remain();
		for (short s : vec)
		{
			myaction.push_back(leveltocard(s));
		}
		break;
	}
	case 16: {
		short start = p->find_card();
		int length = p->find_length();
		for (int i = start; i < start + length; i++)
		{
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
			myaction.push_back(leveltocard(i));
		}
		vector<short> vec = p->find_remain();
		for (short s : vec)
		{
			myaction.push_back(leveltocard(s));
			myaction.push_back(leveltocard(s));
		}
		break;
	}
	case 17: {
		myaction.push_back(leveltocard(13));
		myaction.push_back(leveltocard(14));
		break;
	}
	default:
		break;
	}
}
void renew_action(int position)
{
	int another_pos = -1;
	if (myPosition != landlordPosition)
	{
		for (int i = 0; i <= 2; i++)
		{
			if (i != landlordPosition && i != myPosition)
			{
				another_pos = i;
			}
		}
	}
	bool help = 0;
	bool attack = 0;
	bool yibo = 0;
	if (myPosition == landlordPosition) attack = 1;
	else {
		if (landpush) attack = 1;
		if (cardRemaining[another_pos] <= 3)
		{
			help = 1;
		}
	}
	int card_attack = 17;
	if (myPosition == landlordPosition)
	{
		for (int i = 0; i <= 2; i++)
		{
			if (i != landlordPosition)
			{
				if (cardRemaining[i] < card_attack)
				{
					card_attack = cardRemaining[i];
				}
			}
		}
	}
	else card_attack = cardRemaining[landlordPosition];
	if (nobody_push)
	{
		strategy();
		mystrategy;
		double mmin2 = renew_value + 0.29;
		int vvva = 0;
		double vp = 0;
		bool f1 = 0;
		for (pcards p : mystrategy.vc)
		{
			if (p->getvalue() > 0.0001)
			{
				vvva++;
				if (mystrategy.st[p->number].size() == 1)
				{
					f1 = 1;
				}
			}
			if (vvva >= 2) break;
		}
		if ((vvva <= 1 && pos_bom < 0.005)||(vvva==1&&f1))
		{
			double min_pos = 10000;
			pcards ans = NULL;
			for (int ii = 1; ii <= 17; ii++)
			{
				if ((ii >= 8 && ii <= 10) || (ii <= 17 && ii >= 14)) continue;
				if (!mystrategy.st[ii].empty())
				{
					for (pcards p : mystrategy.st[ii])
					{
						int min_card = 0;
						if (p->find_card()>min_card && p->getvalue() <= 0.01)
						{
							min_card = p->find_card();
							ans = p;
						}
					}
				}
			}
			if (ans != NULL)
			{
				ptoa(ans);
				return;
			}
			for (pcards p : mystrategy.vc)
			{
				if (p->getvalue() <= 0.01)
				{
					ptoa(p);
					return;
				}
			}
		}
		if (mystrategy.vc.size() - mystrategy.st[8].size() - mystrategy.st[17].size() > 2 && card_attack > 3)
		{
			pcards ans = NULL;
			bool ans_got = 0;
			if (help && renew_value > 1.0)
			{
				if (cardRemaining[another_pos] % 2 == 1)
				{
					short min_cannot = 14;
					if (!whattheypassed[another_pos].st[1].empty())
					{
						for (pcards c : whattheypassed[another_pos].st[1])
						{
							if (c->find_card() < min_cannot)
								min_cannot = c->find_card();
						}
					}
					for (int i = 0; i < min_cannot; i++)
					{
						if (a[i])
						{
							a[i] -= 1;
							pcards p = new Single(i);
							strategy();
							if (mmin2 > renew_value)
							{
								mmin2 = renew_value;
								ans = p;
								ans_got = 1;
							}
							a[i] += 1;
						}
					}
					if (ans_got)
					{
						ptoa(ans);
						return;
					}
				}
				if (cardRemaining[another_pos] % 2 == 0)
				{
					short min_cannot = 14;
					if (!whattheypassed[another_pos].st[2].empty())
					{
						for (pcards c : whattheypassed[another_pos].st[2])
						{
							if (c->find_card() < min_cannot)
								min_cannot = c->find_card();
						}
					}
					for (int i = 0; i < min_cannot; i++)
					{
						if (a[i] >= 2)
						{
							a[i] -= 2;
							pcards p = new Pair(i);
							strategy();
							if (mmin2 > renew_value)
							{
								mmin2 = renew_value;
								ans = p;
								ans_got = 1;
							}
							a[i] += 2;
						}
					}
					if (ans_got)
					{
						ptoa(ans);
						return;
					}
				}
			}
			double min_pos = 100;
			double max_pos = 0;
			int ans_ii = -1;
			int num_type = 0;
			for (int ii = 1; ii <= 17; ii++)
			{
				if (ii == 8 || ii == 17) continue;
				if (!mystrategy.st[ii].empty()) num_type++;
			}
			int aim_i1 = -1;
			int aim_i2 = -1;
			double mv = 100;
			if (num_type == 2)
			{
				for (int ii = 1; ii <= 17; ii++)
				{
					if (ii == 8 || ii == 17) continue;
					if (!mystrategy.st[ii].empty())
					{
						aim_i2 = ii;
						for (pcards p : mystrategy.st[ii])
						{
							if (p->value < mv)
							{
								mv = p->value;
								if (ii != aim_i1) aim_i2 = aim_i1;
								aim_i1 = ii;
							}
						}
					}
				}
				if (mystrategy.st[aim_i2].size() == 1 && mv < 0.059)
				{
					double mav = -1;
					for (pcards p : mystrategy.st[aim_i1])
					{
						if (p->value > mav)
						{
							mav = p->value;
							ans = p;
						}
					}
					if (ans != NULL)
					{
						ptoa(ans);
						return;
					}
				}
			}
			for (int ii = 1; ii <= 17; ii++)
			{
				if ((ii >= 8 && ii <= 10) || (ii <= 17 && ii >= 14)) continue;
				if (!mystrategy.st[ii].empty())
				{
					for (pcards p : mystrategy.st[ii])
					{
						if (p->antipossb<min_pos)
						{
							min_pos = p->antipossb;
							ans = p;
						}
					}
				}
			}
			/*	double mmax = -100;
				int max_size = 0;
				int min_val = 0.49;
				int ans_ii = 0;
				for (int ii = 1; ii <= 17; ii++)
				{
					if (ii == 8 || ii == 17) continue;
					if (!mystrategy.st[ii].empty())
					{
						for (pcards p : mystrategy.st[ii])
						{
							if (p->value < min_val)
							{
								min_val = p->value;
								ans_ii = ii;
							}
						}
					}
				}
				for (pcards p : mystrategy.st[ans_ii])
				{
					if (p->value > mmax)
					{
						mmax = p->value;
						ans = p;
					}
				}*/
			if (ans != NULL)
			{
				ptoa(ans);
				return;
			}
			double mmax = -100;
			for (pcards p : mystrategy.vc)
			{
				if (p->value > mmax)
				{
					mmax = p->value;
					ans = p;
				}
			}
			if (ans != NULL)
			{
				ptoa(ans);
				return;
			}
			for (int ii = 3; ii <= 7; ii++)
			{
				if (!mystrategy.st[ii].empty())
				{
					for (pcards p : mystrategy.st[ii])
					{
						if (p->value > mmax)
						{
							mmax = p->value;
							ans = p;
						}
					}
				}
			}
			for (int ii = 11; ii <= 13; ii++)
			{
				if (!mystrategy.st[ii].empty())
					for (pcards p : mystrategy.st[ii])
					{
						ans = p;
					}
			}
			if (ans != NULL)
			{
				ptoa(ans);
				return;
			}
			mmax = -100;
			for (pcards p : mystrategy.vc)
			{
				if (p->value > mmax)
				{
					mmax = p->value;
					ans = p;
				}
			}
			ptoa(ans);
			return;
		}
		else {
			pcards ans = NULL;
			double mmin = 10000;
			if (mystrategy.vc.size() - mystrategy.st[8].size() - mystrategy.st[17].size() < 2)
			{
				if (!mystrategy.st[17].empty())
				{
					ans = mystrategy.st[17][0];
				}
				if (!mystrategy.st[8].empty())
				{
					ans = mystrategy.st[8][0];
				}
			}
			if (ans != NULL)
			{
				ptoa(ans);
				return;
			}
			bool flag = 0;
			if (card_attack <= 3)
			{
				for (int i = 1; i <= 17; i++)
				{
					if (i == card_attack || i == 1 || i == 8 || i == 17) continue;
					if (card_attack == 3 && i == 2) continue;
					if (!mystrategy.st[i].empty())
					{
						int card_mmin = 14;
						for (int j = 0; j < mystrategy.st[i].size(); j++)
						{
							if (mystrategy.st[i][j]->find_card() < card_mmin)
							{
								card_mmin = mystrategy.st[i][j]->find_card();
								ans = mystrategy.st[i][j];
								flag = 1;
							}
						}
					}
				}
				if (flag)
				{
					ptoa(ans);
					return;
				}
				else if (!flag)
				{
					if (card_attack == 1)
					{
						bool flag1 = 0;
						int card_mmin = 14;
						for (int i = 0; i < mystrategy.st[2].size(); i++)
						{
							if (mystrategy.st[2][i]->find_card() < card_mmin)
							{
								card_mmin = mystrategy.st[2][i]->find_card();
								ans = mystrategy.st[2][i];
								flag1 = 1;
							}
						}
						if (flag1)
						{
							ptoa(ans);
							return;
						}
						else if (!flag1)
						{
							bool flag2 = 0;
							int card_max = 0;
							for (int i = 0; i < mystrategy.st[1].size(); i++)
							{
								if (mystrategy.st[1][i]->find_card() > card_max)
								{
									card_max = mystrategy.st[1][i]->find_card();
									ans = mystrategy.st[1][i];
									flag2 = 1;
								}
							}
							if (flag2)
							{
								ptoa(ans);
								return;
							}
						}
					}
					if (card_attack == 2)
					{
						bool flag1 = 0;
						int card_mmin = 14;
						for (int i = 0; i < mystrategy.st[1].size(); i++)
						{
							if (mystrategy.st[1][i]->find_card() < card_mmin)
							{
								card_mmin = mystrategy.st[1][i]->find_card();
								ans = mystrategy.st[1][i];
								flag1 = 1;
							}
						}
						if (flag1)
						{
							ptoa(ans);
							return;
						}
						else if (!flag1)
						{
							bool flag2 = 0;
							int card_max = 0;
							for (int i = 0; i < mystrategy.st[2].size(); i++)
							{
								if (mystrategy.st[2][i]->find_card() > card_max)
								{
									card_max = mystrategy.st[2][i]->find_card();
									ans = mystrategy.st[2][i];
									flag2 = 1;
								}
							}
							if (flag2)
							{
								ptoa(ans);
								return;
							}
						}
					}
					if (card_attack == 3)
					{
						double mv = 100;
						double mp = 100;
						int ans_iii = 1;
						for (pcards p : mystrategy.st[1])
						{
							if (p->possb < mp)
							{
								mp = p->possb;
								ans_iii = 1;
							}
						}
						for (pcards p : mystrategy.st[2])
						{
							if (p->possb < mp)
							{
								mp = p->possb;
								ans_iii = 2;
							}
						}
						double mav = 0;
						bool fl1 = 0;
						for (pcards p : mystrategy.st[ans_iii])
						{
							if (p->value > mav)
							{
								ans = p;
								mav = p->value;
								fl1 = 1;
							}
						}
						if (fl1)
						{
							ptoa(ans);
							return;
						}
						else {
							mv = 100;
							for (pcards p : mystrategy.st[3])
							{
								if (p->value < mv) {
									mv = p->value;
									ans = p;
								}
							}
							if (ans)
							{
								ptoa(ans);
								return;
							}
						}
					}
				}
			}
			flag = 0;
			for (int i = 1; i <= 17; i++)
			{
				if (i == 8 || i == 17) continue;
				if (mystrategy.st[i].size() >= 2)
				{
					int card_mmin = 14;
					for (int j = 0; j < mystrategy.st[i].size(); j++)
					{
						if (mystrategy.st[i][j]->find_card() < card_mmin)
						{
							card_mmin = mystrategy.st[i][j]->find_card();
							ans = mystrategy.st[i][j];
							flag = 1;
						}
					}
				}
			}
			if (!flag)
			{
				double pp = 100;
				for (pcards p : mystrategy.vc)
				{
					if (p->antipossb < pp && p->number != 8 && p->number != 17)
					{
						pp = p->antipossb;
						ans = p;
					}
				}
			}
			ptoa(ans);
			return;
		}
	}

	strategy();
	int size1 = 0;
	int vvva = 0;
	for (pcards p : mystrategy.vc)
	{
		if (p->getvalue() > 0.01) vvva++;
		if (vvva >= 2) break;
	}
	for (int i = 1; i <= 17; i++)
	{
		if (i == 8 || i == 17)
		{
			continue;
		}
		size1 += mystrategy.st[i].size();
	}
	if (vvva <= 1 && pos_bom < 0.005)
	{
		yibo = true;
	}
	if (yibo)
	{
		pcards ans = NULL;
		if (!mystrategy.st[lastcombo->number].empty())
		{
			int card = lastcombo->find_card();
			int mmmax = 0;
			for (pcards p : mystrategy.st[lastcombo->number])
			{
				if (p->find_card() > card && p->find_card() > mmmax&&p->find_length()==lastcombo->find_length())
				{
					mmmax = p->find_card();
					ans = p;
				}
			}
			if (ans != NULL)
			{
				ptoa(ans);
				return;
			}
		}
		if (ans == NULL)
		{
			if (!mystrategy.st[8].empty())
			{
				ptoa(mystrategy.st[8][0]);
				return;
			}
			if (!mystrategy.st[17].empty())
			{
				ptoa(mystrategy.st[17][0]);
				return;
			}
		}
	}
	double mmin;
	bool fflg = 0;
	if (mystrategy.vc.size() <= 2)
	{
		pcards off = NULL;
		pcards in = NULL;
		for (pcards p : mystrategy.vc)
		{
			if (p->getvalue() <= 0.01) off = p;
			else in = p;
		}
		if (off != NULL && in != NULL)
		{
			mmin = in->getvalue() + 0.20;
			fflg = 1;
		}
	}
	if (!fflg)
	{
		if (attack)
		{
			if (myPosition != landlordPosition && renew_value > 4) mmin = renew_value + 0.39 + 0.02 * (20 - card_attack);
			else mmin = renew_value + 0.39 + 0.010 * (20 - card_attack);
			if (card_attack <= 3)
			{
				mmin = renew_value + 0.79;
			}
		}
		else mmin = renew_value + 0.05;
	}
	renew_value = 10000;
	min_value = 10000;

	pcards rock = NULL;
	bool lessthan2 = 0;
	bool can_use_rocket = 0;
	if (a[13] && a[14])
	{
		a[13]--;
		a[14]--;
		rock = new Rocket();
		strategy();
		int size = 0;
		for (int i = 1; i <= 17; i++)
		{
			if (i == 8 || i == 17) continue;
			size += mystrategy.st[i].size();
		}
		if (size <= 1) lessthan2 = true;
		a[13]++;
		a[14]++;
	}
	double value_userocket = renew_value;
	mystrategy;
	if ((value_userocket < 0.39 || lessthan2) && rock != NULL)
	{
		can_use_rocket = 1;
	}
	min_value = 10000;
	renew_value = 10000;

	pcards bom = NULL;
	bool can_use_bomb = 0;
	double value_usebomb = 10000;
	for (int i = 0; i <= 12; i++)
	{
		if (a[i] == 4)
		{
			a[i] -= 4;

			pcards bom2 = new Bomb(i);
			strategy();
			int size = 0;
			for (int i = 1; i <= 17; i++)
			{
				if (i == 8 || i == 17) continue;
				size += mystrategy.st[i].size();
			}
			if (size <= 1) lessthan2 = true;
			if (renew_value < value_usebomb)
			{
				bom = bom2;
				value_usebomb = renew_value;
			}

			a[i] += 4;
		}
	}
	mystrategy;
	if ((value_usebomb < 0.39 || lessthan2) && bom != NULL)
	{
		can_use_bomb = 1;
	}
	min_value = 10000;
	renew_value = 10000;

	switch (lastcombo->number)
	{
	case 17: {
		return;
	}
	case 16: {
		if (!attack) return;
		if (attack || lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		return;
	}
	case 15: {
		if (!attack) return;
		if (attack || lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		return;
	}
	case 14: {
		if (!attack) return;
		if (attack || lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		return;
	}
	case 13: {
		if (lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		if (!attack && !yibo) return;
		if (5 * lastcombo->find_length() > myCards.size())
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		pcards pla = NULL;
		bool ans_got = 0;
		int length = lastcombo->find_length();
		int start = lastcombo->find_card();
		short min_cannot = 12 - length;
		for (int i = start + 1; i <= min_cannot; i++)
		{
			int flag = 1;
			for (int j = 0; j < length; j++)
			{
				if (a[j + i] < 3)
				{
					flag = 0;
					i = i + j;
					break;
				}
			}
			if (flag)
			{
				for (int j = 0; j < length; j++)
				{
					a[j + i] -= 3;
				}
				vector<short>pair;
				int num = 0;
				int flag = 0;
				vector<int>deleted;
				for (int k = 0; k <= 12; k++)
				{
					if (k >= i && k < i + length) continue;
					if (a[k] >= 2) {
						a[k] -= 2;
						deleted.push_back(k);
						num++;
						pair.push_back(k);
					}
					if (num == length)
					{
						flag = 1;
						break;
					}
				}
				strategy();
				if (renew_value < mmin)
				{
					pla = new Plane2(i, length, pair);
					ans_got = 1;
				}
				min_value = 10000;
				renew_value = 10000;
				for (int c : deleted)
				{
					a[c] += 2;
				}
				for (int j = 0; j < length; j++)
				{
					a[j + i] += 3;
				}
			}
			if (ans_got) break;
		}
		if (ans_got) {
			ptoa(pla);
			return;
		}
		else {
			if (attack || lessthan2)
			{
				if (can_use_bomb)
				{
					ptoa(bom);
					return;
				}
				if (can_use_rocket)
				{
					ptoa(rock);
					return;
				}
			}
		}
		return;
	}
	case 12: {
		if (lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		if (!attack && !yibo) return;
		if (4 * lastcombo->find_length() > myCards.size())
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		pcards pla = NULL;
		bool ans_got = 0;
		int length = lastcombo->find_length();
		int start = lastcombo->find_card();
		short min_cannot = 12 - length;
		for (int i = start + 1; i <= min_cannot; i++)
		{
			int flag = 1;
			for (int j = 0; j < length; j++)
			{
				if (a[j + i] < 3)
				{
					flag = 0;
					i = i + j;
					break;
				}
			}
			if (flag)
			{
				for (int j = 0; j < length; j++)
				{
					a[i + j] -= 3;
				}
				vector<short>single;
				int num = 0;
				int flag = 0;
				vector<int>deleted;
				for (int k = 0; k <= 14; k++)
				{
					if (k >= i && k < i + length) continue;
					if (a[k] == 1) {
						a[k] --;
						single.push_back(k);
						num++;
						deleted.push_back(k);
					}
					if (num == length)
					{
						flag = 1;
						break;
					}
				}
				strategy();
				if (renew_value < mmin)
				{
					pla = new Plane1(i, length, single);
					ans_got = 1;
				}
				min_value = 10000;
				renew_value = 10000;
				for (int c : deleted)
				{
					a[c] ++;
				}
				for (int j = 0; j < length; j++)
				{
					a[i + j] += 3;
				}
			}
			if (ans_got) break;
		}
		if (ans_got) {
			ptoa(pla);
			return;
		}
		else {
			if (attack || lessthan2)
			{
				if (can_use_bomb)
				{
					ptoa(bom);
					return;
				}
				if (can_use_rocket)
				{
					ptoa(rock);
					return;
				}
			}
		}
		return;
	}
	case 11: {
		if (lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		if (!attack && !yibo) return;
		if (3 * lastcombo->find_length() > myCards.size())
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		pcards ans = NULL;
		bool ans_got = 0;
		int length = lastcombo->find_length();
		int start = lastcombo->find_card();
		short min_cannot = 12 - length;
		for (int i = start + 1; i <= min_cannot; i++)
		{
			int flag = 1;
			for (int j = 0; j < length; j++)
			{
				if (a[j + i] < 3)
				{
					flag = 0;
					i = i + j;
					break;
				}
			}
			if (flag)
			{
				for (int j = 0; j < length; j++)
				{
					a[i + j] -= 3;
				}

				pcards p = new Plane(i, length);
				double value = p->getvalue();
				strategy();
				if (mmin > renew_value)
				{
					mmin = renew_value;
					ans = p;
					ans_got = 1;
				}

				for (int j = 0; j < length; j++)
				{
					a[i + j] += 3;
				}
			}
		}
		if (ans_got) {
			ptoa(ans);
			return;
		}
		else {
			if (attack || lessthan2)
			{
				if (can_use_bomb)
				{
					ptoa(bom);
					return;
				}
				if (can_use_rocket)
				{
					ptoa(rock);
					return;
				}
			}
		}
		return;
	}
	case 10: {
		if (lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		short card = lastcombo->find_card();
		if (!attack && !yibo) return;
		if (8 > myCards.size())
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}

		pcards ans = NULL;
		bool ans_got = 0;
		vector<short>vec;
		for (int i = card + 1; i <= 12; i++)
		{
			if (a[i] == 4)
			{
				a[i] -= 4;
				for (int j = 0; j <= 11; j++)
				{
					if (a[j] >= 2)
					{
						a[j] -= 2;
						vec.push_back(j);
						for (int k = j + 1; k <= 12; k++)
						{
							if (a[k] >= 2)
							{
								a[k] -= 2;
								vec.push_back(k);
								cardcombotype* p = new Quadruple4(i, vec);
								double value = p->getvalue();
								strategy();
								if (renew_value < mmin)
								{
									mmin = renew_value;
									ans = p;
									ans_got = 1;
								}
								min_value = 10000;
								renew_value = 10000;
								a[k] += 2;
								vec.pop_back();
							}
						}
						vec.pop_back();
						a[j] += 2;
					}
				}
				a[i] += 4;
				if (ans_got) break;
			}
		}
		if (ans_got)
		{
			ptoa(ans);
			return;
		}
		else {
			if (attack || lessthan2)
			{
				if (can_use_bomb)
				{
					ptoa(bom);
					return;
				}
				if (can_use_rocket)
				{
					ptoa(rock);
					return;
				}
			}
		}
		return;
	}
	case 9: {
		if (lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		short card = lastcombo->find_card();
		if (!attack && !yibo) return;
		if (6 > myCards.size())
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}

		pcards ans = NULL;
		bool ans_got = 0;
		vector<short>vec;
		for (int i = card + 1; i <= 12; i++)
		{
			if (a[i] == 4)
			{
				a[i] -= 4;
				for (int j = 0; j <= 14; j++)
				{
					if (a[j])
					{
						a[j]--;
						vec.push_back(j);
						for (int k = j + 1; k <= 14; k++)
						{
							if (a[k])
							{
								a[k]--;
								vec.push_back(k);
								cardcombotype* p = new Quadruple2(i, vec);
								double value = p->getvalue();
								strategy();
								if (mmin > renew_value)
								{
									mmin = renew_value;
									ans = p;
									ans_got = 1;
								}
								min_value = 10000;
								renew_value = 10000;
								a[k]++;
								vec.pop_back();
							}
						}
						vec.pop_back();
						a[j]++;
					}
				}
				a[i] += 4;
				if (ans_got) break;
			}
		}
		if (ans_got)
		{
			ptoa(ans);
			return;
		}
		else {
			if (attack || lessthan2)
			{
				if (can_use_bomb)
				{
					ptoa(bom);
					return;
				}
				if (can_use_rocket)
				{
					ptoa(rock);
					return;
				}
			}
		}
		return;
	}
	case 8: {
		if (lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		short card = lastcombo->find_card();
		if (!attack && !yibo) return;
		if (4 > myCards.size() && attack)
		{
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		pcards ans = NULL;
		bool ans_got = 0;
		for (int i = card + 1; i <= 12; i++)
		{
			if (a[i] == 4)
			{
				a[i] -= 4;
				cardcombotype* p = new Bomb(i);
				double value = p->getvalue();
				strategy();
				if (mmin > renew_value)
				{
					mmin = renew_value;
					ans = p;
					ans_got = 1;
				}
				a[i] += 4;
				if (ans_got) break;
			}
		}
		if (ans_got)
		{
			ptoa(ans);
			return;
		}
		else {
			if (attack || lessthan2)
			{
				if (can_use_bomb)
				{
					ptoa(bom);
					return;
				}
				if (can_use_rocket)
				{
					ptoa(rock);
					return;
				}
			}
		}
		return;
	}
	case 7: {
		if (lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		short card = lastcombo->find_card();
		if (!attack && !yibo)
		{
			if (card >= 10) return;
		}
		if (5 > myCards.size() && attack)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		bool ans_got = 0;
		pcards ans = NULL;
		for (int i = card + 1; i <= 12; i++)
		{
			if (a[i] >= 3)
			{
				a[i] -= 3;
				for (int j = 0; j <= 14; j++)
				{
					if (j == i) continue;
					if (a[j] >= 2)
					{
						a[j] -= 2;
						pcards p = new Triplet2(i, j);
						double value = p->getvalue();
						strategy();
						if (mmin > renew_value)
						{
							mmin = renew_value;
							ans = p;
							ans_got = 1;
						}
						a[j] += 2;
					}
				}
				a[i] += 3;
			}
		}
		if (ans_got)
		{
			ptoa(ans);
			return;
		}
		if (attack || lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		return;
	}
	case 6: {
		if (lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		short card = lastcombo->find_card();
		if (!attack && !yibo)
		{
			if (card >= 10) return;
		}
		if (4 > myCards.size() && attack)
		{
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		bool ans_got = 0;
		pcards ans = NULL;
		for (int i = card + 1; i <= 12; i++)
		{
			if (a[i] >= 3)
			{
				a[i] -= 3;
				for (int j = 0; j <= 14; j++)
				{
					if (j == i) continue;
					if (a[j])
					{
						a[j] -= 1;
						pcards p = new Triplet1(i, j);
						double value = p->getvalue();
						strategy();
						mystrategy;
						if (mmin > renew_value)
						{
							mmin = renew_value;
							ans = p;
							ans_got = 1;
						}
						a[j] += 1;
					}
				}
				a[i] += 3;
			}
		}
		if (ans_got)
		{
			ptoa(ans);
			return;
		}
		if (attack || lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		return;
	}
	case 5: {
		if (lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		short card = lastcombo->find_card();
		if (!attack && !yibo)
		{
			if (card >= 10) return;
		}
		if (3 > myCards.size() && attack)
		{
			if (can_use_rocket)
			{
				ptoa(rock);
			}
			return;
		}
		bool ans_got = 0;
		pcards ans = NULL;
		for (int i = card + 1; i <= 12; i++)
		{
			if (a[i] >= 3)
			{
				a[i] -= 3;
				pcards p = new Triplet(i);
				double value = p->getvalue();
				strategy();
				if (mmin > renew_value)
				{
					mmin = renew_value;
					ans = p;
					ans_got = 1;
				}
				a[i] += 3;
			}
		}
		if (ans_got)
		{
			ptoa(ans);
			return;
		}
		if (attack || lessthan2)
		{
			if (can_use_bomb && bom != NULL)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket && rock != NULL)
			{
				ptoa(rock);
				return;
			}
		}
		return;
	}
	case 2: {
		short card = lastcombo->find_card();
		if (lessthan2)
		{
			if (can_use_bomb && bom != NULL)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket && rock != NULL)
			{
				ptoa(rock);
				return;
			}
		}
		if (!attack && !yibo)
		{
			if (card >= 10) return;
		}
		if (2 > myCards.size()) return;
		bool ans_got = 0;
		pcards ans = NULL;
		if (card_attack == 2 || card_attack == 3)
		{
			for (int i = 12; i >= card + 1; i--)
			{
				if (a[i] >= 2)
				{
					a[i] -= 2;
					pcards p = new Pair(i);
					double value = p->getvalue();
					strategy();
					if (mmin >= renew_value)
					{
						mmin = renew_value;
						ans = p;
						ans_got = 1;
					}
					a[i] += 2;
					if (ans_got)
					{
						ptoa(ans);
						return;
					}
				}
			}
			if (attack || lessthan2)
			{
				if (can_use_bomb && bom != NULL)
				{
					ptoa(bom);
					return;
				}
				if (can_use_rocket && rock != NULL)
				{
					ptoa(rock);
					return;
				}
			}
		}
		short acd = 15;
		if (!mystrategy.st[2].empty() && myCards.size() > 17)
		{
			for (pcards p : mystrategy.st[2])
			{
				if (p->find_card() > card && p->find_card() < acd)
				{
					ans = p;
					acd = p->find_card();
				}
			}
			if (acd != 15 && acd != 12)
			{
				ptoa(ans);
				return;
			}
			else if (acd == 12)
			{
				return;
			}
		}
		for (int i = card + 1; i <= 12; i++)
		{
			if (a[i] >= 2)
			{
				a[i] -= 2;
				pcards p = new Pair(i);
				double value = p->getvalue();
				strategy();
				mystrategy;
				if (mmin > renew_value)
				{
					mmin = renew_value;
					ans = p;
					ans_got = 1;
				}
				a[i] += 2;
			}
		}
		if (ans_got)
		{
			ptoa(ans);
			return;
		}
		if (attack || lessthan2)
		{
			if (can_use_bomb && bom != NULL)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket && rock != NULL)
			{
				ptoa(rock);
				return;
			}
		}
		return;
	}
	case 1: {

		short card = lastcombo->find_card();
		if (!attack && (!yibo))
		{
			if (card >= 11) return;
		}
		bool ans_got = 0;
		pcards ans = NULL;
		if (lessthan2)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		if (card_attack <= 2)
		{
			for (int i = 14; i >= card + 1; i--)
			{
				if (a[i])
				{
					a[i] -= 1;
					pcards p = new Single(i);
					double value = p->getvalue();
					strategy();
					if (mmin > renew_value)
					{
						mmin = renew_value;
						ans = p;
						ans_got = 1;
					}
					a[i] += 1;
					if (ans_got)
					{
						ptoa(ans);
						return;
					}
				}
			}
			if (attack || lessthan2)
			{
				if (can_use_bomb && bom != NULL)
				{
					ptoa(bom);
					return;
				}
				if (can_use_rocket && rock != NULL)
				{
					ptoa(rock);
					return;
				}
			}
		}
		if (!mystrategy.st[1].empty() && myCards.size() > 17)
		{
			for (pcards p : mystrategy.st[1])
			{
				if (p->find_card() > card && p->find_card() <= 11)
				{
					ans = p;
					ptoa(ans);
					return;
				}
			}
		}
		for (int i = card + 1; i <= 14; i++)
		{
			if (a[i])
			{
				a[i] -= 1;
				pcards p = new Single(i);
				strategy();
				if (mmin > renew_value)
				{
					mmin = renew_value;
					ans = p;
					ans_got = 1;
				}
				a[i] += 1;
			}
		}
		if (ans_got)
		{
			ptoa(ans);
			return;
		}
		if (attack || lessthan2)
		{
			if (can_use_bomb && bom != NULL)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket && rock != NULL)
			{
				ptoa(rock);
				return;
			}
		}
		return;
	}
	case 3: {
		if (lessthan2)
		{
			if (can_use_bomb && bom != NULL)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket && rock != NULL)
			{
				ptoa(rock);
				return;
			}
		}
		short card = lastcombo->find_card();
		short length = lastcombo->find_length();
		if (myCards.size() < length && attack)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		bool ans_got = 0;
		pcards ans = NULL;
		for (int i = card + 1; i <= 12 - length; i++)
		{
			int flag = 1;
			for (int j = 0; j < length; j++)
			{
				if (a[j + i] == 0)
				{
					flag = 0;
					i = i + j;
					break;
				}
			}
			if (flag)
			{
				for (int j = 0; j < length; j++)
				{
					a[i + j] -= 1;
				}

				pcards p = new Straight(i, length);
				strategy();
				mystrategy;
				if (mmin > renew_value)
				{
					mmin = renew_value;
					ans = p;
					ans_got = 1;
				}

				for (int j = 0; j < length; j++)
				{
					a[i + j] += 1;
				}
			}
		}
		if (ans_got)
		{
			ptoa(ans);
			return;
		}
		if (attack || lessthan2)
		{
			if (can_use_bomb && bom != NULL)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket && rock != NULL)
			{
				ptoa(rock);
				return;
			}
		}
		return;
	}
	case 4: {
		if (lessthan2)
		{
			if (can_use_bomb && bom != NULL)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket && rock != NULL)
			{
				ptoa(rock);
				return;
			}
		}
		short card = lastcombo->find_card();
		short length = lastcombo->find_length();
		if (myCards.size() < length * 2 && attack)
		{
			if (can_use_bomb)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket)
			{
				ptoa(rock);
				return;
			}
		}
		bool ans_got = 0;
		pcards ans = NULL;
		for (int i = card + 1; i <= 12 - length; i++)
		{
			int flag = 1;
			for (int j = 0; j < length; j++)
			{
				if (a[j + i] < 2)
				{
					flag = 0;
					i = i + j;
					break;
				}
			}
			if (flag)
			{
				for (int j = 0; j < length; j++)
				{
					a[i + j] -= 2;
				}
				pcards p = new Straight2(i, length);
				strategy();
				mystrategy;
				if (mmin > renew_value)
				{
					mmin = renew_value;
					ans = p;
					ans_got = 1;
				}

				for (int j = 0; j < length; j++)
				{
					a[i + j] += 2;
				}
			}
		}
		if (ans_got)
		{
			ptoa(ans);
			return;
		}
		if (attack || lessthan2)
		{
			if (can_use_bomb && bom != NULL)
			{
				ptoa(bom);
				return;
			}
			if (can_use_rocket && rock != NULL)
			{
				ptoa(rock);
				return;
			}
		}
		return;
	}
	default:return;
	}
}