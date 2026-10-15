package main

import (
	"archive/zip"
	"fmt"
	"os"
	"strings"
	"testing"
)

// Real Windows RC6.3.10 recorded four interrupt-entry vector/stack triplets.
// RC6.3.11's ACK-owner stream must preserve every one, not silently filter it.
func rc6311WindowsIRQSample() string {
	var b strings.Builder
	for _, frame := range []int{6333, 6334, 6335, 6337} {
		fmt.Fprintf(&b, "%d\tA\t4d8\t4\t0\n", frame)
		fmt.Fprintf(&b, "%d\tR\t70\t474\t4\n", frame)
		fmt.Fprintf(&b, "%d\tW\t1003fc\t4d8\t4\n", frame)
		fmt.Fprintf(&b, "%d\tW\t1003fa\t4\t2\n", frame)
	}
	return b.String()
}
func TestRC6311IRQOwnershipLiveEvidenceFormat(t *testing.T) {
	s := rc6311WindowsIRQSample()
	acks, events, err := rc6311ParseIRQStream([]byte(s))
	if err != nil || acks != 4 || events != 12 {
		t.Fatalf("4 real interrupt groups: %d/%d %v", acks, events, err)
	}
	for _, sample := range []string{
		s[:len(s)-1],
		strings.Replace(s, "\tA\t4d8\t4\t0", "\tR\t4d8\t4\t0", 1),
		strings.Replace(s, "\tR\t70\t474\t4", "\tR\t70\t474\t2", 1),
		strings.Replace(s, "\tW\t1003fc\t4d8\t4", "\tW\t1003fc\t4d8\t2", 1),
		strings.Replace(s, "\tW\t1003fa\t4\t2", "\tW\t1003fa\t4\t4", 1),
		strings.Replace(s, "6334\tA", "6300\tA", 1),
		strings.Replace(s, "\tA\t4d8\t4\t0", "\tA\t4d8\t8\t0", 1),
		strings.Replace(s, "\tR\t70\t474\t4\n", "", 1),
		strings.Replace(s, "\tW\t1003fa\t4\t2\n", "", 1),
		strings.Replace(s, "\tR\t70\t474\t4", "\tR\txx\t474\t4", 1),
		strings.Replace(s, "\tW\t1003fc\t4d8\t4", "\tW\t1003fb\t4d8\t4", 1),
	} {
		if _, _, err := rc6311ParseIRQStream([]byte(sample)); err == nil {
			t.Fatalf("accepted inconsistent original/native CPU-B IRQ evidence %q", sample[:min(len(sample), 100)])
		}
	}
}

// Optional independent crosscheck against the exact original genuine-Windows
// FAIL evidence. The immutable ZIP contains 12 transactions previously (and
// incorrectly) assigned to BEQ data effects; all must match our ACK groups.
func TestRC6311OriginalWindowsIRQAttribution(t *testing.T) {
	path := os.Getenv("CHQ_RC6310_EVIDENCE_ZIP")
	if path == "" {
		t.Skip("set CHQ_RC6310_EVIDENCE_ZIP to independently audit uploaded Windows ROM trace")
	}
	z, err := zip.OpenReader(path)
	if err != nil {
		t.Fatal(err)
	}
	defer z.Close()
	const wanted = "live-cpu-b/original/native-cpu-b-bus.tsv"
	var data []byte
	for _, f := range z.File {
		if f.Name == wanted {
			r, e := f.Open()
			if e != nil {
				t.Fatal(e)
			}
			buf := make([]byte, 4096)
			for {
				n, e := r.Read(buf)
				data = append(data, buf[:n]...)
				if e != nil {
					break
				}
			}
			r.Close()
			break
		}
	}
	if len(data) == 0 {
		t.Fatal("genuine original-ROM branch/IRQ bus trace absent")
	}
	var expected strings.Builder
	for _, line := range strings.Split(strings.TrimSpace(string(data)), "\n") {
		cols := strings.Split(line, "\t")
		if len(cols) != 5 {
			t.Fatalf("genuine IRQ event malformed: %q", line)
		}
		if cols[1] == "R" {
			fmt.Fprintf(&expected, "%s\tA\t4d8\t4\t0\n", cols[0])
		}
		expected.WriteString(line + "\n")
	}
	if expected.String() != rc6311WindowsIRQSample() {
		t.Fatalf("Windows IRQ evidence did not preserve all four 3-transaction groups: %q", expected.String())
	}
}
