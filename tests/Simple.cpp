#include <ByteStream.hpp>
#include <cassert>
using namespace bs;
int main()
{
  ByteStream byteStream;
  int x = 42;
  int y = 8;
  int z = x * y;
  byteStream.write<const int &>(x);
  byteStream.write<const int &>(y);
  byteStream.write<const int &>(z);
  int x2 = 0;
  int y2 = 0;
  int z2 = 0;
  unsigned long bytesRead = 0;
  byteStream.read(x2, bytesRead);
  byteStream.read(y2, bytesRead);
  byteStream.read(z2, bytesRead);
  assert(x == x2 && y == y2 && z == z2);
};