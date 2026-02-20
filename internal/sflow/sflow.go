package sflow

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"net"
)

type Datagram struct {
	Version      uint32
	IpVersion    uint32
	AgentAddress string
	SubAgentId   uint32
	SeqNumber    uint32
	SwitchUptime uint32
	NumSamples   uint32
	Samples      []*FlowSample
}

func Decode(buf []byte) (*Datagram, error) {
	ptr := 0
	bufLen := len(buf)

	if bufLen < 24 {
		return nil, errors.New("packet too short")
	}

	data := &Datagram{}
	// sFlow usa Network Byte Order (Big Endian)
	data.Version = binary.BigEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4

	if data.Version != 5 {
		return nil, errors.New("unsupported sFlow version")
	}

	data.IpVersion = binary.BigEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4

	if data.IpVersion == 1 { // IPv4
		data.AgentAddress = net.IP(buf[ptr : ptr+4]).String()
		ptr += 4
	} else { // IPv6
		data.AgentAddress = net.IP(buf[ptr : ptr+16]).String()
		ptr += 16
	}

	data.SubAgentId = binary.BigEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	data.SeqNumber = binary.BigEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	data.SwitchUptime = binary.BigEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	data.NumSamples = binary.BigEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4

	// Inizializziamo la slice con capacità pari al numero di sample previsti
	data.Samples = make([]*FlowSample, 0, data.NumSamples)

	for s := 0; s < int(data.NumSamples); s++ {
		if ptr+8 > bufLen {
			break
		}

		sample := &FlowSample{}
		sample.DataFormat = binary.BigEndian.Uint32(buf[ptr : ptr+4])
		ptr += 4
		sample.Length = binary.BigEndian.Uint32(buf[ptr : ptr+4])
		ptr += 4

		// Gestiamo solo i Flow Sample (Format 1)
		if sample.DataFormat == 1 {
			if ptr+int(sample.Length) > bufLen {
				return nil, errors.New("sample length overflow")
			}

			sample.SeqNumber = binary.BigEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			sample.SourceId = binary.BigEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			sample.SamplingRate = binary.BigEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			sample.SamplePool = binary.BigEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			sample.Drops = binary.BigEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			sample.InputIf = binary.BigEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			sample.OutputIf = binary.BigEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			sample.NumRecords = binary.BigEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4

			sample.Records = make([]*FlowRecord, 0, sample.NumRecords)

			for k := 0; k < int(sample.NumRecords); k++ {
				record := &FlowRecord{}
				record.DataFormat = binary.BigEndian.Uint32(buf[ptr : ptr+4])
				ptr += 4
				record.Length = binary.BigEndian.Uint32(buf[ptr : ptr+4])
				ptr += 4

				// Gestiamo Raw Packet Header (Format 1)
				if record.DataFormat == 1 {
					record.Packet.Protocol = binary.BigEndian.Uint32(buf[ptr : ptr+4])
					ptr += 4
					record.Packet.Length = binary.BigEndian.Uint32(buf[ptr : ptr+4])
					ptr += 4
					record.Packet.Stripped = binary.BigEndian.Uint32(buf[ptr : ptr+4])
					ptr += 4
					record.Packet.Size = binary.BigEndian.Uint32(buf[ptr : ptr+4])
					ptr += 4

					// Parsing Ethernet (Protocol 1)
					if record.Packet.Protocol == 1 {
						srcHW := net.HardwareAddr(buf[ptr : ptr+6])
						ptr += 6
						dstHW := net.HardwareAddr(buf[ptr : ptr+6])
						ptr += 6
						record.Packet.DatalinkHeader.EthernetHeader.SrcMacAddress = CleanMAC(srcHW)
						ptr += 6
						record.Packet.DatalinkHeader.EthernetHeader.DstMacAddress = CleanMAC(dstHW)
						ptr += 6

						ethType := binary.BigEndian.Uint16(buf[ptr : ptr+2])
						ptr += 2

						// Gestione VLAN
						if ethType == 0x8100 {
							record.Packet.DatalinkHeader.VlanHeader.Id = binary.BigEndian.Uint16(buf[ptr : ptr+2])
							ptr += 2
							ethType = binary.BigEndian.Uint16(buf[ptr : ptr+2])
							ptr += 2
						}

						// IPv4
						if ethType == 0x0800 {
							// Saltiamo l'header IPv4 per arrivare agli indirizzi
							// In una versione completa qui andrebbe il parsing completo dell'header IP
							ptr += 12 // Salta Preamble, TTL, Protocol, etc.
							record.Packet.Ipv4Header = &IPv4Header{
								SrcIPAddress: buf[ptr : ptr+4],
								DstIPAddress: buf[ptr+4 : ptr+8],
							}
							ptr += 8
						}
					}
				}
				sample.Records = append(sample.Records, record)
				// Allineamento a 4 byte come da specifica sFlow
				ptr += (int(record.Length) + 3) & ^3
			}
		} else {
			ptr += int(sample.Length)
		}
		data.Samples = append(data.Samples, sample)
	}

	return data, nil
}

