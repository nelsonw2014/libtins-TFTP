//
// Created by warnelso on 6/1/16.
//
#include <tins/memory_helpers.h>
#include "tftp.h"

using namespace Tins;


uint32_t TFTP::header_size() const {
    if (_opcode == NONE || _opcode > OPT_ACKNOWLEDGEMENT) return 0;
    uint32_t size = 2;
    if (_opcode == READ_REQUEST || _opcode == WRITE_REQUEST) {
        size += _filename.size() + 1;
        size += (uint32_t) _mode + 1;
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
            switch (_mode) {
                case NETASCII:
                    stream.write((const uint8_t *) "NETASCII", 8);
                    break;
                case OCTET:
                    stream.write((const uint8_t *) "OCTET", 5);
                    break;
                case MAIL:
                    stream.write((const uint8_t *) "MAIL", 4);
                    break;
            }
            stream.write((uint8_t) 0);
        } else if (_opcode == DATA) {
            stream.write(_block);
            stream.write(_data.data(), _data.size());
        } else if (_opcode == ACKNOWLEDGEMENT) {
            stream.write(_block);
        } else if (_opcode == ERROR) {
            stream.write((uint16_t) _error_code);
            stream.write((const uint8_t *) _error.data(), _error.size());
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