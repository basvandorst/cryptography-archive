#include "vlong.hpp"
#include "field.hpp"

class curve; class curve_GF2m; class curve_GFp;

class point // On elliptic curve
{
public:
  point( curve * C, GF * F );
  point( const point & P );
  friend int operator == ( const point &P, const point &Q );
  friend int operator != ( const point &P, const point &Q );
  friend point operator + ( const point &P, const point &Q );
  friend point operator - ( const point &P, const point &Q );
  friend point operator - ( const point &P );
  friend point operator * ( const vlong &k, const point &P ); // sum of k>0 copies of P
  friend curve; friend curve_GF2m; friend curve_GFp;
  GFe x,y;    // co-ordinates
  void save( vlstr & s );
  void load( vlstr & s );
private:
  curve * c; // curve to which point belongs
  int zero;  // zero flag (maybe could use x==y==0 instead?)
};

class curve // Elliptic curve
{
public:
  curve( GF * F );
  void load( vlstr & s );
  void save( vlstr & s ); // save a,b,P and p
  virtual ~curve();
  virtual point add( const point &P, const point &Q )=0;
  virtual point inverse( const point &P )=0;
  friend point operator * ( const vlong &k, const point &P );
  virtual point rp( rng & r )=0;
  virtual int ybit( const point &P )=0; // point compression
  virtual void expand( point &P, int ybit )=0; // point expansion
  GF * f;
  GFe a,b;
  point O;
  point P;
  vlong p; // order of P ( should be large prime )
};

class curve_GFp : public curve // Elliptic curve over GF(p)
{
  GFp f1;
  virtual point add( const point &P, const point &Q );
  virtual point inverse( const point &P );
public:
  virtual point rp( rng & r );
  virtual int ybit( const point &P ); // point compression
  virtual void expand( point &P, int ybit ); // point expansion
  curve_GFp( const vlong & p );
  curve_GFp( vlstr & s );
  void save( vlstr & s );
};

class random_curve_GFp : public curve_GFp // random curve, security uncertain
{
public:
  random_curve_GFp( const vlong & p, rng & r );
};

class curve_GF2m : public curve // Elliptic curve over GF(2^m)
{
  virtual point add( const point &P, const point &Q );
  virtual point inverse( const point &P );
protected:
  curve_GF2m( unsigned M );
public:
  GF2m * f1;
  curve_GF2m( vlstr & s );
  void save( vlstr & s );
  virtual point rp( rng & r );
  virtual int ybit( const point &P ); // point compression
  virtual void expand( point &P, int ybit ); // point expansion
  vlong order();
};

class random_curve_GF2m : public curve_GF2m // random curve, security uncertain
{
public:
  random_curve_GF2m( unsigned m, rng & r );
};

class weil_curve : public curve_GF2m
{
public:
  weil_curve( rng & r, unsigned l=5, unsigned k=13, const vlong & min_p=0 );
  // Some possible values for l,k are: 5,7  5,13  9,11  5,31
  // 10,13  6,23  6,29  11,19. See findlk.* for more.
  // min_p need not be specified, (a default = 2^(l-1)*k is calculated)
  // if min_p is > default, the constructor may not terminate
};

