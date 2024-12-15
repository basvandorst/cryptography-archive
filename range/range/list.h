enum type_kinds {POINT, INTERVAL};

struct region {
     struct dat {
          type_kinds type;
          rvar left, right;};

     region& operator= (region&);
     friend void get(region*&);
     friend void remove(region*);

     region* next;
     dat data[1];

     static region *free;
     static int dim;
     static int size;
     static void initiate(int);
};

#ifdef main_program  

region* region::free = 0;
int region::dim = 0;
int region::size = 0;

#endif

struct list {
     list ();
     ~list ();

     void clear();
     void append(region*);
     void insert(region*);
     void initiate(region*);
     void remove_first();

     region *base;
};

