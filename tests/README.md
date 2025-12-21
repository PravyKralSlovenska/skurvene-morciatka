cd /build
ctest alebo ./morciatko_tests

# Google Tests

```cpp
TEST(nieco, nieco_nazov)
{

}

TEST_F()
{

}
```

### FLAME GRAPHS
sudo pacman -S perf
sudo pacman -S flamegraph

1. make
2. perf record -F 99 -g ./morciatko
3. perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg
4. firefox flame.svg
