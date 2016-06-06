//
//    Copyright (c) 2016, Warren Nelson
//    All rights reserved.
//
//    Redistribution and use in source and binary forms, with or without
//    modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice,
//      this list of conditions and the following disclaimer in the documentation
//      and/or other materials provided with the distribution.
//
//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "tftp.h"

#include <tins/memory_helpers.h>

using namespace Tins;

const uint16_t change_endian(const uint16_t orig) {
    return ((orig & 0x00ff) << 8) | ((orig & 0xff00) >> 8);
}

std::string read_until_deliminator(Memory::InputMemoryStream &stream, const uint8_t delim = 0) {
    std::string return_string;
    uint8_t buff;
    while (stream.can_read(1)) {
        stream.read(buff);
        return_string.push_back(buff);
        if (*(return_string.end() - 1) == '\0') {
            return_string.pop_back();
            break;
        }
    }
    return return_string;
}

TFTP::TFTP(const uint8_t *data, uint32_t sz) {
    Memory::InputMemoryStream stream(data, sz);
    uint16_t buff;
    stream.read(buff);
    _opcode = (OpCodes) change_endian(buff);
    if (_opcode == NONE || _opcode > OPT_ACKNOWLEDGEMENT) throw malformed_packet();
    if (_opcode == READ_REQUEST || _opcode == WRITE_REQUEST) {
        _filename = read_until_deliminator(stream);
        _mode = read_until_deliminator(stream);
    } else if (_opcode == DATA) {
        stream.read(buff);
        _block = change_endian(buff);
        _data.resize(sz - 4);
        stream.read(_data, sz - 4);
    } else if (_opcode == ACKNOWLEDGEMENT) {
        stream.read(buff);
        _block = change_endian(buff);
    } else if (_opcode == ERROR) {
        stream.read(buff);
        _error_code = (ErrorCodes) change_endian(buff);
        _error = read_until_deliminator(stream);
    }
    if (_opcode == READ_REQUEST || _opcode == WRITE_REQUEST || _opcode == OPT_ACKNOWLEDGEMENT) {
        while (stream.can_read(4)) {
            _options.push_back({
                                       read_until_deliminator(stream),
                                       read_until_deliminator(stream)
                               });
        }
    }

}

uint32_t TFTP::header_size() const {
    if (_opcode == NONE || _opcode > OPT_ACKNOWLEDGEMENT) return 0;
    uint32_t size = 2;
    if (_opcode == READ_REQUEST || _opcode == WRITE_REQUEST) {
        size += _filename.size() + 1;
        size += _mode.size() + 1;
    } else if (_opcode == DATA) {
        size += 2 + _data.size();
    } else if (_opcode == ACKNOWLEDGEMENT) {
        size += 2;
    } else if (_opcode == ERROR) {
        size += 2 + _error.size() + 1;
    }
    if ((_opcode == READ_REQUEST || _opcode == WRITE_REQUEST || _opcode == OPT_ACKNOWLEDGEMENT) && !_options.empty()) {
        for (auto &option: _options) {
            size += option.first.size() + 1 + option.second.size() + 1;
        }
    }
    return size;
}

void TFTP::write_serialization(uint8_t *data, uint32_t sz, const PDU *parent) {
    if (header_size()) {
        Memory::OutputMemoryStream stream(data, sz);
        stream.write(change_endian(_opcode));
        if (_opcode == READ_REQUEST || _opcode == WRITE_REQUEST) {
            stream.write((const uint8_t *) _filename.data(), _filename.size());
            stream.write((uint8_t) 0);
            stream.write((const uint8_t *) _mode.data(), _mode.size());
            stream.write((uint8_t) 0);
        } else if (_opcode == DATA) {
            stream.write(change_endian(_block));
            stream.write(_data.data(), _data.size());
        } else if (_opcode == ACKNOWLEDGEMENT) {
            stream.write(change_endian(_block));
        } else if (_opcode == ERROR) {
            stream.write(change_endian(_error_code));
            stream.write((const uint8_t *) _error.data(), _error.size());
            stream.write((uint8_t) 0);
        }
        if ((_opcode == READ_REQUEST || _opcode == WRITE_REQUEST || _opcode == OPT_ACKNOWLEDGEMENT) &&
            !_options.empty()) {
            for (auto &option: _options) {
                stream.write((const uint8_t *) option.first.data(), option.first.size());
                stream.write((uint8_t) 0);
                stream.write((const uint8_t *) option.second.data(), option.second.size());
                stream.write((uint8_t) 0);
            }
        }
    }
}

void TFTP::data(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end) {
    _data.assign(begin, end);
}

TFTP::Option TFTP::search_option(const std::string &option_field) const {
    for (auto &option: _options) {
        if (option.first == option_field) return option;
    }
    throw option_not_found();
}

bool TFTP::add_option(const Option &new_option) {
    for (auto &option: _options) {
        if (option.first == new_option.first) {
            option.second = new_option.second;
            return true;
        }
    }
    _options.push_back(new_option);
    return true;
}

bool TFTP::delete_option(const std::string &option_field) {
    for (auto iter = _options.begin(); iter != _options.end(); iter++) {
        if (iter->first == option_field) {
            _options.erase(iter);
            return true;
        }
    }
    throw option_not_found();
}
