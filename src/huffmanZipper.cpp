#include "huffmanZipper.h"
#include "config.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

HuffmanZipper::HuffmanZipper() {}

HuffmanZipper::~HuffmanZipper() {}

void HuffmanZipper::setFrequency(std::string filename) {
  using namespace std;
  ifstream binary_file;
  binary_file.open(filename, fstream::binary);
  if (!binary_file) {
    std::cout << "open" << filename << " faled." << '\n';
    return;
  }
  unsigned char c;
  int timer = 0;
  while (!binary_file.eof()) {
    binary_file.get((char &)c);
    // std::cout << setbase(10) << ++timer << "  " << setbase(10)
    //           << (unsigned short)c << "  " << c << '\n';
    ++frequency[c];
  }
  // cin.get();
  // std::cout << "over." << '\n';
  binary_file.close();
}
