package sflow

import (
	"encoding/binary"
	"errors"
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

	var data = new(Datagram)
	data.Samples = make([]*FlowSample, 8)

	data.Version = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	data.IpVersion = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	if data.IpVersion == 1 {
		var addr net.IP = buf[ptr : ptr+4]
		data.AgentAddress = addr.String()
		ptr += 4
	} else {
		var addr net.IP = buf[ptr : ptr+16]
		data.AgentAddress = addr.String()
		ptr += 16
	}
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	data.SubAgentId = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	data.SeqNumber = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	data.SwitchUptime = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	data.NumSamples = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	// Samples loop
	for s := 0; s < int(data.NumSamples); s++ {
		var sample = new(FlowSample)
		sample.Records = make([]*FlowRecord, 8)

		sample.DataFormat = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
		ptr += 4
		if ptr >= bufLen {
			return nil, errors.New("not enough data")
		}

		sample.Length = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
		ptr += 4
		if ptr >= bufLen {
			return nil, errors.New("not enough data")
		}

		if sample.DataFormat == 1 {

			sample.SeqNumber = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.SourceId = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.SamplingRate = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.SamplePool = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.Drops = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.InputIf = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.OutputIf = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.NumRecords = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			// Records loop
			for k := 0; k < int(sample.NumRecords); k++ {
				var record = new(FlowRecord)

				record.DataFormat = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
				ptr += 4
				if ptr >= bufLen {
					return nil, errors.New("not enough data")
				}

				record.Length = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
				ptr += 4
				if ptr >= bufLen {
					return nil, errors.New("not enough data")
				}

				if record.DataFormat == 1 {

					record.Packet.Protocol = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
					ptr += 4
					if ptr >= bufLen {
						return nil, errors.New("not enough data")
					}

					record.Packet.Length = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
					ptr += 4
					if ptr >= bufLen {
						return nil, errors.New("not enough data")
					}

					record.Packet.Stripped = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
					ptr += 4
					if ptr >= bufLen {
						return nil, errors.New("not enough data")
					}

					record.Packet.Size = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
					ptr += 4
					if ptr >= bufLen {
						return nil, errors.New("not enough data")
					}

					if record.Packet.Protocol == 1 {
						record.Packet.DatalinkHeader.EthernetHeader.SrcMacAddress = buf[ptr : ptr+6]
						ptr += 6
						if ptr >= bufLen {
							return nil, errors.New("not enough data")
						}

						record.Packet.DatalinkHeader.EthernetHeader.DstMacAddress = buf[ptr : ptr+6]
						ptr += 6
						if ptr >= bufLen {
							return nil, errors.New("not enough data")
						}

						record.Packet.DatalinkHeader.EthernetHeader.EthType = binary.NativeEndian.Uint16(buf[ptr : ptr+2])
						ptr += 2
						if ptr >= bufLen {
							return nil, errors.New("not enough data")
						}

						typeLen := binary.NativeEndian.Uint16(buf[ptr : ptr+2])
						ptr += 2
						if ptr >= bufLen {
							return nil, errors.New("not enough data")
						}

						if typeLen == 0x8100 {
							// vlan header
							record.Packet.DatalinkHeader.VlanHeader.Id = binary.NativeEndian.Uint16(buf[ptr : ptr+2])
							ptr += 2
							if ptr >= bufLen {
								return nil, errors.New("not enough data")
							}
							record.Packet.DatalinkHeader.VlanHeader.Len = 0

							// re-read shifted type
							typeLen = binary.NativeEndian.Uint16(buf[ptr : ptr+2])
							ptr += 2
							if ptr >= bufLen {
								return nil, errors.New("not enough data")
							}
						}

						if typeLen == 0x0800 {
							record.Packet.DatalinkHeader.EthernetHeader.EthType = 0x0800
							record.Packet.DatalinkHeader.VlanHeader.Id = 0
							record.Packet.DatalinkHeader.VlanHeader.Len = 0

							var ipv4Header = new(IPv4Header)

							pLen := binary.NativeEndian.Uint32(buf[ptr : ptr+4])
							ptr += 4
							if ptr >= bufLen {
								return nil, errors.New("not enough data")
							}
							ipv4Header.Preamble = uint16((pLen & 0xffff0000) >> 4)
							ipv4Header.Length = uint16((pLen & 0x0000ffff))

							ttlP := binary.NativeEndian.Uint32(buf[ptr : ptr+4])
							ptr += 4
							if ptr >= bufLen {
								return nil, errors.New("not enough data")
							}

							ipv4Header.Ttl = uint8((ttlP & 0xff000000) >> 6)
							ipv4Header.Protocol = uint8((ttlP & 0x00ff0000) >> 4)

							ipv4Header.SrcIPAddress = buf[ptr : ptr+4]
							ptr += 4
							if ptr >= bufLen {
								return nil, errors.New("not enough data")
							}

							ipv4Header.DstIPAddress = buf[ptr : ptr+4]
							ptr += 4
							if ptr >= bufLen {
								return nil, errors.New("not enough data")
							}

							record.Packet.Ipv4Header = ipv4Header
							record.Packet.Ipv6Header = nil

						} else if typeLen == 0x86dd {

						} else {
							record.Packet.Ipv4Header = nil
							record.Packet.Ipv6Header = nil
						}
					} else {
						return nil, errors.New("not an ethernet frame")
					}
				} else {
					return nil, errors.New("not a raw packet")
				}
				sample.Records = append(sample.Records, record)
			}
			// end of Records loop
		} else {
			return nil, errors.New("not a flow sample")
		}
		data.Samples = append(data.Samples, sample)
	}
	// end of Samples loop

	return data, nil
}
