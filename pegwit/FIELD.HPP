class GFe // element of finite field
{
private:
  friend class GF;
  friend class GFp;
  friend class GF2m;
  friend class GF2m_poly_basis;
  friend class GF2m_opt_norm_basis;
  GF * f;
  vlong v;
  GFe( GF * F, const vlong & V );
public:
  GFe();
  GFe( GF * F );
  GFe& operator = ( const GFe & x );
  void set_random( rng & r );
  void set_value( const vlong & x );
  vlong get_value() const;
  int operator == ( const GFe & x ) const;
  int operator != ( const GFe & x ) const;
  GFe operator + ( const GFe & x ) const;
  GFe operator - ( const GFe & x ) const;
  GFe operator - () const;
  GFe operator * ( const GFe & x ) const;
  GFe operator / ( const GFe & x ) const;
  GFe operator ^ ( const vlong & n ) const;
  friend GFe sq( const GFe & x );
};

GFe operator * ( const vlong &n, const GFe & x );

class GF // abstract finite field
{
public:
  rng * rn;
  GFe zero,one;
  GF();
  virtual vlong add( const vlong & x, const vlong & y )=0;
  virtual vlong sub( const vlong & x, const vlong & y )=0;
  virtual vlong div( const vlong & x, const vlong & y )=0;
  virtual vlong mul( const vlong & x, const vlong & y )=0;
  virtual vlong normalise( const vlong & x )=0;
  virtual vlong square( const vlong &x );
  virtual vlong exp( const vlong & x, const vlong & n );
};

class GFp : public GF // Finite field with prime number of elements
{
public:
  const vlong p;
  int ybit( const GFe & x );
  GFe sqrt( const GFe &x );
  GFp( vlong P );
private:
  virtual vlong add( const vlong & x, const vlong & y );
  virtual vlong sub( const vlong & x, const vlong & y );
  virtual vlong mul( const vlong & x, const vlong & y );
  virtual vlong div( const vlong & x, const vlong & y );
  virtual vlong exp( const vlong & x, const vlong & n );
  virtual vlong normalise( const vlong & x );
};

class GF2m : public GF // Finite field with 2^m elements
{
public:
  unsigned m;
  vlong g; // generating polynomial
  GF2m( unsigned M );
  virtual ~GF2m();
  virtual GFe sqrt( const GFe &x )=0;
  virtual int trace( const GFe &x )=0;
  virtual GFe quad_solve( const GFe &x )=0;
  virtual int ybit( const GFe & x ) = 0;
  virtual vlong * matrix( GF2m & to ) = 0;
private:
  virtual vlong add( const vlong & x, const vlong & y );
  virtual vlong sub( const vlong & x, const vlong & y );
  virtual vlong normalise( const vlong & x );
};

class GF2m_opt_norm_basis : public GF2m
// N.B. optimal basis does not exist for all values of M
{
public:
  static int exists( unsigned M );
  GF2m_opt_norm_basis( unsigned M );
  ~GF2m_opt_norm_basis();
  virtual GFe sqrt( const GFe &x );
  virtual int trace( const GFe &x );
  virtual GFe quad_solve( const GFe &x );
  virtual int ybit( const GFe & x );
  virtual vlong * matrix( GF2m & to );
private:
  static int exists( unsigned M, int typeI );
  virtual vlong mul( const vlong & x, const vlong & y );
  virtual vlong div( const vlong & x, const vlong & y );
  virtual vlong square( const vlong &x );

  unsigned * lambda[2];
  // low-level optimisation stuff
  struct rota // used to make rotated copies of a vlong
  {
    public:
    unsigned ** a; // for i<m, a[i] points to rotated copy of loaded value
    unsigned m;
    unsigned mdu;
    rota(unsigned M);
    void load( const vlong & x );
  };
  rota r0,r1;
  void mul( rota & rx, unsigned xr, rota & ry, vlong & result );
  unsigned mu;    // number of unsigneds required to store m bits
  unsigned mask;  // used bits of msw
  unsigned * d;   // temporary register during by mul
};

class GF2m_poly_basis : public GF2m
{
public:
  GF2m_poly_basis( unsigned M );
  virtual GFe sqrt( const GFe &x );
  virtual int trace( const GFe &x );
  virtual GFe quad_solve( const GFe &x );
  virtual int ybit( const GFe & x );
  virtual vlong * matrix( GF2m & to );
private:
  GFe t;   // element with non-zero trace
  virtual vlong mul( const vlong & x, const vlong & y );
  virtual vlong div( const vlong & x, const vlong & y );
  virtual vlong square( const vlong &x );
};

class GF2lk : public GF2m
// values are represented as polynomials
// whose coefficients are in GF2l
{
public:
  GF2lk( unsigned l, unsigned k );
  virtual GFe sqrt( const GFe &x );
  virtual int trace( const GFe &x );
  virtual GFe quad_solve( const GFe &x );
  virtual int ybit( const GFe & x );
private:
  vlong g; // generating polynomial
  GFe t;   // element with non-zero trace
  virtual vlong mul( const vlong & x, const vlong & y );
  virtual vlong div( const vlong & x, const vlong & y );
  virtual vlong square( const vlong &x );
};

GFe find_root( GF2m & F, const vlong & f );
GFe convert( const GFe & X, GF2m & to, vlong * M ); // M is matrix
vlong * inverse( vlong * M, unsigned m ); // Calculates inverse matrix
