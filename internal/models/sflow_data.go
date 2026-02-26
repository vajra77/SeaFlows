package models

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"log"
	"net"
)

type SflowData struct {
	Timestamp    int64
	SrcMAC       string
	DstMAC       string
	IPv          int
	SrcIP        string
	DstIP        string
	SamplingRate uint64
	Size         uint64
}

type AggregatedFlow struct {
	SrcMAC string
	DstMAC string
	Bytes4 uint64
	Bytes6 uint64
}

type Datagram struct {
	Version      uint32
	IpVersion    uint32
	AgentAddress string
	SubAgentID   uint32
	Sequence     uint32
	Uptime       uint32
	NumSamples   uint32
	Samples      []*Sample
}

type Sample struct {
	DataFormat   uint32
	Length       uint32
	SeqNumber    uint32
	SourceID     uint32
	SamplingRate uint32
	SamplePool   uint32
	Drops        uint32
	InputIf      uint32
	OutputIf     uint32
	NumRecords   uint32
	Records      []*FlowRecord
}

type FlowRecord struct {
	DataFormat uint32
	Length     uint32
	Packet     RawPacket
}

type RawPacket struct {
	Protocol       uint32
	Length         uint32
	Stripped       uint32
	Size           uint32
	DatalinkHeader DatalinkHeader
	IPHeader       IPHeader
}

type EthernetHeader struct {
	SrcMACAddress string
	DstMACAddress string
	EthType       uint16
}

type VlanHeader struct {
	ID  uint16
	Len uint16
}

type DatalinkHeader struct {
	EthernetHeader EthernetHeader
	VlanHeader     VlanHeader
}

type IPHeader struct {
	SrcIPAddress string
	DstIPAddress string
}

func CleanMAC(hw net.HardwareAddr) string {
	var b bytes.Buffer
	for _, octet := range hw {
		_, _ = fmt.Fprintf(&b, "%02x", octet)
	}
	return b.String()
}

