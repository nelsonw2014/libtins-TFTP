//
// Created by warnelso on 6/1/16.
//
#include "tftp.h"

#include <tins/memory_helpers.h>

using namespace Tins;


std::string read_until_deliminator(Memory::InputMemoryStream &stream, const uint8_t delim = 0) {
    std::string return_string;
    uint8_t buff;
    while (stream.can_read(1)) {
        stream.read(buff);
        return_string.push_back(buff);
        if (*(return_string.end()) == 0) {
            return_string.pop_back();
            break;
        }
    }
    return return_string;
}

TFTP::TFTP(const uint8_t *data, uint32_t sz) {
    Memory::InputMemoryStream stream(data, sz);
    stream.read(_opcode);
    if (_opcode == NONE || _opcode > OPT_ACKNOWLEDGEMENT) throw malformed_packet();
    if (_opcode == READ_REQUEST || _opcode == WRITE_REQUEST) {
        _filename = read_until_deliminator(stream);
        _mode = read_until_deliminator(stream);
    } else if (_opcode == DATA) {
        stream.read(_block);
        _data.resize(sz - 4);
        stream.read(_data, sz - 4);
    } else if (_opcode == ACKNOWLEDGEMENT) {
        stream.read(_block);
    } else if (_opcode == ERROR) {
        stream.read(_error_code);
        _error = read_until_deliminator(stream);
    }
    if ((_opcode == READ_REQUEST || _opcode == WRITE_REQUEST || _opcode == OPT_ACKNOWLEDGEMENT) && !_options.empty()) {
        while (stream.can_read(4)) {
            _options.push_back(std::make_pair(read_until_deliminator(stream), read_until_deliminator(stream)));
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
        stream.write((uint16_t) _opcode);
        if (_opcode == READ_REQUEST || _opcode == WRITE_REQUEST) {
            stream.write((const uint8_t *) _filename.data(), _filename.size());
            stream.write((uint8_t) 0);
            stream.write((const uint8_t *) _mode.data(), _mode.size());
            stream.write((uint8_t) 0);
        } else if (_opcode == DATA) {
            stream.write(_block);
            stream.write(_data.data(), _data.size());
        } else if (_opcode == ACKNOWLEDGEMENT) {
            stream.write(_block);
        } else if (_opcode == ERROR) {
            stream.write((uint16_t) _error_code);
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
