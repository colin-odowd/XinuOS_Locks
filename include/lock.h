/* lock.h */

#define NO_OWNER (-1)
#define NO_LOCK 0
#define NSPINLOCKS 20
#define NLOCKS 20   
#define NALOCKS 20

struct	sl_lock_t {			
    uint32 flag;
    pid32 owner;
};

struct lock_t {
    uint32 flag;
    uint32 guard;
    pid32 owner;
    qid16 q;
};

struct al_lock_t {
    uint32 flag;
    uint32 guard;
    uint32 id;
    pid32 owner;
    qid16 q;
};

typedef struct sl_lock_t sl_lock_t;
typedef struct lock_t lock_t;
typedef struct al_lock_t al_lock_t;



