package seaflows_exporter

import (
	"fmt"
)

func main() {

	e := NewRRDExporter("/srv/rrd", 1.0)
	e.GetFlow("daily", "c0d6824e32ef", "1c34da8fff0d")
	fmt.Print(e)
}
