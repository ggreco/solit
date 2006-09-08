#ifndef ROW_H

#define ROW_H

#include "stackable.h"

class Row : public Stackable
{
    public:
        Card GetCard() { Card c = Get(); Remove(); return c; } // temp
        Row() {}
        bool CanGet(const Card &c) 
        {
            if (Empty())
                return (c.Value() == 13);

            if (Get().Covered())
                return false;

            return (((Get().IsBlack() && c.IsRed()) ||
                     (Get().IsRed() && c.IsBlack()) ) &&
                      Get().Value() == (c.Value() + 1));
        }
};

#endif
