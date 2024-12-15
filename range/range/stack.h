class stack { // number of rvars held is set by constructor
              // only 1 stack at a time can be used in a program
public:
     stack(int); // int of constuctor gives no. of rvar's in a level;
                 // lowest level is available like a vector after
                 // this constructor called.
     void push();// push levels; construct, if necessary,
                 // new lowest level. After a push, new lowest level
                 // has rvars which may have any initial value.
     void pop(); // move levels back one.

     rvar& operator() (int);
                 // If stack has name NAME, then NAME(i) 
                 // gives access to ith component of lowest level.
     
     static int not_empty; // FALSE when pop not possible;
                           // TRUE after first push;
     
     ~stack();
private:
     struct level {
          char size; level* forward; level* back; rvar comp[1];};
     static int no_chars;
     static int no_comps;
     static level* sta;
};

inline rvar& stack::operator() (int i) {
     return sta->comp[i];}

#ifdef main_program  

int stack:: not_empty = FALSE;
int stack:: no_chars = 0;
int stack:: no_comps = 0;
stack::level* stack::sta = 0;

#endif


