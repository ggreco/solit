#ifndef GAME_H

#define GAME_H

#include "deck.h"
#include "row.h"
#include "seed.h"
#include "renderer.h"
#include "action.h"
#include <stack>
#include <map>

#define SAVEGAME_NAME "_saved_game"

enum GameId {KLONDIKE_ID, SPIDER_ID, NUMBER_OF_GAMES};

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
    int linked;

    Move(Stackable *from, int srcpos, 
         Stackable *to = NULL,  int destpos = 0, 
         const Stackable *c = NULL, int link = -1) :
         source(from), source_pos(srcpos),
         dest(to), dest_pos(destpos),
         linked(link)
         {
             if (c)
                 cards = *c;
         }
};

class MoveList 
{
    std::stack<Move> moves_;
public:

    void Revert();

    bool Empty() const { return moves_.empty(); }
    size_t Size() const { return moves_.size(); }
    void Clear() { while(!moves_.empty()) moves_.pop(); }
    void Add(Stackable &from, int srcpos, Stackable &to, int destpos, const Card &c, int link = -1) {
        Stackable st;

        st.Add(c);
        moves_.push(Move(&from, srcpos,  &to, destpos, &st, link));
    }

    void Add(Stackable &from, int srcpos, Stackable &to, int destpos, const Stackable &c, int link = -1) {
        moves_.push(Move(&from, srcpos,  &to, destpos, &c, link));
    }

    void Add(Row &from, int position) {
        moves_.push(Move(&from, position));
    }
};

typedef std::map<int, std::pair<int, int> > HighScoreMap;

class Game : public ActionManager, public Serializable
{
protected:
	
	typedef struct
	{
	    int x;
		int y;
	} PosData;
	

    enum StatusValue {PLAYING, PLAYING_VICTORY, AUTOCOMPLETE};

    int status_;
    MoveList moves_;
    Renderer *rend_;
    Selection selection_;
    Deck deck_;

    static int game_id_;

    static HighScoreMap scores_;

    bool restarted_;

    virtual bool IsCompleted() = 0;

    void MouseMove(const Point &);
	void KeyPress(char key) {}
    virtual void DoubleClick(const Point &p) {}
    void UndoMove();
	virtual void ReleaseButton(const Point &);
	virtual void KeyRelease(char);
    void Exposed();
    void OnQuit();


    void Save();

// names on the filesystem
    static std::string DirName();
    static std::string GameName();
    static std::string ScoreName();

	static PosData data_[];
public:

    Renderer *Rend() { return rend_; }
    virtual void Restart();
   	void Victory(Stackable &, int, int);
    virtual void Update() = 0;
    virtual void SetupCards() = 0;
    Game(int game_id, int res_id, int columns, int seeds = -1, bool card_slot = false);
    virtual ~Game() {};

    static void update_scores();
    void check_scores();
    void read_scores();

    bool check_highlight(int pos);
    void startup();
    void draw_selection(const Point &);

    virtual void serialize(Serializer& p) {
        p & status_;
//        p & moves_; we cannot go behind the save point
        p & selection_;
        p & deck_;
    }
};

#endif
