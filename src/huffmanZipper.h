#pragma once
#include "config.h"
#include <string>
#include <vector>

class HuffmanZipper {
public:
  unsigned long long frequency[MAX_SYMBOL_SIZE] = {0};
  char cache[MAX_CACHE_SIZE] = {'0'};

  //构造、析构函数，什么也不做
  HuffmanZipper();
  ~HuffmanZipper();

  //打印频路分布信息
  void showFrequency();

  // 1.读取filename代表的文件，统计字符频率信息，存放在frequency中
  void setFrequency(std::string intput_filename);

  // 2.开始压缩
  bool compress(std::string input_filename, std::string output_filename);

  // 3.解压
  bool decompress(std::string intput_filename);
};
