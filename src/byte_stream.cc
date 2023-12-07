#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity ), queue(), push_len( 0 ), pop_len( 0 ), write_state( false ), error_state( false )
{}

void Writer::push( string data )
{
  for ( auto ch : data ) {
    if ( available_capacity() > 0 ) {
      queue.push( ch );
      push_len++;
    }
  }
  // Your code here.
  (void)data;
}

void Writer::close()
{
  // Your code here.
  write_state = true;
}

void Writer::set_error()
{
  // Your code here.
  error_state = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return write_state;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - queue.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return push_len;
}

string_view Reader::peek() const
{

  return string_view { &queue.front(), 1 };
}

bool Reader::is_finished() const
{
  if ( bytes_buffered() == 0 && write_state ) {
    return true;
  }
  return false;
}

bool Reader::has_error() const
{
  // Your code here.

  return error_state;
}

void Reader::pop( uint64_t len )
{
  for ( uint64_t i = 0; i < len; i++ ) {
    queue.pop();
    pop_len++;
  }

  (void)len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return queue.size();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return pop_len;
}
