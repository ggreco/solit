#ifndef ROW_H

#define ROW_H

#include "stackable.h"

class Row : public Stackable
{
    public:
        Card GetCard() { Card c = Get(); Remove(); return c; } // temp
        Row() {}
		int HowManyCovered() {
			int c = 0;

			for (CardIterator it = cards_.begin(); it != cards_.end(); ++it)
				if (it->Covered())
					c++;

			return c;
		}
};

#endif
