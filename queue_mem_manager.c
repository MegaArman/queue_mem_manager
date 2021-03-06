#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

unsigned char dat[2048];
typedef unsigned char Q;

void printInt(unsigned char b1, unsigned char b2)
{
  const unsigned int n = b1 | (b2 << 8);
  printf("%u \n", n); 
}

int twoByteToInt(unsigned char b1, unsigned char b2)
{
  return (b1 | (b2 << 8));
}

//set last allocated byte
void defrag()
{   
  for (int i = 2; i < 386; i+=6) //380 avoid bounds issue
  {
    int headIndex = twoByteToInt(dat[i], dat[i+1]);
    int length = twoByteToInt(dat[i+2], dat[i+3]);
    int capacity = twoByteToInt(dat[i+4], dat[i+5]);

    Q* qToShift = 0;
    if (length < capacity)
    {
      int nextHeadIndex = 2047;
      for (int h = 2; h < 386; h += 6)
      {
        int currentHeadIndex = twoByteToInt(dat[h], dat[h+1]);
        if (h==i || currentHeadIndex > 0)
        {
          continue;
        }
        if (currentHeadIndex > headIndex && currentHeadIndex < nextHeadIndex)
        {
          nextHeadIndex = currentHeadIndex;
          qToShift = &dat[h];
        }
      }

      //close prev queue capacity
      dat[i+4] = dat[i+2];
      dat[i+5] = dat[i+3];

      if (qToShift > 0) 
      {
        int nextLength = twoByteToInt(qToShift[2], qToShift[3]);
        
        //shift
        int qToShiftNewHeadIndex = i + length + 1;

        memcpy(&dat[qToShiftNewHeadIndex], &dat[nextHeadIndex], nextLength); 
        qToShift[0] = (unsigned char) qToShiftNewHeadIndex;
        qToShift[1] = (unsigned char) (qToShiftNewHeadIndex >> 8);

        //reduce top
        dat[0] = (unsigned char) qToShiftNewHeadIndex;
        dat[1] = (unsigned char) (qToShiftNewHeadIndex >> 8);
      }
    }
  }  
}

//return pointer to created queue after split
Q* split_underutilized_queue()
{
  Q* q = 0;
  int index_1;
  int length_1;
  int capacity_1;
  int index_2;
  int length_2 = 0;
  int capacity_2;

  for (int i = 2; i < 386; i+=6)
  {
    index_1 = twoByteToInt(dat[i], dat[i+1]);
    length_1 = twoByteToInt(dat[i+2], dat[i+3]);
    capacity_1 = twoByteToInt(dat[i+4], dat[i+5]);
    if (length_1 < capacity_1 / 2)
    {
      index_2  = floor(index_1 + capacity_1 / 2);
      capacity_2 = ceil(capacity_1 / 2);

      capacity_1 = floor(capacity_1/2);
      dat[i+4] = (unsigned char) capacity_1;
      dat[i+5] = (unsigned char) (capacity_1 >> 8);
      break;
    } 
  }
  
  int i = 2;
  for (i = 2; i < 386; i+=6)
  {
    if (dat[i] == 0)
    {
     dat[i] = (unsigned char) index_2; // head of queue
     dat[i+1] = (unsigned char) (index_2 >> 8);
     dat[i+2] = 0; //length of queue
     dat[i+3] = 0;
     dat[i+4] = (unsigned char) capacity_2; //capacity of queue 
     dat[i+5] = (unsigned char) (capacity_2 >> 8);
     break;
    }
  }
  q = &dat[i];
  if (q != 0)
  {
    return &dat[i];
  }
  else
  {
    defrag();
    int lastAllocated = twoByteToInt(dat[0], dat[1]);
    if (lastAllocated < 2047)
    {
      split_underutilized_queue();
    }
  }
}


Q* create_queue()
{
 unsigned int highestAllocatedIndex = dat[0] | (dat[1] << 8);
 
 unsigned int newHeadIndex;
 if (highestAllocatedIndex < 386)
 {
  newHeadIndex = 386;
 }
 else if (highestAllocatedIndex < 2047) 
 {
  newHeadIndex = highestAllocatedIndex + 1;
 }
 else
 {
  return split_underutilized_queue();
 }

 unsigned int blockEndIndex = newHeadIndex;
 if  (blockEndIndex + 99 < 2047)
 {
  blockEndIndex += 99;
 }
 else
 {
  blockEndIndex = 2047;
 }

 dat[0] = (unsigned char) blockEndIndex;
 dat[1] = (unsigned char) (blockEndIndex >> 8);
 int capacity = blockEndIndex - newHeadIndex + 1;

  int i = 2;
  for (i = 2; i < 386; i+=6)
  {
    if (dat[i] == 0)
    {
     dat[i] = (unsigned char) newHeadIndex; // head of queue
     dat[i+1] = (unsigned char) (newHeadIndex >> 8);
     dat[i+2] = 0; //length of queue
     dat[i+3] = 0;
     dat[i+4] = (unsigned char) capacity; //capacity of queue 
     dat[i+5] = (unsigned char) (capacity >> 8);
     break;
    }
  }

  return &dat[i];
}

