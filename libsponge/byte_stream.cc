#include "byte_stream.hh"

#include <algorithm>
#include <iterator>
#include <stdexcept>

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : _buffer(vector<char>(capacity))
    , _capacity(capacity)
    , _read_count(0)
    , _write_count(0)
    , _input_ended_flag(false)
    , _error(false) {}

size_t ByteStream::write(const string &data) {
    size_t n = data.size();
    if (n == 0) {
        return 0;
    }
    if (n > remaining_capacity()) {
        n = remaining_capacity();
    }
    for (size_t i = 0; i < n; ++i) {
        _buffer[_write_count % _capacity] = data[i];
        ++_write_count;
    }
    return n;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t length = len;
    if (length > buffer_size()) {
        length = buffer_size();
    }
    string res{};
    size_t i = _read_count;
    while (length--) {
        res += _buffer[i % _capacity];
        ++i;
    }
    return res;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t length = len;
    if (length > buffer_size()) {
        length = buffer_size();
    }
    _read_count += length;
}

void ByteStream::end_input() { _input_ended_flag = true; }

bool ByteStream::input_ended() const { return _input_ended_flag; }

size_t ByteStream::buffer_size() const { return (_write_count - _read_count); }

bool ByteStream::buffer_empty() const { return _write_count == _read_count; }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _write_count; }

size_t ByteStream::bytes_read() const { return _read_count; }

size_t ByteStream::remaining_capacity() const { return _capacity - buffer_size(); }
