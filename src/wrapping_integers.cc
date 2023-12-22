#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  (void)n;
  (void)zero_point;
  return zero_point + static_cast<uint32_t>( n );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // uint64_t base_num=1UL<<32;
  uint64_t abs_seqno = static_cast<uint64_t>( this->raw_value_ - zero_point.raw_value_ );
  // near checkpoint
  uint64_t mod_times = checkpoint >> 32;
  uint64_t the_mod = checkpoint << 32 >> 32;

  uint64_t boundary;
  if ( the_mod < ( 1UL << 31 ) )
    boundary = mod_times;
  else
    boundary = mod_times + 1;

  uint64_t absl = abs_seqno + ( ( boundary == 0 ? 0 : boundary - 1 ) << 32 );
  uint64_t absr = abs_seqno + ( boundary << 32 );

  if ( checkpoint < ( absl + absr ) / 2 ) {
    abs_seqno = absl;
  } else {
    abs_seqno = absr;
  }

  (void)zero_point;
  (void)checkpoint;
  return abs_seqno;
}
