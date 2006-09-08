#ifndef GAME_H

#define GAME_H

#define COLUMNS 12

#include "deck.h"
#include "row.h"
#include "seed.h"
#include "renderer.h"
#include "action.h"

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

class Game : public ActionManager
{
    Renderer *rend_;
    Deck deck_;
    Stackable cards_;
    Row  rows_[COLUMNS];
    Seed seeds_[8];
    Selection selection_;

    void MouseMove(const Point &);
    void PressButton(const Point &);
    void ReleaseButton(const Point &);
    void DoubleClick(const Point &);

public:
    void Update();
    Game(Renderer *);
    ~Game() {};
};

#endif
