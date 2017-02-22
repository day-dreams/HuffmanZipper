#include "huffmanZipper.h"
#include "config.h"
#include "huffmanTree.h"
#include <bitset>
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
  // int timer = 0;
  while (!binary_file.eof()) {
    binary_file.get((char &)c);
    // std::cout << setbase(10) << ++timer << "  " << setbase(10)
    //           << (unsigned short)c << "  " << c << '\n';
    ++frequency[c];
  } //
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
    // std::cout << entry.second<<"--"<<entry.first << '\n';
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
  // std::cout << "\njson during compress:" << json_str << '\n';
  // 2.写入源文件，每次读取一个字节,写入文件
  std::ifstream input;
  input.open(input_filename, std::fstream::binary);
  char c, code = 0;
  int left = 0;
  bool jump_flag = false;
  while (!input.eof()) {
    input.get(c);
    auto encode = reflection[c];
    // std::cout << "get a char:" << std::setbase(16) << (unsigned short)(c)
    //           << " encode:" << encode << '\n';
    int current = 0;               //指向当前正要写入的bit
    if (left != 0) {               //上次有遗留的bit没有写进去
      while (current + left < 8) { //如果encode足够，补齐8bit后跳出
        if (current == encode.length()) {
          jump_flag = true;
          left += current;
          break;
        }
        // std::cout << " code:" << std::setbase(16) << (unsigned short)code
        //           << " encode[current]:" << encode[current]
        //           << " 7-left-current:" << (7 - left - current)
        //           << "  encode[current]<<(7-left-current):"
        //           << ((uint8_t)(encode[current] - '0') << (7 - left -
        //           current))
        //           << '\n';
        code += (encode[current] - '0') << (7 - left - current);
        // std::cout << "_code:" << std::setbase(16) << (unsigned short)code
        //           << " encode[current]:" << encode[current]
        //           << " 7-left-current:" << (7 - left - current) << '\n';
        current++;
      }
      if (jump_flag) {
        // std::cout << "jumping.." << '\n';
        jump_flag = false;
        continue;
      }
      output.write(&code, 1);
      left = 0;
      // std::cout << "InLine:" << std::setbase(10) << __LINE__
      //           << " writing: " << std::setbase(16) << (unsigned short)(c)
      //           << " CODE:" << (unsigned short)(code) << '\n';
    } //消除了遗留或者是没有遗留
    // while ((encode.length() - current) % 8 == 0 &&
    //        (encode.length() != current)) { //可以直接写入整块的char
    while ((encode.length() - current) > 8 &&
           (encode.length() != current)) { //可以直接写入整块的char
      code = 0;
      for (int i = 0; i != 8; ++i) {
        // std::cout << "InLine:" << std::setbase(10) << __LINE__
        //           << "code:" << std::setbase(16) << (unsigned short)code
        //           << '\n';
        code = code << 1;
        // std::cout << "InLine:" << std::setbase(10) << __LINE__
        //           << "code:" << std::setbase(16) << (unsigned short)code
        //           << '\n';
        code += (encode[current] - '0');
        current++;
        // std::cout << "_InLine:" << std::setbase(10) << __LINE__
        //           << "code:" << std::setbase(16) << (unsigned short)code
        //           << '\n';
      }
      output.write(&code, 1);
      // std::cout << "InLine:" << std::setbase(10) << __LINE__
      //           << " writing: " << std::setbase(16) << (unsigned short)(c)
      //           << " CODE:" << (unsigned short)(code) << '\n';
    }
    if (current < encode.length()) { //仍然有遗留，把他们冲到code里
      code = 0;
      for (int i = 0; current < encode.length(); ++i) {
        // std::cout << "InLine:" << std::setbase(10) << __LINE__
        //           << "code:" << std::setbase(16) << (unsigned short)code
        //           << '\n';
        code += (encode[current] - '0') << (7 - i);
        current++;
        // std::cout << "_InLine:" << std::setbase(10) << __LINE__
        //           << "code:" << std::setbase(16) << (unsigned short)code
        //           << '\n';
        ++left;
      }
      //遗留清理完毕，读取下一个字符
    }
  }
  //可能还有多余的遗留，凑不够8bit,但是code已经经过了补0，所以直接写入
  if (left != 0) {
    output.write(&code, 1);
    // std::cout << "InLine:" << std::setbase(10) << __LINE__
    //           << " writing: " << std::setbase(16) << (unsigned short)(code)
    //           << " CODE:" << (unsigned short)(code) << '\n';
  }
}

/*
 * 假设bits是文件最后一个字节，来判断一个bit有没有发生补齐
 */
bool _polishing_zero(std::bitset<8> &bits, int index) {
  for (int i = index; i >= 0; --i) {
    if (bits[i])
      return false;
  }
  // std::cout << "怎么没补齐？" << bits << " index:" << index << '\n';
  return true;
}

bool HuffmanZipper::decompress(std::string input_filename,
                               std::string output_filename) {
  using namespace std;
  ifstream input;
  input.open(input_filename, fstream::binary | fstream::in);
  char buffer[4];

  // 0.读取magic number
  input.read(buffer, 4);
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
  input.read(buffer, 4);
  int json_size = 0;
  for (int i = 0; i != 4; ++i) {
    json_size += (uint8_t)buffer[i] << (24 - 8 * i);
  }

  // 2. 读取json串，并转换成 map<string,unsigned char> {"010101":'a'}
  vector<char> json_buffer(json_size, 0);
  input.read(&json_buffer[0], json_size);
  auto x = nlohmann::json::parse(json_buffer);
  // std::cout << "\njson during decompress:" << x << '\n';
  map<string, unsigned char> reflection;
  for (auto ite = x.begin(); ite != x.end(); ++ite)
    reflection[ite.key()] = ite.value();
  // for (auto &entry : reflection)
  //   std::cout << entry.first << "  " << setbase(16)
  //             << (unsigned int)entry.second << '\n';

  // 3.解压源文件
  ofstream output;
  output.open(output_filename, fstream::binary);
  if (!output) { //打开文件失败
    return false;
  }
  std::string code(20, '0'); //预先分配好内存，避免后续动态增长
  code.clear();
  char c, char_buffer;
  bitset<8> bits;
  input.get(char_buffer);
  while (input && !input.eof()) {
    c = char_buffer;
    // cout << "decompress--got a char:" << setbase(16) << (unsigned short)c
    //      << "\n";
    input.get(char_buffer);
    // std::cout << "eof?" << input.eof() << '\n';
    bits = bitset<8>(c);
    for (int i = 7; i >= 0; --i) {
      code.push_back(bits[i] ? '1' : '0');
      if (reflection.find(code) != reflection.end()) { //匹配成功
        // std::cout << "eof?:" << output.eof() << " bits:" << bits
        //           << " code:" << code << " raw char:" << setbase(16)
        //           << (unsigned short)reflection[code] << " i:" << i << '\n';
        if (_polishing_zero(bits, i - 1) && input.eof()) { //补齐
          // std::cout << "polishing..." << '\n';
          c = reflection[code];
          output.write(&c, 1);
          return true;
        } else { //不是补齐，直接写入
          // std::cout << "not polishing..." << '\n';
          c = reflection[code];
          output.write(&c, 1);
          code.clear();
        }
      }
    }
  }

  return true;
}
