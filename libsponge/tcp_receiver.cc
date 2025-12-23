#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs&&... /* unused */) {}

using namespace std;

bool TCPReceiver::segment_received(const TCPSegment& seg) {
  bool is_syn_fin = false;
  size_t abs_seq = 0;
  size_t length = 0;
  if (seg.header().syn) {
    if (_sync_flag) {  // 已经接收过了sync
      return false;
    }
    _sync_flag = true;
    is_syn_fin = true;
    _isn = seg.header().seqno.raw_value();
    abs_seq = 1;
    _base = 1;
    length = seg.length_in_sequence_space() - 1;
    if (length == 0) {  // 没有负载信息
      return true;
    }
  } else if (!_sync_flag) {
    return false;
  } else {  // 普通数据段
    abs_seq = unwrap(seg.header().seqno, WrappingInt32(_isn), abs_seq);
    length = seg.length_in_sequence_space();
  }

  if (seg.header().fin) {
    if (_fin_flag) {
      return false;
    }
    _fin_flag = true;
    is_syn_fin = true;
  } else if (length == 0 && abs_seq == _base) {
    return true;
  } else if (abs_seq + length <= _base ||
             abs_seq >=
                 _base +
                     window_size()) {  // 注意条件，比如seq = 3，len =
                                       // 3，_base为6的情况，seq = 6是取不到的
    if (!is_syn_fin) {
      return false;
    }
  }

  _reassembler.push_substring(seg.payload().copy(), abs_seq - 1,
                              seg.header().fin);
  _base = _reassembler.first_unassembled() + 1;
  if (seg.header().fin) {
    ++_base;
  }
  return true;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
  if (_base > 0) {
    return WrappingInt32(wrap(_base, WrappingInt32(_isn)));
  } else {
    return nullopt;
  }
}

size_t TCPReceiver::window_size() const {
  return _capacity - _reassembler.stream_out().buffer_size();
}
