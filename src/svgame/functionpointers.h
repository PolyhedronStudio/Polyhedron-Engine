typedef enum {
    P_bad,

    P_prethink,
    P_think,
    P_blocked,
    P_touch,
    P_use,
    P_pain,
    P_die,

    P_moveinfo_endfunc,
} ptr_type_t;

typedef struct {
    ptr_type_t type;
    void *ptr;
} save_ptr_t;

//extern const save_ptr_t save_ptrs[];
//extern const int num_save_ptrs;
