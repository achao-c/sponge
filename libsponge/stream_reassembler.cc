#include "stream_reassembler.hh"
#include <iostream>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity), 
_next_assemble_idx(0), _unassemble_strs(), _unassemble_bytes_num(0), _eof_idx(-1) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    
    if (eof) _eof_idx = index + data.size();

    /* --1.字符串开头位置处理 */
    auto pos_iter = _unassemble_strs.upper_bound(index); // 获取map中第一个比index大的下标
    if (pos_iter != _unassemble_strs.begin()) --pos_iter; // 此时有三种情况：获取map小于等于index的下标；map为空或者map均为大于index的下标。

    size_t new_idx = index;
    // 1.如果前面有待排列字串
    if (pos_iter != _unassemble_strs.end() && pos_iter->first <= index) {
        const size_t up_idx = pos_iter->first;
        // 如果当前字符串与上一个发生了重叠
        if (up_idx + pos_iter->second.size() > index) {
            // 如果上一个字符串将当前串包含了
            if (up_idx + pos_iter->second.size() >= index + data.size()) return;
            new_idx = up_idx + pos_iter->second.size();
        }
    }
    // 2.前面没有字串，需要比较与待处理字符下标的关系
    else {
        if (index + data.size() <= _next_assemble_idx) {
            if (_eof_idx <= _next_assemble_idx) _output.end_input();
            return;
        }
        if (index < _next_assemble_idx) new_idx = _next_assemble_idx;
    }

    // 3.若起始位置超过容量了，就舍去
    size_t first_unacceptable_idx = _next_assemble_idx - _output.buffer_size() + _capacity;
    if (new_idx >= first_unacceptable_idx) return;

    // 新字符串的长度，略微与答案不同
    // 如果当前字符串被已存在字符串包含，在上面逻辑已经处理。
    size_t data_size = data.size()-(new_idx-index); 
    

    /* --2.处理字符串结尾的位置 */
    pos_iter = _unassemble_strs.lower_bound(new_idx);  // 此时可能新起始位置与down_idx重合
    
    // 1. 若后面有未排序的字符串，要考虑重合的情况
    while (pos_iter != _unassemble_strs.end() && new_idx <= pos_iter->first) {
        // 1.无接触
        if (new_idx + data_size <= pos_iter->first) break;
        // 2.有接触
        else {
            // 部分接触
            if (new_idx + data_size <= pos_iter->first + pos_iter->second.size()) {
                data_size = pos_iter->first - new_idx;
                break;
            }
            // 完全包含下一个字符串
            else {
                _unassemble_bytes_num -= pos_iter->second.size(); // 未排序字符个数减去删除的序列长度
                pos_iter = _unassemble_strs.erase(pos_iter); // erase函数内可以是key也可以是迭代器，只有迭代器才会返回下一个迭代器
            }
        } 
    } 
    // 2.跟起始位置统一逻辑，若超出容量限制，就保存至最大容量的长度
    if (new_idx + data_size > first_unacceptable_idx) data_size = 
    data_size - (new_idx + data_size - first_unacceptable_idx);

    // 这一步是关键，若新的截断字串长度为0，则必须防止它将已有的key覆盖
    if (!data_size) {
        if (_eof_idx <= _next_assemble_idx) _output.end_input();
        return;
    }
    /* --3.获取到新的字串及下标后，进行插入*/
    string new_data = data.substr(new_idx - index, data_size);
    // 将新字符串插入map中
    _unassemble_strs[new_idx] = new_data;
    _unassemble_bytes_num += new_data.size();

    // 遍历map字符串，使得可以添加到bytestream的map中的字符串进入
    for (auto iter = _unassemble_strs.begin(); iter != _unassemble_strs.end(); ) {
        //assert(iter->first >= _next_assemble_idx);
        if (iter->first == _next_assemble_idx) {
            size_t write_num = _output.write(iter->second);
            _next_assemble_idx += write_num;
            // 若此时流内满了
            if (write_num < iter->second.size()) {
                _unassemble_bytes_num += iter->second.size()-write_num;
                _unassemble_strs[_next_assemble_idx] = iter->second;

                _unassemble_bytes_num -= iter->second.size();
                _unassemble_strs.erase(iter);
                break;
            }
            else {
                _unassemble_bytes_num -= iter->second.size();
                iter = _unassemble_strs.erase(iter);
            }
        }
        else break;
    }

    if (_eof_idx <= _next_assemble_idx) {std::cout << _eof_idx; _output.end_input();}


}

size_t StreamReassembler::unassembled_bytes() const { return _unassemble_bytes_num; }

bool StreamReassembler::empty() const { return _unassemble_bytes_num == 0; }

size_t StreamReassembler::show_map() { 
    for (auto iter = _unassemble_strs.begin(); iter != _unassemble_strs.end(); ++iter) cout << iter->first << ' ' << iter->second.size() << endl;
    return 0; }