#include "huffmanZipper.h"
#include "config.h"
#include "huffmanTree.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <json/json.hpp>
#include <memory>
#include <string>

HuffmanZipper::HuffmanZipper() {}

HuffmanZipper::~HuffmanZipper() {}

void HuffmanZipper::showFrequency() {
  for (int i = 0; i != MAX_SYMBOL_SIZE; ++i)
    if (frequency[i] != 0)
      std::cout << i << "--" << frequency[i] << '\n';
}

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

bool HuffmanZipper::compress(std::string input_filename,
                             std::string output_filename) {
  using namespace nlohmann;
  HuffmanTree tree(this->frequency);
  // tree.showTree();
  tree.buildTree();
  tree.travelTree();
  //窃取了字符编码的映射,map<unsigned char,string> {'a':"010101"}
  auto reflection = std::move(tree.getReflection());
  json j;
  for (auto &entry : reflection) {
    j[entry.second] = (unsigned int)entry.first;
  }
  string json_str = j.dump();
  // std::cout << j << '\n';
  // std::cout << "----------" << '\n';
  // std::cout << json_str.length() << '\n';
  //打开文件
  std::ofstream output;
  output.open(output_filename, std::ofstream::binary);
  if (!output) {
    return false;
  }
  char buffer[4];

  // 0.写入magic number
  for (int i = 0; i != 4; ++i) {
    buffer[i] = MAGIC_NUM >> (24 - 8 * i);
  }
  output.write(buffer, 4);

  // 1.写入json大小，4字节int
  int size = json_str.size();
  // std::cout << "size: " << std::setbase(16) << size << '\n';
  for (int i = 0; i != 4; ++i) {
    buffer[i] = size >> (24 - 8 * i);
  }
  // std::cout << size << '\n';
  output.write(buffer, sizeof(size));

  // 1.写入json串，明文
  output << json_str;

  // 2.写入源文件，每次读取一个字节,写入文件
  std::ifstream input;
  input.open(input_filename, std::fstream::binary);
  char c;
  int left = -1; // left 指向上一个遗留bit
  char left_code = 0, current_code = 0;
  long long int current_size = 0;
  while (!input.eof()) {
    bool flag = true;
    input.get(c);
    std::cout << "current bit size:" << current_size++ << '\n';
    auto encode = reflection[c];
    int current = 0;
    if (left >= 0) {
      //上次有遗留bit没写入
      char _left_code = 0;
      for (; current + left + 1 < 8; ++current) {
        if (current >= encode.size()) { //本次写入结束，但仍然没有填满整个char
          std::cout << "-------------------------------" << '\n';
          left += encode.size();
          flag = false;
          break;
        }
        _left_code << 1;
        _left_code += encode[current] - '0';
        std::cout << "encode :" << encode << " current:" << current
                  << " in line:" << __LINE__ << " left:" << left << '\n';
      }
      if (!flag)
        continue;
      left_code += _left_code;
      output.write(&left_code, 1);
      std::cout << "writing: " << std::setbase(16) << (unsigned short)left_code
                << "in line:" << __LINE__ << '\n';
      left = -1;
    }
    //没有了遗留bit，开始新的bit,且只写入完整的char
    for (; encode.size() - current >= 8;) { //剩下的bit > 8bit
      std::cout << "encode " << encode << " current:" << current
                << " in line:" << __LINE__ << '\n';
      current_code = 0;
      for (int i = 0; i != 8; ++i, ++current) {
        current_code << 1;
        current_code += encode[current] - '0';
        std::cout << "encode " << encode << " current:" << current
                  << " in line:" << __LINE__ << '\n';
      }
      output.write(&current_code, 1);
      std::cout << "writing: " << std::setbase(16)
                << (unsigned short)current_code << "in line :" << __LINE__
                << '\n';
      std::cout << "encode " << encode << " current : " << current
                << " in line:" << __LINE__ << '\n';
    }
    //把剩下的bit写入left_code中，下次读入再写入
    left_code = 0;
    left = encode.length() - current;
    for (int i = 0; i != 8; ++i) {
      left_code << 1;
      if (current < encode.length()) {
        left_code += encode[current++] - '0';
        std::cout << "encode " << encode << " current:" << current
                  << " in line : " << __LINE__ << '\n';
      }
      std::cout << "encode " << encode << " current:" << current
                << " in line : " << __LINE__ << '\n';
    }
  }
  if (left >= 0) {
    //文件bit不是8的整数倍;由于每次更新left_code都会置0 所以直接写入
    output.write(&left_code, 1);
    std::cout << "writing :" << std::setbase(16) << (unsigned short)left_code
              << "in line:" << __LINE__ << '\n';
  }
  output.close();
  input.close();
  return true;
}

