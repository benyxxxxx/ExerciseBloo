#include <stdio.h>
#include <string.h>
#include <iostream>

template <class T, class Size = unsigned long long> class MemoryManager {
 public:
  //constructor
  MemoryManager(T* memory, Size size);
  
  //defragments the memory
  MemoryManager& defragment();

  //logs data out
  void print();  
  
 private:
  
  enum STATE {
    INSIDE_FREE_AREA,
    OUT_OF_FREE_AREA
  };

 private:
  
  //checks whether a memory element is 0
  bool isNull(T* memory) {
    char* tmp = (char*)memory;

    for (unsigned i = 0; i < sizeof(T); i++) {
      if (tmp[i] != 0)
        return false;
    }
    return true;
  }
  
  //adds a new Free Block node.
  void addNode(T* memory, Size size) {
    list* newNode = new list(memory, size, 0, 0);

    if (m_begin == 0) {
      m_begin = newNode;
    } else {
      m_end->setNext(newNode);
      newNode->setPrev(m_end);
    }
    m_end = newNode;
  }
  

  //destroys the list structure
  void destroyStructure() {
    list* currentBlock = m_begin;
    
    while (currentBlock) {
      list* next = currentBlock->next();
      delete currentBlock;
      currentBlock = next;
    }

    m_begin = m_end = 0;
  }

  //shifts a data block of <size> length, starting at <start> to the right <offset> elements.
  void shiftDataRight(T* start, Size size, Size offset) {
    memmove(start+offset, start, size*sizeof(T));
  }

 private:
  
  //list of free nodes
  class list {
   public:
    list(T* memory, Size size, 
	 list* prev,  list* next) : 
      m_memory(memory), m_size(size), m_prev(prev), m_next(next) {
    }
    
    T* getMemory() {
      return m_memory;
    }
    
    Size getSize() {
      return m_size;
    }
    
    list* prev() {
      return m_prev;
    }
    
    list* next() {
      return m_next;
    }

    void setNext(list* next) {
      m_next = next;
    }

    void setPrev(list* prev) {
      m_prev = prev;
    }
    
   private:
    T* m_memory; //start of free block
    Size m_size; //free block size

    list* m_prev; //prev list element
    list* m_next; //next list element
  };
  
  
private:
  T* m_memory;
  Size m_size; //size of memery
  
  list* m_begin; //first Free Block element 
  list* m_end; //last Free Block element 
};

template <class T, class Size>
inline MemoryManager<T, Size>::MemoryManager(T* memory, Size size) : 
  m_memory(memory), m_size(size), m_begin(0), m_end(0) {
  
  STATE state = OUT_OF_FREE_AREA;
  T* currentFreeBlockStart = 0;
  
  //create free nodes
  //traverse the memory, locates areas with 0's and created FreeNodes for every such area
    for (Size i = 0; i < m_size; i++) {

      if (state == INSIDE_FREE_AREA) {
        
        if (!isNull(m_memory + i)) {
          //create a new Node
          addNode(currentFreeBlockStart, m_memory + i - currentFreeBlockStart);

          currentFreeBlockStart = 0;
          state = OUT_OF_FREE_AREA;
        }
        
      } else {
        
        if (isNull(m_memory + i)) { 
          currentFreeBlockStart = m_memory + i;
          state = INSIDE_FREE_AREA;
        }
      }
    }
    
    //check whether the memory finishes with a free area
    if (currentFreeBlockStart != 0) {

      //create a new Node
      addNode(currentFreeBlockStart, m_memory + m_size - currentFreeBlockStart);
    }
}

template <class T, class Size>
MemoryManager<T, Size>& MemoryManager<T, Size>::defragment() {
  int offsetForMove = 0;
  list* currentBlock = m_end;

  
  //traverse FreeNodes from the end and move all the Data areas to the right, to be make data area contignuos and placed at the end of the memory
  
  //1. Before defragment
  // |free|data1|free|data2
  
  //2. After defragment
  // |free      |data1|data2
  
  while(currentBlock) {
    offsetForMove += currentBlock->getSize();
    
    list* prevBlock = currentBlock->prev();
    T* dataToMove =  prevBlock ? 
      (prevBlock->getMemory() + prevBlock->getSize()) : m_memory;
    
    
    shiftDataRight(dataToMove, currentBlock->getMemory() - dataToMove, 
                   offsetForMove);
    
    currentBlock = currentBlock->prev();
  } 
  
  //destroy the current list
  destroyStructure();
  
  if (offsetForMove) {
    //setup Free Area to be all 0.
    memset(m_memory, 0, offsetForMove*sizeof(T));
    
    //add a Node which represens Free Area
    addNode(m_memory, offsetForMove);
  }
  
  return *this;
}

template <class T, class Size>
void MemoryManager<T, Size>::print() {

  //print free blocks
  std::cout << "Free block lengths:";
  list* current = m_begin;
  while (current) {    
    std::cout << " " << current->getSize();
    current = current->next();
    if (current)
      std::cout << ",";
  }
  std::cout << " | Occupied block contents: ";
  
  bool repeatData = true;
  for (int i = 0; i < m_size; i++) {

    if (isNull(m_memory + i)) {
      if (!repeatData) { 
        std::cout << ",";
        repeatData = true;
      } 
    } else {

      repeatData = false;
      char* tmp = (char*)(m_memory + i);
      for (int j = 0; j < sizeof(T); j++) {
        if (tmp[j])
          std::cout << tmp[j];
      }
    }
  }
  std::cout << std::endl;
}


int main()
{
  int array[] = {
    0, 0, 0, // free block of 3
    'C', 'O', 'N', 'T', 'I', // occupied block of 5
    0, 0, 0, 0, 0, // free block of 5
    'G', 'U', 'O', 'U', // occupied block of 4
    0, 0, // free block of 2
    'S', '!' }; // occupied block of 2
  MemoryManager<int> mm(array, 21);
  mm.print();
  mm.defragment().print();
  
  
  char array_char[] = {}; // occupied block of 2
  MemoryManager<char> mm_char(array_char, 0);
  mm_char.print();
  mm_char.defragment().print();
  
 return 0;
}
