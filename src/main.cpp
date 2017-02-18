#include "config.h"
#include "huffmanTree.h"
#include "huffmanZipper.h"
#include <fstream>
#include <iostream>
using namespace std;

int main(int argc, char const *argv[]) {
  HuffmanZipper zipper;
  zipper.setFrequency(argv[1]);
  zipper.compress(argv[1], "helloworld.txt");
  zipper.decompress("helloworld.txt");
  return 0;
}
