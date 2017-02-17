#pragma once

#include "config.h"
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <vector>

using std::priority_queue;
using std::string;
using std::map;
using std::vector;
using std::shared_ptr;

class HuffmanNodeBase;
class HuffmanLeafNode;
class HuffmanInsiderNode;

const int TypeOfNodebase = 0;
const int TypeOfInsiderNode = 1;
const int TypeOfLeafNode = 2;

/*
 * 霍夫曼树节点的虚基类，不允许实例化。
 */
class HuffmanNodeBase {
public:
  unsigned long long frequency = 0xffffffffffffffff;
  HuffmanNodeBase *parent = nullptr;
  /*
   * 重载运算符 <
   */
  bool operator<(HuffmanNodeBase *other);

  virtual int getType();

  /*
   * 虚析构函数，保证子类被delete时可以自动销毁孩子节点
   */
  virtual ~HuffmanNodeBase();
};

/*
 * 内部节点类
 */
class HuffmanInsiderNode : public HuffmanNodeBase {
public:
  HuffmanNodeBase *left = nullptr, *right = nullptr;

  /*
   * 结合叶子节点和内部节点 或 结合内部节点和内部节点
   * 返回生成的父节点
   */
  HuffmanNodeBase *combine(HuffmanInsiderNode *other);
  HuffmanNodeBase *combine(HuffmanLeafNode *other);

  virtual int getType();

  virtual ~HuffmanInsiderNode();
};

/*
 * 叶子节点类
 */
class HuffmanLeafNode : public HuffmanNodeBase {
public:
  unsigned char charactor;

  HuffmanLeafNode(unsigned char charactor, unsigned long long weight);

  /*
   * 结合叶子节点和内部节点 或 结合叶子节点和叶子节点
   * 返回生成的父节点
   */
  HuffmanNodeBase *combine(HuffmanInsiderNode *other);
  HuffmanNodeBase *combine(HuffmanLeafNode *other);

  virtual int getType();

  virtual ~HuffmanLeafNode();
};

/*
 * HuffmanNodeBase 的比较函数，用于优先队列中
 */
typedef shared_ptr<HuffmanNodeBase> ptrToNode;
class compare {
public:
  bool operator()(ptrToNode a, ptrToNode b);
};

class ReflectionCom {
public:
  bool operator()(std::pair<unsigned char, string> a,
                  std::pair<unsigned char, string> b);
};

typedef map<unsigned char, string> ReflectionType;
/*
 * 霍夫曼树,唯一构造函数接受一个字符频率表作为输入，最后通过其他函数得到编码映射
 */
class HuffmanTree {
private:
  priority_queue<ptrToNode, vector<ptrToNode>, compare> tree, _backup;
  ReflectionType reflect;
  unsigned long long *frequency;

public:
  // constructor, add leaf node to tree
  HuffmanTree(unsigned long long *frequency);

  // 合并优先队列里的所有节点，构建Huffman树
  void buildTree();

  //遍历整颗树，并断言节点大于等于3(不考虑压缩只有字符数极少的文件)
  void travelTree();

  //执行上述函数返回从源字符到二进制码的映射
  ReflectionType getReflection();

  //打印树的信息
  void showTree();

  //打印最终编码信息
  void showReflect();
};
