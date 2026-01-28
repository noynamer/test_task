#include <stddef.h>
#include <stdint.h>

#define SMALL_SIZE 15
#define LARGE_SIZE 180
#define MAX_SMALL 1000
#define MAX_LARGE 500
#define ALIGNMENT (sizeof(void*))

// Align sizes
#define SMALL_ALIGNED ((SMALL_SIZE + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
#define LARGE_ALIGNED ((LARGE_SIZE + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

static uint8_t small_pool[MAX_SMALL][SMALL_ALIGNED];
static uint8_t large_pool[MAX_LARGE][LARGE_ALIGNED];

// Bit mask to track free blocks
static uint32_t small_bitmap[(MAX_SMALL + 31) / 32];
static uint32_t large_bitmap[(MAX_LARGE + 31) / 32];

// Helper function to find the first zero bit
static int find_free_bit(uint32_t word) 
{
   uint32_t inverted = ~word;
   if (inverted == 0) return -1; // all occupied

   // Find the first zero bit
   int bit = 0;
   while ((inverted & 1) == 0) 
   {
      inverted >>= 1;
      bit++;
   }
   return bit;
}

void* malloc(size_t size) 
{
   if (size == 0) return NULL;

   if (size <= SMALL_SIZE) 
   {
      for (int i = 0; i < (MAX_SMALL + 31) / 32; i++) 
      {
         uint32_t word = small_bitmap[i];
         if (word != 0xFFFFFFFF) 
         {  // there are free ones
            int bit = find_free_bit(word);
            if (bit >= 0) 
            {
               int index = i * 32 + bit;
               if (index < MAX_SMALL) 
               {
                  small_bitmap[i] |= (1U << bit);
                  return small_pool[index];
               }
            }
         }
      }
   }
   else if (size <= LARGE_SIZE) 
   {
      for (int i = 0; i < (MAX_LARGE + 31) / 32; i++) 
      {
         uint32_t word = large_bitmap[i];
         if (word != 0xFFFFFFFF) 
         {
            int bit = find_free_bit(word);
            if (bit >= 0) 
            {
               int index = i * 32 + bit;
               if (index < MAX_LARGE) 
               {
                  large_bitmap[i] |= (1U << bit);
                  return large_pool[index];
               }
            }
         }
      }
   }

   return NULL;
}

void free(void* ptr) 
{
   if (!ptr) return;

   uint8_t* p = (uint8_t*)ptr;

   // Check small pool
   if (p >= (uint8_t*)small_pool && p < (uint8_t*)(small_pool + MAX_SMALL)) 
   {
      size_t index = (p - (uint8_t*)small_pool) / SMALL_ALIGNED;
      int i = index / 32;
      int bit = index % 32;
      small_bitmap[i] &= ~(1U << bit);
      return;
   }

   // Check large pool
   if (p >= (uint8_t*)large_pool && p < (uint8_t*)(large_pool + MAX_LARGE)) 
   {
      size_t index = (p - (uint8_t*)large_pool) / LARGE_ALIGNED;
      int i = index / 32;
      int bit = index % 32;
      large_bitmap[i] &= ~(1U << bit);
      return;
   }
}

void init_allocator(void) 
{
   for (int i = 0; i < (MAX_SMALL + 31) / 32; i++) 
   {
      small_bitmap[i] = 0;
   }
   for (int i = 0; i < (MAX_LARGE + 31) / 32; i++) 
   {
      large_bitmap[i] = 0;
   }
}