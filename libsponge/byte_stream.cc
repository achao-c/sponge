#include "byte_stream.hh"
// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity):  que(), lgt(capacity) {}

size_t ByteStream::write(const string &data) {
    /*
    首先观察剩余空间与data的关系，将多出与未多出的情况分类讨论
    */
    size_t len = data.size() > lgt-que.size()? lgt-que.size(): data.size();
    for (size_t i = 0; i < len; ++i) {
        que.push(data[i]);
    }
    write_num += len;
    return len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) {
    string s = read(len);
    read_num -= s.size();
    string tmp;
    while (!que.empty()) {
        tmp += que.front();
        que.pop();
    }
    tmp = s + tmp;
    for (char ch: tmp) que.push(ch);
    return s;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    size_t num = len <= que.size()? len: que.size();
    for (size_t i = 0; i < num; ++i) que.pop();
    read_num += num;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string reS;
    size_t num = len <= que.size()? len: que.size();
    while (num--) {
        reS += que.front();
        que.pop();
    }
    read_num += reS.size();
    return reS;
}

void ByteStream::end_input() {_input_ended_flag = true; }

bool ByteStream::input_ended() const { return _input_ended_flag; }

size_t ByteStream::buffer_size() const { return que.size(); }

bool ByteStream::buffer_empty() const { return que.empty(); }

bool ByteStream::eof() const { return que.empty() && _input_ended_flag; }

size_t ByteStream::bytes_written() const { return write_num; }

size_t ByteStream::bytes_read() const { return read_num; }

size_t ByteStream::remaining_capacity() const { return lgt-que.size(); }
