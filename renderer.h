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
    void set(int x, int y) { x_ = x; y_ = y; }
    Point & operator+(const Point &p) { x_ += p.x_; y_ += p.y_; return *this; }
    Point & operator-(const Point &p) { x_ -= p.x_; y_ -= p.y_; return *this; }
    bool operator==(Point &p) { return (x_ == p.x_ && y_ == p.y_); }
};

class Rect
{
    Point pos_;
    Point size_;
public:
    Rect() : pos_(0,0), size_(0,0) {}
    Rect(int x, int y, int w, int h) : pos_(x, y), size_(w, h) {}
    void set(int x, int y, int w, int h) { pos_.set(x, y); size_.set(w, h); }
    int width() const { return size_.X(); }
    int height() const { return size_.Y(); }
    int X() const { return pos_.X(); }
    int Y() const { return pos_.Y(); }
    bool inside(int x, int y) const { return (x >= pos_.X() && x < (pos_.X() + size_.X()) &&
                                          y >= pos_.Y() && y < (pos_.Y() + size_.Y()) ); }
    bool inside(const Point &p) const { return inside(p.X(), p.Y()); }
};

inline Point operator-(const Point &p1, const Point &p2) { return Point(p1.X() - p2.X(),p1.Y() - p2.Y());  }

class CardRenderer
{
    public:
        virtual void Draw(const Card &, const Point &) = 0;
        virtual void Move(const Card &, const Point &) = 0;
        virtual void Refresh(const Card &) = 0;
        virtual void Clear(const Card &) = 0;
};

enum Widgets {NEW_GAME, QUIT_GAME, UNDO_MOVE, HIGHSCORE, WIDGET_NUM};

class Renderer
{
    public:
        enum {DeckPos = 0 , CardPos = 1, FirstSeedPos = 2, 
             LastSeedPos = (FirstSeedPos + TOTAL_SEEDS - 1), 
             FirstRow , LastRow = (FirstRow + COLUMNS - 1), 
             FirstWidget, LastWidget = (FirstWidget + WIDGET_NUM -1 )};
        virtual void Poll() = 0;
        virtual void Delay(int) = 0;
        virtual bool Wait() = 0;
		virtual void Update() = 0;
        virtual void UpdateAll() = 0;
		virtual void Clear() = 0;
        void Draw(const Row &, int pos);
        void Move(const Card &c, const Point &pos) { 
            GetCardRenderer()->Move(c, 
                Point( (int) ((double)pos.X() * scaling_), 
                       (int) ((double)pos.Y() * scaling_)));
        }
		void Move(const Card &c, int pos); 
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
		void KeyPress(const char key) { if (action_) action_->KeyPress(key); }
		void KeyRelease(char key) { if (action_) action_->KeyRelease(key); }
        int GetPosition(const Point &p);
        std::pair<int,int> GetRowInfo() { return std::make_pair(column_positions_[0].Y(), card_size_.Y() ); }
        virtual ~Renderer() {};

    protected:
        Rect widget_positions_[WIDGET_NUM];
        Point deck_position_;
        Point cards_position_;
        Point card_size_;
        Point seed_positions_[TOTAL_SEEDS];
        Point column_positions_[COLUMNS];
        Point screen_size_;
        double scaling_;
    private:
		bool inside(const Point &p, const Point &orig, const Point &size)
		{
			return (p.X() >= (orig.X() - card_size_.X()/3) && 
				    p.Y() >= (orig.Y() - card_size_.Y()/3) &&
			        p.X() <= (orig.X() + size.X() - card_size_.X()/4) &&
				    p.Y() <= (orig.Y() + size.Y()));
		}
        ActionManager *action_;
};

#endif
