#ifndef QUEUE_H
#define QUEUE_H

#define QMAX 1024

typedef struct queue
{
  uint64_t data[QMAX];
  int rear,front;
} queue;
 

static inline void q_initialize(queue *P) {
  P->rear=-1;
  P->front=-1;
}
 
static inline int q_empty(queue *P) {
  if(P->rear==-1)
    return(1);
  return(0);
}
 
 /*
static inline int full(queue *P) {
  if((P->rear+1)%QMAX==P->front)
    return(1);
  return(0);
}
*/

static inline void q_enqueueF(queue *P,uint64_t x) {
  if(q_empty(P)) {
    P->rear=0;
    P->front=0;
    P->data[0]=x;
  } else {
    P->front=(P->front-1+QMAX)%QMAX;
    P->data[P->front]=x;
  }
}
 
static inline uint64_t q_dequeueF(queue *P)  {
  uint64_t x;
  x=P->data[P->front];
  if(P->rear==P->front) {//delete the last element
    q_initialize(P);
  } else {
    P->front=(P->front+1)%QMAX;
  }
  return(x);
}

#endif