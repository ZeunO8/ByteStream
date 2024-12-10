# ByteStream

A library that implements a ByteStream that allows writing/reading types to a char pointer

Uses CMake for it's build system and comes with some included tests

### Building

Either use your preferred IDE of choice, or run the following commands to build

```
cmake -B build .
cmake --build build
```

### Testing

```
ctest --test-dir build --rerun-failed --output-on-failure -C Debug
```

### Basic Usage

```cpp
#include <ByteStream.hpp>
#include <cassert>

ByteStream byteStream;
int x = 128;
double y = 6.75;
long double z = x * y;
byteStream.write<const int &>(x);
byteStream.write<const double &>(y);
byteStream.write<const long double>(z);
int x2;
double y2;
long double z2;
unsigned long bytesRead = 0;
byteStream.read(x2, bytesRead);
byteStream.read(y2, bytesRead);
byteStream.read(z2, bytesRead);
assert(x == x2 && y == y2 && z == z2);
```

See tests for more usage examples

### License

Code is distributed under MIT license, feel free to use it in your proprietary projects as well.