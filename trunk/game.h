#ifndef GAME_H

#define GAME_H

#define COLUMNS 12
#define TOTAL_DECKS 2
#define TOTAL_SEEDS (TOTAL_DECKS * 4)

#include "deck.h"
#include "row.h"
#include "seed.h"
#include "renderer.h"
#include "action.h"
#include <stack>

class Renderer;
class Point;

class Selection : public Stackable
{
    Stackable *origin_;
    int position_;
public:
    Selection() : origin_(NULL) {}
    void Add(const Card &c, Stackable &orig, int p) { origin_ = &orig; position_ = p; Stackable::Add(c); }
    void AddFirst(const Card &c, Stackable &orig, int p) { origin_ = &orig; position_ = p; Stackable::AddFirst(c); }
    void Remove() { origin_ = NULL; Stackable::Remove(); }
    Stackable *Origin() const { return origin_; }
    int OriginPosition() const { return position_; }
};

class Move
{
public:
    Stackable *source;
    int source_pos;
    Stackable *dest;
    int dest_pos;
    Stackable cards;

    Move(Stackable *from, int srcpos, 
         Stackable *to = NULL,  int destpos = 0, 
         const Stackable *c = NULL) :
         source(from), source_pos(srcpos),
         dest(to), dest_pos(destpos) 
         {
             if (c)
                 cards = *c;
         }
};

class MoveList 
{
    std::stack<Move> moves_;
public:

    void Revert(Renderer *);

    void Clear() { while(!moves_.empty()) moves_.pop(); }
    void Add(Stackable &from, int srcpos, Stackable &to, int destpos, const Card &c) {
        Stackable st;

        st.Add(c);
        moves_.push(Move(&from, srcpos,  &to, destpos, &st));
    }

    void Add(Stackable &from, int srcpos, Stackable &to, int destpos, const Stackable &c) {
        moves_.push(Move(&from, srcpos,  &to, destpos, &c));
    }

    void Add(Row &from, int position) {
        moves_.push(Move(&from, position));
    }
};

class Game : public ActionManager
{
    enum StatusValue {PLAYING, PLAYING_VICTORY};

    Renderer *rend_;
    Deck deck_;
    Stackable cards_;
    Row  rows_[COLUMNS];
    Seed seeds_[TOTAL_SEEDS];
    Selection selection_;
    MoveList moves_;
    StatusValue status_;

    void MouseMove(const Point &);
    void PressButton(const Point &);
    void ReleaseButton(const Point &);
    void DoubleClick(const Point &);
	void KeyPress(char key) {}
	void KeyRelease(char key);
	void AutoComplete();
	void Victory();
	bool SeedsFull();
    void UndoMove();
    bool IsCompleted();
public:
	void Restart();
    void Update();
    Game(Renderer *);
    ~Game() {};
};

#endif