void destroy_queue(Q* q)
{
  q[0] = 0;
  q[1] = 0;
  q[2] = 0;
  q[3] = 0;
  q[4] = 0;
  q[5] = 0;
}

void enqueue_byte(Q *q, unsigned char b)
{
  int headIndex = twoByteToInt(q[0], q[1]);
  int length = twoByteToInt(q[2], q[3]);
  int capacity = twoByteToInt(q[4], q[5]);
  int lastAllocated = twoByteToInt(dat[0], dat[1]);

  if (length < capacity) //queue has space
  {
    dat[headIndex + length] = b;
    length++;
    q[2] = (unsigned char) length;
    q[3] = (unsigned char) (length >> 8);
  }
  else if (lastAllocated < 2047 && headIndex + capacity == lastAllocated) //last queue in list
  {
    int newLength = length + 1;
    int newCapacity = capacity + 1;
    dat[headIndex + newLength] = b;
    q[2] = (unsigned char) newLength;
    q[3] = (unsigned char) (newLength >> 8);
    q[4] = (unsigned char) newCapacity;
    q[5] = (unsigned char) (newCapacity >> 8);
    
    int newLastAllocated = headIndex + newLength;
    dat[0] = (unsigned char) newLastAllocated;
    dat[1] = (unsigned char) (newLastAllocated >> 8);
  }
  else if (lastAllocated + length + 1 < 2047) ///not last queue in list, must copy
  {
    memcpy(&dat[lastAllocated+1], &dat[headIndex], length);
    int newHeadIndex = lastAllocated + 1;
    int newLength = length + 1;
    int lastBlockAllocated = newHeadIndex + newLength;
    dat[lastBlockAllocated] = b;

    dat[0] = (unsigned char) lastBlockAllocated; 
    dat[1] = (unsigned char) (lastBlockAllocated >> 8);

    q[0] = (unsigned char) newHeadIndex;
    q[1] = (unsigned char) (newHeadIndex >> 8);
    q[2] = (unsigned char) newLength;
    q[3] = (unsigned char) (newLength >> 8);
    q[4] = q[2];
    q[5] = q[3];
  }
  else //queue at end with no space
  {
    printf("defragged \n");
    defrag(q); //should give where this queue will now point to 
    int newLastAllocated = twoByteToInt(dat[0], dat[1]);
    if (newLastAllocated + length + 1 < 2047)
    {
     enqueue_byte(q, b); 
    }
    else
    {
      printf("Out of memory \n");
    }
  }
} 

unsigned char dequeue_byte(Q* q)
{
  int headIndex = twoByteToInt(q[0], q[1]);
  int byte = dat[headIndex];
  int oldLength = twoByteToInt(q[2], q[3]);
  int newLength = oldLength - 1;
  q[2] = (unsigned char) newLength;
  q[3] = (unsigned char) (newLength >> 8);
  
  for (int i = 0; i < newLength; i++) 
  {
    dat[headIndex + i] = dat[headIndex+i+1];
  }
  return byte;
}

int main()
{
  for (int i = 0; i < 386; i++)
  {
    dat[i] = 0;
  }

  // NOTE: UNCOMMENT ONLY ONE TEST
  //test 1: proof queues function properly 
  Q* q0 = create_queue();
  enqueue_byte(q0,  0);
  enqueue_byte(q0,  1);
  Q* q1 = create_queue();
  enqueue_byte(q1, 3);
  enqueue_byte(q0, 2);
  enqueue_byte(q1, 4);
  printf("%d\n", dequeue_byte(q0));
  printf("%d\n", dequeue_byte(q0));
  enqueue_byte(q0, 5);
  enqueue_byte(q1, 6);
  printf("%d\n", dequeue_byte(q0));
  printf("%d\n", dequeue_byte(q0));
  destroy_queue(q0);
  printf("%d\n", dequeue_byte(q1));
  printf("%d\n", dequeue_byte(q1));
  printf("%d\n", dequeue_byte(q1));
 
  
  //test 1: enqueuing more bytes than avail in a queue
  //Q* q0 = create_queue();
  //Q* q1 = create_queue();
  //for (int i = 0; i < 110; i++)
  //{
  //  enqueue_byte(q0, 9);
  //}
  //printf("%d \n", dequeue_byte(q0));
  
  //test 3: create 64 queues
  //for (int i = 0; i < 64; i++)
  //{
  //  create_queue();
  //}
 
  //LEAVE UNCOMMENTED FOR ALL TESTS, PRINTS RESERVED BLOCK
  for (int i = 0; i < 386; i+=2)
  {
    int n = twoByteToInt(dat[i], dat[i+1]);
    printf("%d ", n);
  }
}

