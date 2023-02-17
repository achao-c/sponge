#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity):capicity (capacity) {  }

size_t ByteStream::write(const string &data) {
    if (data.size() <= left_num()) {
        totol_write += data.size();
        for (const char ch: data) buffer.push_back(ch);
        return data.size();
    }
    else {
        totol_write += left_num();
        const size_t add_num = left_num();
        size_t idx = 0;
        for (size_t i = add_num; i; --i) {
            buffer.push_back(data[idx++]);
        }
        return add_num;
    }
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string re;
    size_t num = min(len, buffer_size());
    for (auto iter = buffer.begin(); iter < buffer.begin()+num; ++iter) {
        re += *iter;
    }
    return re;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    size_t num = min(len, buffer_size());
    for (size_t i = 0; i < num; ++i) {
        buffer.pop_front();
    }
    totol_pop += num;
 }

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string re = peek_output(len);
    pop_output(len);
    return re;
}

void ByteStream::end_input() { is_end = true; }

bool ByteStream::input_ended() const { return is_end; }

size_t ByteStream::buffer_size() const { return buffer.size(); }

bool ByteStream::buffer_empty() const { return buffer.empty(); }

bool ByteStream::eof() const { return buffer_empty() && is_end; }

size_t ByteStream::bytes_written() const { return totol_write; }

size_t ByteStream::bytes_read() const { return totol_pop; }

size_t ByteStream::remaining_capacity() const { return left_num(); }
