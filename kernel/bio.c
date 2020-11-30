// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"
#define NBUCKETS 13
struct {
  struct buf buf[NBUF];
  struct spinlock lock[NBUCKETS];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  struct buf hashbucket[NBUCKETS];
} bcache;

void
binit(void)
{
  struct buf *b;
  // Create linked list of buffers
  for(int i = 0 ; i < NBUCKETS; i++){
    //初始化自旋锁
    initlock(&bcache.lock[i], "bcache");
    //初始化缓存块链表
    bcache.hashbucket[i].prev = &bcache.hashbucket[i];
    bcache.hashbucket[i].next = &bcache.hashbucket[i];
  }

  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
      b->next = bcache.hashbucket[0].next;
      b->prev = &bcache.hashbucket[0];
      initsleeplock(&b->lock, "buffer");
      bcache.hashbucket[0].next->prev = b;
      bcache.hashbucket[0].next = b;
    }  
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  //对应的哈希值
  int hashId = blockno % NBUCKETS;

  struct buf *b;

  acquire(&bcache.lock[hashId]);

  // Is the block already cached?
  for(b = bcache.hashbucket[hashId].next; b != &bcache.hashbucket[hashId]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[hashId]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached; recycle an unused buffer.
  for(b = bcache.hashbucket[hashId].prev; b != &bcache.hashbucket[hashId]; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock[hashId]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  //当前哈希桶没有可用缓存块
  for(int i = 0 ; i < NBUCKETS; i++){
    if(i == hashId){
      continue;
    }
    //锁住当前哈希桶
    acquire(&bcache.lock[i]);
    for(b = bcache.hashbucket[i].prev; b != &bcache.hashbucket[i]; b = b->prev){
      if(b->refcnt == 0) {
        
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        //将b从被抢占的哈希桶中去除
        b->next->prev = b->prev;
        b->prev->next = b->next;
        release(&bcache.lock[i]);
        //将b加入当前哈希桶
        b->next = bcache.hashbucket[hashId].next;
        b->prev = &bcache.hashbucket[hashId];
        bcache.hashbucket[hashId].next->prev = b;
        bcache.hashbucket[hashId].next = b;

        release(&bcache.lock[hashId]);
        acquiresleep(&b->lock);
        return b;
      }
    }
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;
  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b->dev, b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b->dev, b, 1);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void
brelse(struct buf *b)
{
  int hashId = b->blockno % NBUCKETS;
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache.lock[hashId]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.hashbucket[hashId].next;
    b->prev = &bcache.hashbucket[hashId];
    bcache.hashbucket[hashId].next->prev = b;
    bcache.hashbucket[hashId].next = b;
  }
  
  release(&bcache.lock[hashId]);
}

//引用缓存块
void
bpin(struct buf *b) {
  int hashId = b->blockno % NBUCKETS;
  acquire(&bcache.lock[hashId]);
  b->refcnt++;
  release(&bcache.lock[hashId]);
}

//去除引用缓存块
void
bunpin(struct buf *b) {
  int hashId = b->blockno % NBUCKETS;
  acquire(&bcache.lock[hashId]);
  b->refcnt--;
  release(&bcache.lock[hashId]);
}


