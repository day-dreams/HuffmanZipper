#include "huffmanTree.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <typeinfo>
#include <vector>

bool HuffmanNodeBase::operator<(HuffmanNodeBase *other) {
  return this->frequency > other->frequency;
}

HuffmanNodeBase *HuffmanInsiderNode::combine(HuffmanInsiderNode *other) {
  auto parent = new HuffmanInsiderNode();
  parent->frequency = this->frequency + other->frequency;
  if (this->frequency < other->frequency) {
    parent->left = this;
    parent->right = other;
  } else {
    parent->left = other;
    parent->right = this;
  }
  parent->left->parent = parent;
  parent->right->parent = parent;
  return parent;
}

HuffmanNodeBase *HuffmanInsiderNode::combine(HuffmanLeafNode *other) {
  auto parent = new HuffmanInsiderNode();
  parent->frequency = this->frequency + other->frequency;
  if (this->frequency < other->frequency) {
    parent->left = this;
    parent->right = other;
  } else {
    parent->left = other;
    parent->right = this;
  }
  parent->left->parent = parent;
  parent->right->parent = parent;
  return parent;
}

int HuffmanNodeBase::getType() { return TypeOfNodebase; }

int HuffmanInsiderNode::getType() { return TypeOfInsiderNode; }

HuffmanLeafNode::HuffmanLeafNode(unsigned char charactor,
                                 unsigned long long weight) {
  this->charactor = charactor;
  this->frequency = weight;
}

HuffmanNodeBase *HuffmanLeafNode::combine(HuffmanInsiderNode *other) {
  auto parent = new HuffmanInsiderNode();
  parent->frequency = this->frequency + other->frequency;
  if (this->frequency < other->frequency) {
    parent->left = this;
    parent->right = other;
  } else {
    parent->left = other;
    parent->right = this;
  }
  parent->left->parent = parent;
  parent->right->parent = parent;
  return parent;
}

HuffmanNodeBase *HuffmanLeafNode::combine(HuffmanLeafNode *other) {
  auto parent = new HuffmanInsiderNode();
  parent->frequency = this->frequency + other->frequency;
  if (this->frequency < other->frequency) {
    parent->left = this;
    parent->right = other;
  } else {
    parent->left = other;
    parent->right = this;
  }
  parent->left->parent = parent;
  parent->right->parent = parent;
  return parent;
}

int HuffmanLeafNode::getType() { return TypeOfLeafNode; }

HuffmanNodeBase::~HuffmanNodeBase() {
  // std::cout << "-a base node destroyed." << '\n';
}

HuffmanInsiderNode::~HuffmanInsiderNode() {
  // std::cout << "---a insider node destroyed" << '\n';
}

HuffmanLeafNode::~HuffmanLeafNode() {
  // std::cout << "---a leaf node destroyed" << '\n';
}

HuffmanTree::HuffmanTree(unsigned long long *frequency) {
  this->frequency = frequency;
  for (int i = 0; i != MAX_SYMBOL_SIZE; ++i) {
    if (frequency[i] != 0) {
      // auto p = static_cast<ptrToNode>(
      // std::cout << "before show: "<<i<<"  "<<frequency[i] << '\n';
      HuffmanNodeBase *p = new HuffmanLeafNode((unsigned char)i, frequency[i]);
      auto x = shared_ptr<HuffmanNodeBase>(p);
      tree.push(x);
      _backup.push(x);
    }
  }
  // this->showTree();
}

void HuffmanTree::showTree() {
  std::cout << "\n\n" << '\n';
  // auto copy = tree;
  auto _tree = tree;
  while (!_tree.empty()) {
    auto x = _tree.top();
    std::cout << "his parent: " << x.get()->parent << ' ';
    if (x->getType() == TypeOfLeafNode)
      std::cout << std::setbase(10) << " charctor:"
                << (unsigned long long)static_cast<HuffmanLeafNode *>(x.get())
                       ->charactor
                << std::setbase(10) << " frequency:" << x->frequency << '\n';
    else
      std::cout << "an insider node " << x->frequency << '\n';
    _tree.pop();
  }
  std::cout << _tree.size() << '\n';
}

