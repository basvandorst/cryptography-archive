// elliptic curves over GF(2^(L*K))

#define MAXL 16
#define MAXK 193

class small_field
{
  #if MAXL > 16 // assumes that unsigned short is at least 16 bits
    typedef unsigned lunit;
  #else
    #if MAXL > 8
      typedef unsigned short lunit;
    #else
      typedef unsigned char lunit;
    #endif
  #endif
  lunit * const alog; // index range is [0..(BASE-2)]
  lunit * const log;  // index range is [1..(BASE-1)], but log[0] is set to (BASE-1)
  friend class field;
public:
  unsigned * curve_order();
  const unsigned L,BASE,BASE_M1;
  small_field( unsigned L, unsigned root );
  ~small_field();
};

class field // elements are polynomials with coefficients in small_field
: public small_field
{
  virtual unsigned rand( unsigned base );
  const unsigned M,K,T;
  typedef unsigned poly[2*MAXK];
  unsigned prng;
  poly nzt; // element with non-zero trace
  poly tm;  // trace mask ( trace(x) is number of bits in x & tm )
  void addmul( poly a, unsigned alpha, unsigned j, const poly b );
  static void add( const poly a, const poly b, poly c );
  static void copy ( const poly a, poly b );
  static int equal( const poly a, const poly b );
  void div( poly a, unsigned b );
  void set_random( poly a );
  int set_K( unsigned K, unsigned T );
  void reduce( poly a );
  void mul( const poly a, const poly b, poly c );
  void square( const poly a, poly b );
  void inverse( const poly a, poly b );
  int trace( const poly a );
  int slow_trace( const poly a );
  void quad_solve( const poly a, poly b );
  void sqrt( const poly a, poly b );
  field( class full_curve_parameter & a );
  friend class curve;
  friend class field_element;
  friend class full_curve_parameter;
  void unpack( const vlong & x, poly a );
  vlong pack( const poly a );
};

class field_element
{
  field * f;
  field::poly v;
  friend class field;
  friend class curve;
  friend class point;
public:
  int operator == ( const field_element & x ) const;
  int operator == ( unsigned x ) const;
  field_element operator + ( const field_element & x ) const;
  field_element operator * ( const field_element & x ) const;
  field_element operator / ( const field_element & x ) const;
  field_element& operator = ( const field_element & x );
  field_element( const field_element & x );
  field_element( field * F );
  field_element();
};

class point
{
  curve * c;
  field_element x,y;
  point( curve * C );
  friend class curve;
  friend class ec_crypt;
  friend point operator * ( const vlong & k, const point & P );
public:
  point();
  point( const point & P );
  point & operator = ( const point & P );
  point operator + ( const point & P ) const;
  point operator - ( const point & P ) const;
};

struct curve_parameter
{
  unsigned L,K,T,root,b,nso,ntf;
};

class full_curve_parameter : public curve_parameter
{
public:
  vlong tm,p0,P0;
  full_curve_parameter( const curve_parameter & bp );
};

class curve : public field
{
  field_element b;
  void add( const point & P, const point & Q, point & R );
  void sub( const point & P, const point & Q, point & R );
  void mul( const point & P, const vlong & x, point & Q );
  int calc_y( point & R, unsigned ybit=0 );
  static int MOV( unsigned B, const vlong & q, const vlong & r );
  static vlong small_lucas( vlong P, vlong Z, unsigned ik );
  static unsigned ybit( const field_element & x );
  static field_element sq( const field_element & x );
  friend class curve_factory;
  friend class point;
  friend point operator * ( const vlong & k, const point & P );
public:
  curve( full_curve_parameter & a );
  point P; // point with prime order
  vlong p; // prime order of P
  point random_point();
  static vlong pack( const point & P );
  point unpack( const vlong & X );
  static vlong to_vlong( const point & P );
};

class curve_factory
// Used for calculating curve_parameter but in practice
// use pre-calculated table ncdata.hpp or equivalent
{
  unsigned L,root,so_min,so_max,*so_set;
  vlong comp;
public:
  curve_factory( unsigned L ); // can take a long time
  int find( unsigned K, curve_parameter & result );
  ~curve_factory();
};

struct pair { vlong s,r; };

class ec_crypt : private curve
{
public:
  const unsigned bits; // number of bits in prime order
  virtual unsigned rand( unsigned base )=0;

  vlong make_private_key();
  vlong make_public_key( const vlong & private_key );

  vlong make_secret( const vlong & public_key, pair & message );
  vlong decode_secret( const vlong & private_key, const pair & message );

  pair nr_sign( const vlong & data, const vlong & private_key ); // data < pow2(bits)
  vlong nr_verify( const pair & sig, const vlong & public_key );

  pair dsa_sign( const vlong & data, const vlong & private_key );
  int dsa_verify( const vlong & data, const pair & sig, const vlong & public_key );

  ec_crypt( full_curve_parameter & a );

};
