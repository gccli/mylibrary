#include <stdio.h>
#include <stdlib.h>

template <typename T>
class BNode {
public:
  BNode () 
    :parent(NULL)
    ,lchild(NULL)
    ,rchild(NULL)
    ,level(0)
    ,seqno(0)
  {}

public:
  BNode *parent, *lchild, *rchild;
  T     value;
  int   level; // depth or level
  int   seqno; // sequence number, 1-n

  int Insert(BNode<T>* node, int type)
  {
    if (node == NULL) return 1;
    
    if (type == 0)  // left
      this->lchild = node;
    else if (type == 1) // right
      this->rchild = node;

    node->parent = this;
    node->level  = this->level + 1;
  }

  int Delete();
};

template <typename T>
class BinaryTree 
{
public:
  BinaryTree()
    :root(NULL)
    ,depth(0)
  {}
  ~BinaryTree() {
  }

public:
  int Init(T* value)
  {
    root = new BNode<T>;
    root->level = 1;
    root->seqno = 1;
    root->value = value;
  }
  int Destroy();
  
  int Create(T* array, size_t size)
  {
    T* ptr = array;
    Init(ptr);
    
    BNode<T>* cur = root;

    for (int i=2; i<=size; ++i) {
      ptr = array+i-1;
      BNode<T>* node = new BNode<T>;
      node->value = *ptr;
      node->seqno = i;

      if (cur->*2 > i) {
	
      }

      cur->Insert(node, i%2);
    }
  }
  int Clear();

  bool IsEmpty();
  int Depth();
  int PreOrderTraverse(int (*visit)(T));
  
  BNode<T>* Root() { return root; }

private:
  BNode<T> *root;
  int depth;
  
};

int main(int argc, char* argv[])
{
  int array[12] = {1,2,3,4,5,6,7,8,9,10,11,12};

  BinaryTree<int> BT;
  BT.Create(array, 12);

  return 0;
}