void HuffmanTree::buildTree() {
  while (tree.size() != 1) {
    // std::cout << tree.size() << '\n';
    auto x = tree.top();
    tree.pop();
    auto y = tree.top();
    tree.pop();
    HuffmanNodeBase *z = nullptr;
    // TODO:
    // 判断这里是否真的发生了对象截断或者是生成了新对象？不然怎么会有析构发生？严重怀疑combine函数
    //并没有！
    if (x->getType() == TypeOfInsiderNode) {
      auto a = static_cast<HuffmanInsiderNode *>(x.get());
      if (y->getType() == TypeOfInsiderNode) {
        auto b = static_cast<HuffmanInsiderNode *>(y.get());
        z = a->combine(b);
      } else {
        auto b = static_cast<HuffmanLeafNode *>(y.get());
        z = a->combine(b);
      }
    } else {
      auto a = static_cast<HuffmanLeafNode *>(x.get());
      if (y->getType() == TypeOfInsiderNode) {
        auto b = static_cast<HuffmanInsiderNode *>(y.get());
        z = a->combine(b);
      } else {
        auto b = static_cast<HuffmanLeafNode *>(y.get());
        z = a->combine(b);
      }
    }
    auto p = ptrToNode(z);
    tree.push(p);
    _backup.push(p);
  }
}

HuffmanNodeBase *_getLeftChild(HuffmanNodeBase *parent) {
  if (!parent)
    return nullptr;
  if (parent->getType() == TypeOfLeafNode)
    return nullptr;
  else {
    // auto x = dynamic_cast<HuffmanInsiderNode *>(parent.get());
    //一定是这里发生了类型转化！
    // return static_cast<ptrToNode>(x->left);
    return dynamic_cast<HuffmanInsiderNode *>(parent)->left;
  }
}

unsigned char _getCharactor(ptrToNode node) {
  //不做检查，断言node非空且是个叶子节点
  return static_cast<HuffmanLeafNode *>(node.get())->charactor;
}

void HuffmanTree::travelTree() {
  string binary_code;
  auto root = tree.top().get();
  tree.pop();
  HuffmanNodeBase *current = _getLeftChild(root);
  HuffmanNodeBase *prev = root;
  binary_code.push_back('0');
  while (_getLeftChild(current)) {
    current = _getLeftChild(current);
    binary_code.push_back('0');
  }
  // current 到达中序遍历的最前驱,添加到映射中；添加完后回溯一层
  // std::cout << binary_code << '\n';
  reflect[static_cast<HuffmanLeafNode *>(current)->charactor] = binary_code;
  prev = current;
  current = current->parent;
  binary_code.pop_back();

  while (current) {
    /*
      只要不是回溯到了根节点，这棵树就还没访问完；
      而且每次回溯都表明这个节点的左子树已经访问完了，所以不需要检查这一点
    */
    if (current->getType() == TypeOfLeafNode) {
      //这是个叶子节点，没有子树，应该编码后再回溯一层
      reflect[static_cast<HuffmanLeafNode *>(current)->charactor] = binary_code;
      // std::cout << "/* _treege */" << '\n';
      prev = current;
      current = current->parent;
      binary_code.pop_back();
    } else if (static_cast<HuffmanInsiderNode *>(current)->right == prev) {
      //右子树访问完毕，回溯一层，二进制代码需要丢弃末位
      prev = current;
      current = current->parent;
      binary_code.pop_back();
    } else {
      //继续访问右子树，且从右子树的最前驱开始
      prev = current;
      current = static_cast<HuffmanInsiderNode *>(current)->right;
      binary_code.push_back('1');
      while (_getLeftChild(current)) {
        prev = current;
        current = _getLeftChild(current);
        binary_code.push_back('0');
      }
    }
  }
  // std::cout <<"frequency:"<<current->frequency<<"
  // size:"<<binary_code.size()<<" binary_code:"<< binary_code << '\n';
}

ReflectionType HuffmanTree::getReflection() {
    return this->reflect;
}

void HuffmanTree::showReflect() {
  std::cout << "\n\n" << '\n';
  std::vector<std::pair<unsigned char, std::string>> _reflect(reflect.begin(),
                                                              reflect.end());
  std::sort(_reflect.begin(), _reflect.end(), ReflectionCom());
  for (auto &entry : _reflect)
    std::cout << std::setw(-20) << std::setfill('_') << entry.second
              << std::setbase(10) << " frequency:" << frequency[entry.first]
              << ' ' << std::setbase(10) << (unsigned short)entry.first << '\n';
  std::cout << "binary-code size: " << reflect.size() << '\n';
}

bool compare::operator()(ptrToNode a, ptrToNode b) {
  return a->frequency > b->frequency;
}

bool ReflectionCom::operator()(std::pair<unsigned char, string> a,
                               std::pair<unsigned char, string> b) {
  if (a.second.length() == b.second.length())
    return a.second < b.second;
  else
    return a.second.length() < b.second.length();
}

// TODO: 判断优先队列是否错误，目前的编码结果很是错误。
