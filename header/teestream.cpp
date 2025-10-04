#include "teestream.h"


#include <iostream>
#include <ostream>

// https://stackoverflow.com/a/33366028


teestream::teestream(std::ostream & o1, std::ostream & o2)
  : std::ostream(&tbuf)
  , tbuf(o1.rdbuf(), o2.rdbuf())
{
}