func CleanMAC(hw net.HardwareAddr) string {
	var b bytes.Buffer
	for _, octet := range hw {
		_, _ = fmt.Fprintf(&b, "%02x", octet)
	}
	return b.String()
}

//func Decode(buf []byte) (*Datagram, error) {
//	ptr := 0
//	bufLen := len(buf)
//
//	var data = new(Datagram)
//	data.Samples = make([]*FlowSample, 8)
//
//	data.Version = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//	ptr += 4
//	if ptr >= bufLen {
//		return nil, errors.New("not enough data")
//	}
//
//	data.IpVersion = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//	ptr += 4
//	if ptr >= bufLen {
//		return nil, errors.New("not enough data")
//	}
//
//	if data.IpVersion == 1 {
//		var addr net.IP = buf[ptr : ptr+4]
//		data.AgentAddress = addr.String()
//		ptr += 4
//	} else {
//		var addr net.IP = buf[ptr : ptr+16]
//		data.AgentAddress = addr.String()
//		ptr += 16
//	}
//	if ptr >= bufLen {
//		return nil, errors.New("not enough data")
//	}
//
//	data.SubAgentId = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//	ptr += 4
//	if ptr >= bufLen {
//		return nil, errors.New("not enough data")
//	}
//
//	data.SeqNumber = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//	ptr += 4
//	if ptr >= bufLen {
//		return nil, errors.New("not enough data")
//	}
//
//	data.SwitchUptime = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//	ptr += 4
//	if ptr >= bufLen {
//		return nil, errors.New("not enough data")
//	}
//
//	data.NumSamples = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//	ptr += 4
//	if ptr >= bufLen {
//		return nil, errors.New("not enough data")
//	}
//
//	// Samples loop
//	for s := 0; s < int(data.NumSamples); s++ {
//		var sample = new(FlowSample)
//		sample.Records = make([]*FlowRecord, 8)
//
//		sample.DataFormat = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//		ptr += 4
//		if ptr >= bufLen {
//			return nil, errors.New("not enough data")
//		}
//
//		sample.Length = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//		ptr += 4
//		if ptr >= bufLen {
//			return nil, errors.New("not enough data")
//		}
//
//		if sample.DataFormat == 1 {
//
//			sample.SeqNumber = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//			ptr += 4
//			if ptr >= bufLen {
//				return nil, errors.New("not enough data")
//			}
//
//			sample.SourceId = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//			ptr += 4
//			if ptr >= bufLen {
//				return nil, errors.New("not enough data")
//			}
//
//			sample.SamplingRate = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//			ptr += 4
//			if ptr >= bufLen {
//				return nil, errors.New("not enough data")
//			}
//
//			sample.SamplePool = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//			ptr += 4
//			if ptr >= bufLen {
//				return nil, errors.New("not enough data")
//			}
//
//			sample.Drops = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//			ptr += 4
//			if ptr >= bufLen {
//				return nil, errors.New("not enough data")
//			}
//
//			sample.InputIf = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//			ptr += 4
//			if ptr >= bufLen {
//				return nil, errors.New("not enough data")
//			}
//
//			sample.OutputIf = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//			ptr += 4
//			if ptr >= bufLen {
//				return nil, errors.New("not enough data")
//			}
//
//			sample.NumRecords = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//			ptr += 4
//			if ptr >= bufLen {
//				return nil, errors.New("not enough data")
//			}
//
//			// Records loop
//			for k := 0; k < int(sample.NumRecords); k++ {
//				var record = new(FlowRecord)
//
//				record.DataFormat = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//				ptr += 4
//				if ptr >= bufLen {
//					return nil, errors.New("not enough data")
//				}
//
//				record.Length = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//				ptr += 4
//				if ptr >= bufLen {
//					return nil, errors.New("not enough data")
//				}
//
//				if record.DataFormat == 1 {
//
//					record.Packet.Protocol = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//					ptr += 4
//					if ptr >= bufLen {
//						return nil, errors.New("not enough data")
//					}
//
//					record.Packet.Length = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//					ptr += 4
//					if ptr >= bufLen {
//						return nil, errors.New("not enough data")
//					}
//
//					record.Packet.Stripped = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//					ptr += 4
//					if ptr >= bufLen {
//						return nil, errors.New("not enough data")
//					}
//
//					record.Packet.Size = binary.BigEndian.Uint32(buf[ptr : ptr+4])
//					ptr += 4
//					if ptr >= bufLen {
//						return nil, errors.New("not enough data")
//					}
//
//					if record.Packet.Protocol == 1 {
//						record.Packet.DatalinkHeader.EthernetHeader.SrcMacAddress = buf[ptr : ptr+6]
//						ptr += 6
//						if ptr >= bufLen {
//							return nil, errors.New("not enough data")
//						}
//
//						record.Packet.DatalinkHeader.EthernetHeader.DstMacAddress = buf[ptr : ptr+6]
//						ptr += 6
//						if ptr >= bufLen {
//							return nil, errors.New("not enough data")
//						}
//
//						record.Packet.DatalinkHeader.EthernetHeader.EthType = binary.BigEndian.Uint16(buf[ptr : ptr+2])
//						ptr += 2
//						if ptr >= bufLen {
//							return nil, errors.New("not enough data")
//						}
//
//						typeLen := binary.BigEndian.Uint16(buf[ptr : ptr+2])
//						ptr += 2
//						if ptr >= bufLen {
//							return nil, errors.New("not enough data")
//						}
//
//						if typeLen == 0x8100 {
//							// vlan header
//							record.Packet.DatalinkHeader.VlanHeader.Id = binary.BigEndian.Uint16(buf[ptr : ptr+2])
//							ptr += 2
//							if ptr >= bufLen {
//								return nil, errors.New("not enough data")
//							}
//							record.Packet.DatalinkHeader.VlanHeader.Len = 0
//
//							// re-read shifted type
//							typeLen = binary.BigEndian.Uint16(buf[ptr : ptr+2])
//							ptr += 2
//							if ptr >= bufLen {
//								return nil, errors.New("not enough data")
//							}
//						}
//
//						if typeLen == 0x0800 {
//							record.Packet.DatalinkHeader.EthernetHeader.EthType = 0x0800
//							record.Packet.DatalinkHeader.VlanHeader.Id = 0
//							record.Packet.DatalinkHeader.VlanHeader.Len = 0
//
//							var ipv4Header = new(IPv4Header)
//
//							pLen := binary.BigEndian.Uint32(buf[ptr : ptr+4])
//							ptr += 4
//							if ptr >= bufLen {
//								return nil, errors.New("not enough data")
//							}
//							ipv4Header.Preamble = uint16((pLen & 0xffff0000) >> 4)
//							ipv4Header.Length = uint16((pLen & 0x0000ffff))
//
//							ttlP := binary.BigEndian.Uint32(buf[ptr : ptr+4])
//							ptr += 4
//							if ptr >= bufLen {
//								return nil, errors.New("not enough data")
//							}
//
//							ipv4Header.Ttl = uint8((ttlP & 0xff000000) >> 6)
//							ipv4Header.Protocol = uint8((ttlP & 0x00ff0000) >> 4)
//
//							ipv4Header.SrcIPAddress = buf[ptr : ptr+4]
//							ptr += 4
//							if ptr >= bufLen {
//								return nil, errors.New("not enough data")
//							}
//
//							ipv4Header.DstIPAddress = buf[ptr : ptr+4]
//							ptr += 4
//							if ptr >= bufLen {
//								return nil, errors.New("not enough data")
//							}
//
//							record.Packet.Ipv4Header = ipv4Header
//							record.Packet.Ipv6Header = nil
//
//						} else if typeLen == 0x86dd {
//
//						} else {
//							record.Packet.Ipv4Header = nil
//							record.Packet.Ipv6Header = nil
//						}
//					} else {
//						return nil, errors.New("not an ethernet frame")
//					}
//				} else {
//					return nil, errors.New("not a raw packet")
//				}
//				sample.Records = append(sample.Records, record)
//			}
//			// end of Records loop
//		} else {
//			return nil, errors.New("not a flow sample")
//		}
//		data.Samples = append(data.Samples, sample)
//	}
//	// end of Samples loop
//
//	return data, nil
//}
