package sflow

import (
	"encoding/binary"
	"errors"
	"net"
)

type Datagram struct {
	version      uint32
	ipVersion    uint32
	agentAddress string
	subAgentId   uint32
	seqNumber    uint32
	switchUptime uint32
	numSamples   uint32
	samples      []*FlowSample
}

func Decode(buf []byte) (*Datagram, error) {
	ptr := 0
	bufLen := len(buf)

	var data = new(Datagram)
	data.samples = make([]*FlowSample, 8)

	data.version = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	data.ipVersion = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	if data.ipVersion == 1 {
		var addr net.IP = buf[ptr : ptr+4]
		data.agentAddress = addr.String()
		ptr += 4
	} else {
		var addr net.IP = buf[ptr : ptr+16]
		data.agentAddress = addr.String()
		ptr += 16
	}
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	data.subAgentId = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	data.seqNumber = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	data.switchUptime = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	data.numSamples = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
	ptr += 4
	if ptr >= bufLen {
		return nil, errors.New("not enough data")
	}

	// Samples loop
	for s := 0; s < int(data.numSamples); s++ {
		var sample = new(FlowSample)
		sample.records = make([]*FlowRecord, 8)

		sample.dataFormat = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
		ptr += 4
		if ptr >= bufLen {
			return nil, errors.New("not enough data")
		}

		sample.length = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
		ptr += 4
		if ptr >= bufLen {
			return nil, errors.New("not enough data")
		}

		if sample.dataFormat == 1 {

			sample.seqNumber = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.sourceId = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.samplingRate = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.samplePool = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.drops = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.inputIf = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.outputIf = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			sample.numRecords = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
			ptr += 4
			if ptr >= bufLen {
				return nil, errors.New("not enough data")
			}

			// Records loop
			for k := 0; k < int(sample.numRecords); k++ {
				var record = new(FlowRecord)

				record.dataFormat = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
				ptr += 4
				if ptr >= bufLen {
					return nil, errors.New("not enough data")
				}

				record.length = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
				ptr += 4
				if ptr >= bufLen {
					return nil, errors.New("not enough data")
				}

				if record.dataFormat == 1 {

					record.packet.protocol = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
					ptr += 4
					if ptr >= bufLen {
						return nil, errors.New("not enough data")
					}

					record.packet.length = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
					ptr += 4
					if ptr >= bufLen {
						return nil, errors.New("not enough data")
					}

					record.packet.stripped = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
					ptr += 4
					if ptr >= bufLen {
						return nil, errors.New("not enough data")
					}

					record.packet.size = binary.NativeEndian.Uint32(buf[ptr : ptr+4])
					ptr += 4
					if ptr >= bufLen {
						return nil, errors.New("not enough data")
					}

					if record.packet.protocol == 1 {
						record.packet.datalinkHeader.ethernetHeader.srcMacAddress = buf[ptr : ptr+6]
						ptr += 6
						if ptr >= bufLen {
							return nil, errors.New("not enough data")
						}

						record.packet.datalinkHeader.ethernetHeader.dstMacAddress = buf[ptr : ptr+6]
						ptr += 6
						if ptr >= bufLen {
							return nil, errors.New("not enough data")
						}

						record.packet.datalinkHeader.ethernetHeader.ethType = binary.NativeEndian.Uint16(buf[ptr : ptr+2])
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
							record.packet.datalinkHeader.vlanHeader.id = binary.NativeEndian.Uint16(buf[ptr : ptr+2])
							ptr += 2
							if ptr >= bufLen {
								return nil, errors.New("not enough data")
							}
							record.packet.datalinkHeader.vlanHeader.len = 0

							// re-read shifted type
							typeLen = binary.NativeEndian.Uint16(buf[ptr : ptr+2])
							ptr += 2
							if ptr >= bufLen {
								return nil, errors.New("not enough data")
							}
						}

						if typeLen == 0x0800 {
							record.packet.datalinkHeader.ethernetHeader.ethType = 0x0800
							record.packet.datalinkHeader.vlanHeader.id = 0
							record.packet.datalinkHeader.vlanHeader.len = 0

							var ipv4Header = new(IPv4Header)

							pLen := binary.NativeEndian.Uint32(buf[ptr : ptr+4])
							ptr += 4
							if ptr >= bufLen {
								return nil, errors.New("not enough data")
							}
							ipv4Header.preamble = uint16((pLen & 0xffff0000) >> 4)
							ipv4Header.length = uint16((pLen & 0x0000ffff))

							ttlP := binary.NativeEndian.Uint32(buf[ptr : ptr+4])
							ptr += 4
							if ptr >= bufLen {
								return nil, errors.New("not enough data")
							}

							ipv4Header.ttl = uint8((ttlP & 0xff000000) >> 6)
							ipv4Header.protocol = uint8((ttlP & 0x00ff0000) >> 4)

							ipv4Header.srcIPAddress = buf[ptr : ptr+4]
							ptr += 4
							if ptr >= bufLen {
								return nil, errors.New("not enough data")
							}

							ipv4Header.dstIPAddress = buf[ptr : ptr+4]
							ptr += 4
							if ptr >= bufLen {
								return nil, errors.New("not enough data")
							}

							record.packet.ipv4Header = ipv4Header
							record.packet.ipv6Header = nil

						} else if typeLen == 0x86dd {

						} else {
							record.packet.ipv4Header = nil
							record.packet.ipv6Header = nil
						}
					} else {
						return nil, errors.New("not an ethernet frame")
					}
				} else {
					return nil, errors.New("not a raw packet")
				}
				sample.records = append(sample.records, record)
			}
			// end of Records loop
		} else {
			return nil, errors.New("not a flow sample")
		}
		data.samples = append(data.samples, sample)
	}
	// end of Samples loop

	return data, nil
}
