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
	samples      [16]FlowSample
}

func Decode(buf []byte) (*Datagram, error) {
	ptr := 0
	bufLen := len(buf)

	var data = new(Datagram)

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

						if record.packet.datalinkHeader.ethernetHeader.ethType == 0x8100 {
							// vlan header
						} else {
							// plain ethernet type
						}

					} else {
						return nil, errors.New("not an ethernet frame")
					}
				} else {
					return nil, errors.New("not a raw packet")
				}

			}
			// end of Records loop
		} else {
			return nil, errors.New("not a flow sample")
		}

	}
	// end of Samples loop

	return data, nil
}
