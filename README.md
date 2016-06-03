## libtins-TFTP
#### A TFTP PDU for libtins
---
##### What is TFTP?
Trivial File Transfer Protocol (TFTP) is an Internet software utility for transferring files that is simpler to use than the File Transfer Protocol (FTP) but less capable. It is used where user authentication and directory visibility are not required.

It is used primarily for Preboot Execution Environment (PXE) as the mode of transport for images and configuration to a bare metal server booting off of the local area network.

---
###Using libtins-TFTP:

Adding the library is simple so long as you're using git...
 1. Navigate to your project's repository in your terminal
 2. Decide what location inside the project you want the source to reside
 3. Use the command ```git submodule add https://github.com/nelsonw2014/libtins-TFTP``` to add the repository as a submodule

TODO: Add way to statically link

---
###Typcial Usage:
Example (mostly) taken from [PXESim](https://github.com/nelsonw2014/PXESim)
```c++

// create layer2 tftp read request packet
const Tins::EthernetII create_tftp_read() {
    Tins::EthernetII eth("00:00:de:ad:be:ef", "00:00:B1:6B:00:B5");
    auto *ip = new Tins::IP("8.8.8.8", "127.0.0.1");
    auto *udp = new Tins::UDP(69, 1337);
    auto *tftp = new Tins::TFTP();
    tftp->opcode(TFTP::READ_REQUEST);
    tftp->mode("octet");
    tftp->filename("pxelinux.0");
    tftp->add_option({"blksize", "4096"});
    tftp->add_option({"tsize", "0"});
    udp->inner_pdu(tftp);
    ip->inner_pdu(udp);
    eth.inner_pdu(ip);
    return eth;
}
```

---
#####All six packet types are implemented:

|OpCode|Type|What does it do?|
|:--|:---|:---|
|0x01|READ_REQUEST|Requests a file to read off of the TFTP server|
|0x02|WRITE_REQUEST|Requests to write a file to the TFTP server|
|0x03|DATA|Data block|
|0x04|ACK|Acknowledges a block|
|0x05|ERROR|Specifies error code and string|
|0x06|OPT_ACK|Acknowledges option extension|


---
More information can be found on the specification at the following locations:
* [The TFTP Protocol (Revision 2)](http://www.ietf.org/rfc/rfc783.txt)
* [TFTP Option Extension](https://tools.ietf.org/html/rfc2347)
