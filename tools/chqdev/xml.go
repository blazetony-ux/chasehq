package main

import "encoding/xml"

func unmarshalCTestXML(b []byte, v any) error { return xml.Unmarshal(b, v) }
