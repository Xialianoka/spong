#include "byte_stream.hh"

#include <algorithm>
#include <iterator>
#include <stdexcept>

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : _capacity(capacity)
    , _read_count(0)
    , _write_count(0)
    , _input_ended_flag(false)
    , _error(false) {}

size_t ByteStream::write(const string &data) {
    size_t len = data.size();
    if (len > remaining_capacity()) {
        len = remaining_capacity();
    }
    _write_count += len;
    _buffer.append(Buffer(move(string().assign(data.begin(), data.begin() + len))));
    return len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t length = len;
    if (length > buffer_size()) {
        length = buffer_size();
    }
    string s = _buffer.concatenate();
    return string().assign(s.begin(), s.begin() + length);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t length = len;
    if (length > buffer_size()) {
        length = buffer_size();
    }
    _read_count += length;
    _buffer.remove_prefix(length);
    return;
}

void ByteStream::end_input() { _input_ended_flag = true; }

bool ByteStream::input_ended() const { return _input_ended_flag; }

size_t ByteStream::buffer_size() const { return (_write_count - _read_count); }

bool ByteStream::buffer_empty() const { return _write_count == _read_count; }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _write_count; }

size_t ByteStream::bytes_read() const { return _read_count; }

size_t ByteStream::remaining_capacity() const { return _capacity - buffer_size(); }
