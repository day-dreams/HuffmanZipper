#include "config.h"
#include "huffmanTree.h"
#include "huffmanZipper.h"
#include <fstream>
#include <iostream>
using namespace std;

int main(int argc, char const *argv[]) {
  HuffmanZipper zipper;
  zipper.setFrequency(argv[1]);
  HuffmanTree tree(zipper.frequency);
  tree.buildTree();
  tree.travelTree();
  //tree.showReflect();
  return 0;
}
