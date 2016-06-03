//
// Created by warnelso on 6/1/16.
//

#ifndef LIBTINS_TFTP_TFTP_H
#define LIBTINS_TFTP_TFTP_H

#include <tins/tins.h>
#include <utility>
#include <string>

namespace Tins {
    class TFTP : public PDU {
    public:
        enum OpCodes : uint16_t {
            NONE = 0,
            READ_REQUEST = 1,
            WRITE_REQUEST = 2,
            DATA = 3,
            ACKNOWLEDGEMENT = 4,
            ERROR = 5,
            OPT_ACKNOWLEDGEMENT = 6
        };

        enum ErrorCodes : uint16_t {
            UNDEFINED = 0,
            FILE_NOT_FOUND = 1,
            ACCESS_VIOLATION = 2,
            DISK_FULL = 3,
            ILLEGAL_OPERATION = 4,
            UNKNOWN_XID = 5,
            FILE_EXISTS = 6,
            NO_SUCH_USER = 7,
            OPTION_ERROR = 8
        };

        typedef std::pair<std::string, std::string> Option;

        static const PDU::PDUType pdu_flag = PDU::USER_DEFINED_PDU;

        TFTP() { }

        TFTP(const uint8_t *data, uint32_t sz);

        TFTP *clone() const { return new TFTP(*this); }

        uint32_t header_size() const;

        PDUType pdu_type() const { return pdu_flag; }

        void write_serialization(uint8_t *data, uint32_t sz, const PDU *parent);

        OpCodes opcode() const { return _opcode; }

        void opcode(OpCodes opcode) { _opcode = opcode; }

        ErrorCodes error_code() const { return _error_code; }

        void error_code(ErrorCodes error_code) { _error_code = error_code; }

        uint16_t block() const { return _block; }

        void block(uint16_t block) { _block = block; }

        const std::string filename() const { return _filename; }

        void filename(const std::string &filename) { _filename = filename; }

        std::string mode() const { return _mode; }

        void mode(std::string mode) { _mode = mode; }

        const std::string error() const { return _error; }

        void error(const std::string &error) { _error = error; }

        Option search_option(const std::string &option_field) const;

        bool add_option(const Option &new_option);

        bool delete_option(const std::string &option_field);

    private:
        OpCodes _opcode;
        ErrorCodes _error_code;

        uint16_t _block;
        std::string _filename;
        std::string _mode;
        std::string _error;

        std::vector<Option> _options;
        std::vector<uint8_t> _data;


    };


}


#endif //LIBTINS_TFTP_TFTP_H
