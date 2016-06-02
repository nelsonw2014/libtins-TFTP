//
// Created by warnelso on 6/1/16.
//

#include "tftp.h"

using namespace Tins;


uint32_t TFTP::header_size() const {
    if (_opcode == NONE || _opcode > OPT_ACKNOWLEDGEMENT) return 0;
    uint32_t size = 2;
    if (_opcode == READ_REQUEST || _opcode == WRITE_REQUEST) {
        size += _filename.length() + 1;
        size += (uint32_t) _mode + 1;
    } else if (_opcode == DATA) {
        size += 2 + _data.size();
    } else if (_opcode == ACKNOWLEDGEMENT) {
        size += 2;
    } else if (_opcode == ERROR) {
        size += 2 + _error.length() + 1;
    }
    if ((_opcode == READ_REQUEST || _opcode == WRITE_REQUEST || _opcode == OPT_ACKNOWLEDGEMENT) && !_options.empty()) {
        for (auto &option: _options) {
            size += option.first.length() + 1 + option.second.length() + 1;
        }
    }
    return size;
}

void TFTP::write_serialization(uint8_t *data, uint32_t sz, const PDU *parent) {
    // TODO: Build out serialization
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