bool HuffmanZipper::decompress(std::string input_filename) {
  using namespace std;
  ifstream input_file;
  input_file.open(input_filename, fstream::binary | fstream::in);
  char buffer[4];

  // 0.读取magic number
  input_file.read(buffer, 4);
  int magic = 0;
  for (int i = 0; i != 4; ++i) {
    // std::cout << "buffer:" << setbase(16) << buffer[i] << " magic:" <<
    // magic
    //           << '\n';
    magic += (uint8_t)buffer[i] << (24 - 8 * i);
  }
  // std::cout << setbase(16) << magic << '\n';
  if (magic != MAGIC_NUM) {
    //不是本压缩算法产生的文件，放弃解压
    return false;
  }

  // 1.读取json串长度
  input_file.read(buffer, 4);
  int json_size = 0;
  for (int i = 0; i != 4; ++i) {
    json_size += (uint8_t)buffer[i] << (24 - 8 * i);
  }

  // 2. 读取json串，并转换成 map<string,unsigned char> {"010101":'a'}
  vector<char> json_buffer(json_size, 0);
  input_file.read(&json_buffer[0], json_size);
  auto x = nlohmann::json::parse(json_buffer);
  // std::cout << x << '\n';
  map<string, unsigned char> reflection;
  for (auto ite = x.begin(); ite != x.end(); ++ite)
    reflection[ite.key()] = ite.value();
  for (auto &entry : reflection)
    std::cout << entry.first << "  " << setbase(16)
              << (unsigned int)entry.second << '\n';
  // 3.解压源文件
  return true;
}

// if (encode.length() <= (8 - bit_index)) {
//   //本次写入不会超过当前的char
//   uint8_t code = 0;
//   for (int i = 0; i != encode.length(); ++i) {
//     code << 1;
//     code += encode[i] - '0';
//   }
//   for (int i = 0; i != (8 - bit_index - encode.length()); ++i)
//     code << 1;
//   write_buffer[byte_index] += code;
//   bit_index += encode.length();
// } else {
//   //本次写入会越界到后面的char，且可能越过不止一个
//   //将字符串分为三份[0,a) [a,b) [b,c],分别
//   //直接加到当前char、直接加到完整的后续char、加到后续的一个char(不完整)
//   int a = 8 - bit_index;
//   int b = (encode.length() - a) / 8 * 8 + a;
//   int c = encode.length() - 1;
//   uint8_t code = 0;
//   for (int i = 0; i != a; ++i) {
//     code << 1;
//     code += encode[i] - '0';
//   }
//   write_buffer[byte_index++] += code; //[0,a)
//   code = 0;
//   for (int i = 0; i != (b - a); i += 8) {
//     for (int j = 0; j != 8; ++j) {
//       code << 1;
//       code += encode[a + i + j] - '0';
//     }
//     write_buffer[byte_index++] += code; //[a,b)
//     code = 0;
//   }
//   code = 0; // ensure
//   for (int i = 0; i <= (c - b); ++i) {
//     code << 1;
//     code += encode[b + i];
//   }
//   for (int i = 0; i != (8 - c + b); ++i)
//     code << 1;
//   write_buffer[byte_index] += code; //[b,c]
//   bit_index = c - b + 1;
// }
// //判断时候要把buffer写入
