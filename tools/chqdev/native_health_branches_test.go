package main

import (
	"strings"
	"testing"
)

func TestRC633ObservedROMPathClassification(t *testing.T) {
	cases := []struct{ name, path, want string }{
		{"bypass", "a10c,a110,a120,a124,a128", "gate-bypass"},
		{"nonterminal", "a10c,a110,a112,a116,a120,a124,a128", "decrement-nonterminal"},
		{"terminal", "a10c,a110,a112,a116,a118,a11e,a128", "terminal-underflow"},
	}
	for _, c := range cases {
		pc, err := rc633ParseOriginalPath([]byte("6334\trom\t" + c.name + "\t" + c.path + "\n"))
		if err != nil {
			t.Fatal(err)
		}
		if got := rc633ClassifyPath(pc); got != c.want {
			t.Fatalf("%s: %s", c.name, got)
		}
	}
	for _, bad := range []string{
		"6334\tnative\tbypass\ta10c,a110,a120,a124,a128\n",
		"6334\trom\tbypass\ta10c,a110,a120,a124,aaaa\n",
		"6334\trom\tbypass\ta10c,a110,a120,a124,a128\n6335\trom\tbypass\ta10c,a128\n",
		"6334\trom\tbypass\ta10c,a110,90000,a128\n",
	} {
		if _, err := rc633ParseOriginalPath([]byte(bad)); err == nil {
			t.Errorf("accepted malformed/unreviewed ROM path %q", bad)
		}
	}
	if got := rc633ClassifyPath(strings.Split("a10c,a110,a128", ",")); got != "UNKNOWN" {
		t.Fatalf("accepted unclassified path %s", got)
	}
}
func TestRC633MissingROMDoesNotCreateMatrixSuccess(t *testing.T) {
	d := t.TempDir()
	m, e := rc633RunBranchMatrix(d, d, "/no-rom", "/no-native-lib", nil)
	if e == nil || m.Status != "NOT_VERIFIED" || m.AutonomousPromotionAllowed {
		t.Fatalf("bad missing-ROM fail closed: %+v %v", m, e)
	}
}
