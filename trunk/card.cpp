#include "card.h"
#include <sstream>

std::string Card::
str() const
{
    std::ostringstream name;

    switch (value_) {
        case 1:
            name << "Ace";
            break;
        case 11:
            name << "Jack";
            break;
        case 12:
            name << "Queen";
            break;
        case 13:
            name << "King";
            break;
        default:
            name << value_;
    }
            
    name << " of ";

    switch (seed_) {
        case Pikes:
            name << "Pikes";
            break;
        case Hearts:
            name << "Hearts";
            break;
        case Squares:
            name << "Squares";
            break;
        case Flowers:
            name << "Flowers";
            break;
    }

    return name.str();
}
