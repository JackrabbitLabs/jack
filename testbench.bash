#!/bin/bash

echo -e \\n------------------------------------------------------------------------------
echo -e 1: Show help menu for each command \\n

set -x
jack 

jack aer 
jack ld 
jack ld cfg 
jack ld mem 
jack mctp 
jack port 
jack port bind      
jack port config 
jack port control
jack port unbind 
jack set ld 
jack set ld allocations
jack set qos 
jack set qos allocated 
jack set qos control 
jack set qos limit 
jack show 
jack show ld
jack show ld allocations
jack show ld info
jack show port -h 
jack show qos 
jack show qos allocated
jack show qos control
jack show qos limit
jack show qos status 
jack show switch -h 
jack show vcs -h
set +x

echo -e \\n------------------------------------------------------------------------------
echo -e 2: Show Commands \\n

set -x
jack show id 
jack show bos
jack show limit 
jack show switch
jack show port -a 
jack show vcs -a 
jack show ld info -p 1 
jack show ld allocations -p 1 
jack show qos status -p 1 
jack show qos control -p 1 
jack show qos allocated -p 1 
jack show qos limit -p 1 
set +x

echo -e \\n------------------------------------------------------------------------------
echo -e 3: Port CXL.io CFG commands \\n

set -x
jack port config -p 0 -r 0 -e 0 -f 0xF 
jack port config -p 0 -r 0 -e 0 -f 0xF -w --data 0xa1a2a3a4
jack port config -p 0 -r 0 -e 0 -f 0xF 
set +x

echo -e \\n------------------------------------------------------------------------------
echo -e 3: LD CXL.io CFG commands \\n

set -x
jack ld config -p 1 -l 0 -r 0 -e 0 -f 0xF 
jack ld config -p 1 -l 0 -r 0 -e 0 -f 0xF -w --data 0xa1a2a3a4
jack ld config -p 1 -l 0 -r 0 -e 0 -f 0xF 
set +x

echo -e \\n------------------------------------------------------------------------------
echo -e 4: Port Control \\n

set -x
jack show port -p 1 
jack port ctrl -p 1 --assert-perst 
jack show port -p 1 
jack port ctrl -p 1 --deassert-perst 
jack show port -p 1 
set +x

echo -e \\n------------------------------------------------------------------------------
echo -e 5: Port Bind/Unbind \\n

set -x
jack show port -p 1 
jack port bind -p 1 -l 0 -c 0 -b 0 
jack show port -p 1 
jack show vcs -c 0
jack port unbind -c 0 -b 0 
jack show port -p 1 
jack show vcs -c 0
set +x

echo -e \\n------------------------------------------------------------------------------
echo -e 6: Set LD Allocations \\n

set -x
jack show ld allocations -p 1 
jack set ld allocations -p 1 --range1 0,1,2,3,4,5,6,7,8,9,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f --range2 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
jack show ld allocations -p 1 
set +x

echo -e \\n------------------------------------------------------------------------------
echo -e 7: Set QoS Allocated \\n

set -x
jack show qos allocated -p 1 
jack set qos allocated -p 1 -f 0,1,2,3,4,5,6,7,8,9,0xa,0xb,0xc,0xd,0xe,0xf
jack show qos allocated -p 1 
set +x

echo -e \\n------------------------------------------------------------------------------
echo -e 8: Set QoS Limit \\n

set -x
jack show qos limit -p 1 
jack set qos limit -p 1 -f 0xf,0xe,0xd,0xc,0xb,0xa,9,8,7,6,5,4,3,2,1,0
jack show qos limit -p 1 
set +x

echo -e \\n------------------------------------------------------------------------------
echo -e 9: Set QoS Control \\n

set -x
jack show qos control -p 1 
jack set qos control -p 1 -m 52 -s 63 -k 18 -q 21 -i 74 
jack show qos control -p 1 
set +x

echo -e \\n------------------------------------------------------------------------------
echo -e 10: AER \\n

set -x
jack aer -c 0 -b 0 -e 0xc1c2c3c4 -t 000102030405060708090A0B0C0D0E0F000102030405060708090A0B0C0D0E0F
set +x

echo -e \\n------------------------------------------------------------------------------
echo -e 11: MCTP \\n

set -x
jack mctp --get-eid
jack mctp --get-uuid
jack mctp --get-type 
jack mctp --get-ver 0x7
set +x

