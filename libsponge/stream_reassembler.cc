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
_next_assemble_idx(0), _unassemble_strs(), _unassemble_bytes_num(0), _eof_idx(-1) {

}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof)
 {
    // 思路就是57. 插入区间的思路，将已有区间分为三块，然后将待插入区间不断与中间重叠区间合并。
    if (eof) _eof_idx = index + data.size();
    
    size_t start = index;
    if (!data.size()) {
        if (_eof_idx <= _next_assemble_idx) _output.end_input();
        return;
    }
    size_t end = index+data.size()-1;  // 容易引起溢出！！！！！！无符号数-1，所以要在前面判断
    // 1.首先验证其是否超过两端
    if (start < _next_assemble_idx) start = _next_assemble_idx;
    size_t first_unacceptable_idx = _next_assemble_idx - _output.buffer_size() + _capacity;
    if (end >= first_unacceptable_idx) end = first_unacceptable_idx-1;
    if (start > end) {
        if (_eof_idx <= _next_assemble_idx) _output.end_input();
        return;
    }
    string ist_data = data.substr(start-index, end-start+1);
    // 2.首先找出已存在未排序的字符串（其起始位置大于待插入字符串的结束）
    // 该字符串一定不与新字符串重合
    auto iter = _unassemble_strs.upper_bound(end);
    // 2.1/2 下面两种情况说明没有重合，不需要改变
    if (_unassemble_strs.empty()) {}
    else if (iter == _unassemble_strs.begin()) {}
    // 2.3 一直向前与未排序字符串比较，直到找到与前面字符串不重合或者遍历完
    else {
        --iter;  // 前一个判断与新字符串是否相交
        // set由于迭代器只是双向迭代器，因此需要在begin()停下，但是该处也有可能重合，所以需要加上一种特殊情况
        // 因为迭代器没有>=操作符
        while ((iter != _unassemble_strs.begin() && iter->second.first >= start) 
        || (iter == _unassemble_strs.begin() && iter->second.first >= start)) {
            // 新字符串与set中已有的进行组合
            if (start > iter->first) {
                size_t lgt = start-iter->first;
                ist_data = iter->second.second.substr(0, lgt) + ist_data;
                start = iter->first;
            }
            if (end < iter->second.first) {
                size_t lgt = iter->second.first-end;
                ist_data += iter->second.second.substr(iter->second.second.size()-lgt);
                end = iter->second.first;
            }
            // 删除set中与新字符串重叠的字符串，需要区分是否是set第一个字符串的两种情况
            auto _iter = iter;
            if (iter != _unassemble_strs.begin()) {
                --iter;
                _unassemble_bytes_num -= _iter->second.second.size();
                _unassemble_strs.erase(_iter);
            }
            else {
                _unassemble_bytes_num -= _iter->second.second.size();
                _unassemble_strs.erase(_iter);
                break;
            }
        }

    } 
    
    // 3.添加到字典中
    _unassemble_strs[start] = {end, ist_data};
    _unassemble_bytes_num += end-start+1;
    // 4.遍历字典，添加到流中
    for (auto _iter = _unassemble_strs.begin(); _iter != _unassemble_strs.end(); ) {
        //assert(iter->first >= _next_assemble_idx);
        if (_iter->first == _next_assemble_idx) {
            size_t write_num = _output.write(_iter->second.second);
            _next_assemble_idx += write_num;
            // 若此时流内满了
            if (write_num < _iter->second.second.size()) {
                _unassemble_bytes_num += _iter->second.second.size()-write_num;
                _unassemble_strs[_next_assemble_idx] = 
                {end, _iter->second.second.substr(write_num)};

                _unassemble_bytes_num -= _iter->second.second.size();
                _unassemble_strs.erase(_iter);
                break;
            }
            else {
                _unassemble_bytes_num -= _iter->second.second.size();
                _iter = _unassemble_strs.erase(_iter);
            }
        }
        else break;
    }
    
    if (_eof_idx <= _next_assemble_idx) _output.end_input();




}

size_t StreamReassembler::unassembled_bytes() const { return _unassemble_bytes_num; }

bool StreamReassembler::empty() const { return _unassemble_bytes_num == 0; }

//size_t StreamReassembler::show_map() { 
 //   for (auto iter = _unassemble_strs.begin(); iter != _unassemble_strs.end(); ++iter) cout << iter->first << ' ' << iter->second.size() << endl;
  //  return 0; }