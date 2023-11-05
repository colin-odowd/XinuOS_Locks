/* lock.h */

#define NSPINLOCKS 20

struct	sl_lock_t {			
    uint32 flag;
    pid32 owner;
};

typedef struct sl_lock_t sl_lock_t;
extern sl_lock_t mutex;
