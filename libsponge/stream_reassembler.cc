#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

size_t StreamReassembler::first_unread() { return _output.bytes_read(); }

size_t StreamReassembler::first_unacceptable() { return first_unread() + _capacity; }

size_t StreamReassembler::first_unassembled() { return _output.bytes_written(); }

size_t StreamReassembler::available_capacity() { return _output.remaining_capacity(); }

long StreamReassembler::merge_data(buffer_node &elem1, const buffer_node &elem2) {
    buffer_node prev_data, next_data;
    if (elem1._first_index < elem2._first_index) {
        prev_data = elem1;
        next_data = elem2;
    } else {
        prev_data = elem2;
        next_data = elem1;
    }

    if (prev_data._first_index + prev_data.data.length() < next_data._first_index) {
        // 没有交集，无法合并
        return -1;
    } else if (prev_data._first_index + prev_data.data.length() >= next_data._first_index + next_data.data.length()) {
        elem1 = prev_data;
        return next_data.data.length();
    } else {
        elem1._first_index = prev_data._first_index;
        elem1.data = prev_data.data +
                     next_data.data.substr(prev_data._first_index + prev_data.data.length() - next_data._first_index);
        return prev_data._first_index + prev_data.data.length() - next_data._first_index;
    }
}

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (index + data.length() < first_unassembled() || index > first_unacceptable() || available_capacity() == 0 ||
        _output.eof()) {
        return;
    }
    buffer_node elem{};
    if (index < first_unassembled()) {
        size_t offset = first_unassembled() - index;
        elem._first_index = index + offset;
        elem.data.assign(data.begin() + offset, data.end());
    } else {
        elem._first_index = index;
        elem.data = data;
    }
    _unassembled_bytes += elem.data.length();

    long merged_bytes = 0;
    auto iter = _buffers.lower_bound(elem);
    while (iter != _buffers.end() && (merged_bytes = merge_data(elem, *iter)) >= 0) {
        _unassembled_bytes -= merged_bytes;  // 合并这么多就得减掉这么多（重复了）
        _buffers.erase(iter);
        iter = _buffers.lower_bound(elem);
    }

    if (iter != _buffers.begin()) {
        iter--;
        while ((merged_bytes = merge_data(elem, *iter)) >= 0) {
            _unassembled_bytes -= merged_bytes;
            _buffers.erase(iter);
            iter = _buffers.lower_bound(elem);
            if (iter == _buffers.begin()) {
                break;
            }
            iter--;
        }
    }
    _buffers.insert(elem);

    if (!_buffers.empty() && _buffers.begin()->_first_index == first_unassembled()) {
        const buffer_node prepared_data = *_buffers.begin();
        size_t write_bytes = _output.write(prepared_data.data);
        _unassembled_bytes -= write_bytes;
        _buffers.erase(_buffers.begin());
    }

    if (eof) {
        _eof = true;
    }
    if (_eof && empty()) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }
