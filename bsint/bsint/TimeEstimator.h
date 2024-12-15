class TimeEstimator
{
	int		*times;
	int		next;
	int		valid;
	int		nsamples;
	
public:
	TimeEstimator( int sampleSize = 4 )
	{
		next = 0;
		valid = 0;
		nsamples = sampleSize;
		if ( nsamples < 3 )
			nsamples = 3;
		times = new int[nsamples];
	}
	
	~TimeEstimator( void )
	{
		delete[] times;
	}
	
	void	takeSample( void )
	{
		extern unsigned long TickCount( void );
		
		times[ next ] = TickCount();
		if ( ++next == nsamples )
			next = 0;
		if ( valid < nsamples )
			valid++;
	}
	
	float	estimateCompletion( int todo )
	{
		int	last;
		
		if ( valid != nsamples )
			return 0;
			
		last = next - 1;
		if ( last < 0 )
			last = nsamples - 1;
			
		return float(times[last] - times[next]) * todo / float(nsamples-1) / 60.0;
	}
};