// UnmarshalBinary decodes a buffer of data from the net according to sFlow (v5) packet format and
// fills in Datagram container struct. This is the core func responsible for the parsing of network
// received data
// Returns error
func (d *Datagram) UnmarshalBinary(data []byte) error {

	// check if enough data
	if len(data) < 24 {
		return errors.New("sflow packet too short")
	}

	// sFlow version
	d.Version = binary.BigEndian.Uint32(data[0:4])
	if d.Version != 5 {
		return fmt.Errorf("unsupported sflow version: %d", d.Version)
	}

	// Parsing Agent IP (v4 o v6)
	ipVersion := binary.BigEndian.Uint32(data[4:8])
	ptr := 8
	if ipVersion == 1 {
		d.AgentAddress = net.IP(data[ptr : ptr+4]).String()
		ptr += 4
	} else {
		d.AgentAddress = net.IP(data[ptr : ptr+16]).String()
		ptr += 16
	}

	// additional sFlow header fields
	d.SubAgentID = binary.BigEndian.Uint32(data[ptr : ptr+4])
	d.Sequence = binary.BigEndian.Uint32(data[ptr+4 : ptr+8])
	d.Uptime = binary.BigEndian.Uint32(data[ptr+8 : ptr+12])
	numSamples := binary.BigEndian.Uint32(data[ptr+12 : ptr+16])
	ptr += 16

	// prepare buffer for Flow Samples
	d.Samples = make([]*Sample, 0, d.NumSamples)

	for i := 0; i < int(numSamples); i++ {
		if ptr+8 > len(data) {
			break
		}
		sample := &Sample{}
		sample.DataFormat = binary.BigEndian.Uint32(data[ptr : ptr+4])
		ptr += 4
		sample.Length = binary.BigEndian.Uint32(data[ptr : ptr+4])
		ptr += 4

		if sample.DataFormat == 1 {
			if ptr+int(sample.Length) > len(data) {
				return errors.New("sample length overflow")
			}

			sample.SeqNumber = binary.BigEndian.Uint32(data[ptr : ptr+4])
			ptr += 4
			sample.SourceID = binary.BigEndian.Uint32(data[ptr : ptr+4])
			ptr += 4
			sample.SamplingRate = binary.BigEndian.Uint32(data[ptr : ptr+4])
			ptr += 4
			sample.SamplePool = binary.BigEndian.Uint32(data[ptr : ptr+4])
			ptr += 4
			sample.Drops = binary.BigEndian.Uint32(data[ptr : ptr+4])
			ptr += 4
			sample.InputIf = binary.BigEndian.Uint32(data[ptr : ptr+4])
			ptr += 4
			sample.OutputIf = binary.BigEndian.Uint32(data[ptr : ptr+4])
			ptr += 4
			sample.NumRecords = binary.BigEndian.Uint32(data[ptr : ptr+4])
			ptr += 4

			// init buffer for Flow Records
			sample.Records = make([]*FlowRecord, 0, sample.NumRecords)

			for k := 0; k < int(sample.NumRecords); k++ {
				record := &FlowRecord{}
				record.DataFormat = binary.BigEndian.Uint32(data[ptr : ptr+4])
				ptr += 4
				record.Length = binary.BigEndian.Uint32(data[ptr : ptr+4])
				ptr += 4

				nextRecordPtr := ptr + (int(record.Length)+3) & ^3

				if record.DataFormat == 1 {
					record.Packet.Protocol = binary.BigEndian.Uint32(data[ptr : ptr+4])
					ptr += 4
					record.Packet.Length = binary.BigEndian.Uint32(data[ptr : ptr+4])
					ptr += 4
					record.Packet.Stripped = binary.BigEndian.Uint32(data[ptr : ptr+4])
					ptr += 4
					record.Packet.Size = binary.BigEndian.Uint32(data[ptr : ptr+4])
					ptr += 4

					if record.Packet.Protocol == 1 {
						record.Packet.DatalinkHeader.EthernetHeader.DstMACAddress = CleanMAC(data[ptr : ptr+6])
						ptr += 6
						record.Packet.DatalinkHeader.EthernetHeader.SrcMACAddress = CleanMAC(data[ptr : ptr+6])
						ptr += 6

						ethType := binary.BigEndian.Uint16(data[ptr : ptr+2])
						ptr += 2

						if ethType == 0x8100 {
							record.Packet.DatalinkHeader.VlanHeader.ID = binary.BigEndian.Uint16(data[ptr : ptr+2])
							ptr += 2
							ethType = binary.BigEndian.Uint16(data[ptr : ptr+2])
							ptr += 2
						}

						record.Packet.DatalinkHeader.EthernetHeader.EthType = ethType

						// IPv4
						if ethType == 0x0800 {
							if ptr+20 > len(data) {
								log.Println("[WARN] IPv4 sample length overflow, skipping record")
								goto EndOfRecord
							}
							ptr += 12
							record.Packet.IPHeader.SrcIPAddress = net.IP(data[ptr : ptr+4]).String()
							record.Packet.IPHeader.DstIPAddress = net.IP(data[ptr+4 : ptr+8]).String()
							ptr += 8
						}

						// IPv6
						if ethType == 0x86dd {
							if ptr+40 > len(data) {
								log.Println("[WARN] IPv4 sample length overflow, skipping record")
								goto EndOfRecord
							}
							ptr += 8
							record.Packet.IPHeader.SrcIPAddress = net.IP(data[ptr : ptr+16]).String()
							record.Packet.IPHeader.DstIPAddress = net.IP(data[ptr+16 : ptr+32]).String()
							ptr += 32
						}
					}
				}
			EndOfRecord:
				sample.Records = append(sample.Records, record)
				// Conform to 4 byte alignment
				ptr = nextRecordPtr
			}
		} else {
			ptr += int(sample.Length)
		}
		d.Samples = append(d.Samples, sample)
	}

	return nil
}
