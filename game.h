#ifndef GAME_H

#define GAME_H

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
protected:
    enum StatusValue {PLAYING, PLAYING_VICTORY, AUTOCOMPLETE};

    StatusValue status_;
    MoveList moves_;
    Renderer *rend_;
    Selection selection_;
    Deck deck_;

    virtual bool IsCompleted() = 0;

    void MouseMove(const Point &);
	void KeyPress(char key) {}
    virtual void DoubleClick(const Point &p) {}
    void UndoMove();
	virtual void ReleaseButton(const Point &);
public:

    Renderer *Rend() { return rend_; }
    virtual void Restart();
   	void Victory();
    virtual void Update() = 0;
    Game(int rend_id, int columns, int seeds = -1, bool card_slot = false);
    virtual ~Game() {};
};

#endif
