#ifndef RENDERER_H

#define RENDERER_H

#include "game.h"
#include "card.h"
#include "row.h"
#include "deck.h"
#include "seed.h"
#include "action.h"

class Point
{
    int x_, y_;

public:
    Point() : x_(0), y_(0) {}
    Point(int x, int y) : x_(x), y_(y) {}
    int X() const { return x_; }
    int Y() const { return y_; }
    void X(int v) { x_ = v; }
    void Y(int v) { y_ = v; }

    Point & operator+(Point &p) { x_ += p.x_; y_ += p.y_; return *this; }
    Point & operator-(Point &p) { x_ -= p.x_; y_ -= p.y_; return *this; }
    bool operator==(Point &p) { return (x_ == p.x_ && y_ == p.y_); }
};

class CardRenderer
{
    public:
        virtual void Draw(const Card &, const Point &) = 0;
        virtual void Move(const Card &, const Point &) = 0;
        virtual void Refresh(const Card &) = 0;
        virtual void Clear(const Card &) = 0;
};

class Renderer
{
    public:
        enum {DeckPos = 0 , CardPos = 1, FirstSeedPos = 2, LastSeedPos = 9, FirstRow = 10, LastRow = 9 + COLUMNS};
        virtual bool Wait() = 0;
        virtual void Update() = 0;
        void Draw(const Row &, int pos);
        void Draw(const Stackable &, int pos);
        void Draw(const Card &c, const Point &p) { GetCardRenderer()->Draw(c, p); }
        void Clear(const Card &c) { GetCardRenderer()->Clear(c); }
        void SetActionManager(ActionManager *a) { action_ = a; }
        virtual CardRenderer *GetCardRenderer()  = 0;
        virtual void DrawEmpty(const Point &p) = 0;
        Renderer() : action_(NULL) {};

        void MouseMove(const Point &p) { if (action_) action_->MouseMove(p); }
        void PressButton(const Point &p) { if (action_) action_->PressButton(p); }
        void ReleaseButton(const Point &p) { if (action_) action_->ReleaseButton(p); }
        void DoubleClick(const Point &p) { if (action_) action_->DoubleClick(p); }
        int GetPosition(const Point &p);
        std::pair<int,int> GetRowInfo() { return std::make_pair(column_positions_[0].Y(), card_size_.Y()/3 ); }
        virtual ~Renderer() {};

    protected:
        Point deck_position_;
        Point cards_position_;
        Point card_size_;
        Point seed_positions_[8];
        Point column_positions_[COLUMNS];
        Point screen_size_;
    private:
        ActionManager *action_;
};

#endif
