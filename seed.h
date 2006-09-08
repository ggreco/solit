#ifndef SEED_H

#define SEED_H

#include "stackable.h"

class Seed : public Stackable
{
    public:
        bool CanGet(const Card &c) { 
            if (Empty())
                return (c.Value() == 1);

            return ( Get().Value() == (c.Value() - 1) &&
                     Get().Seed()  == c.Seed());
        }
};

#endif
