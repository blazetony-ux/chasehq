package main

// RC4.3 editable graphics catalogue. All writes are deterministic, the original
// CHQPACK1 is never mutated, and a changed atlas produces a sparse overlay.
// PNG palette indices preserve the native 4bpp pixel values. Display colours
// are diagnostic placeholders until a verified runtime palette is attached.
import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"hash/crc32"
	"image"
	"image/color"
	"image/png"
	"io"
	"os"
	"path/filepath"
	"sort"
	"strings"
)

type assetRecord struct {
	Name     string `json:"name"`
	Atlas    string `json:"atlas"`
	Bytes    int    `json:"bytes"`
	CRC32    string `json:"crc32"`
	Tiles    int    `json:"tiles"`
	TileSize int    `json:"tileSize"`
	Columns  int    `json:"columns"`
}
type assetManifest struct {
	Schema           string        `json:"schema"`
	SourcePackSHA256 string        `json:"sourcePackSha256"`
	PaletteAuthority string        `json:"paletteAuthority"`
	Records          []assetRecord `json:"records"`
}

func assetRegions() []string {
	return []string{"graphics/obj-a.bin", "graphics/obj-b.bin", "graphics/tc0100scn.bin"}
}
func assetLayout(name string) (side, stride int) {
	if name == "graphics/tc0100scn.bin" {
		return 8, 32
	}
	return 16, 1024
}
func assetPalette() color.Palette {
	p := make(color.Palette, 16)
	for i := range p {
		v := uint8(i * 17)
		p[i] = color.NRGBA{v, v, v, 255}
	}
	return p
}
func readAssetPack(path string) (map[string][]byte, error) {
	b, e := os.ReadFile(path)
	if e != nil {
		return nil, e
	}
	if len(b) < 16 || string(b[:8]) != "CHQPACK1" || binary.LittleEndian.Uint32(b[8:12]) != 1 {
		return nil, fmt.Errorf("invalid CHQPACK1 header")
	}
	count := binary.LittleEndian.Uint32(b[12:16])
	if count > 1024 {
		return nil, fmt.Errorf("too many pack entries")
	}
	at := 16
	out := map[string][]byte{}
	for i := uint32(0); i < count; i++ {
		if at+2 > len(b) {
			return nil, fmt.Errorf("truncated entry")
		}
		n := int(binary.LittleEndian.Uint16(b[at:]))
		at += 2
		if n == 0 || at+n+12 > len(b) {
			return nil, fmt.Errorf("invalid name/metadata")
		}
		name := string(b[at : at+n])
		at += n
		sz := binary.LittleEndian.Uint64(b[at:])
		at += 8
		crc := binary.LittleEndian.Uint32(b[at:])
		at += 4
		if sz > 1<<31 || sz > uint64(len(b)-at) {
			return nil, fmt.Errorf("invalid %s size", name)
		}
		v := b[at : at+int(sz)]
		at += int(sz)
		if crc32.ChecksumIEEE(v) != crc {
			return nil, fmt.Errorf("CRC mismatch: %s", name)
		}
		if _, ok := out[name]; ok {
			return nil, fmt.Errorf("duplicate entry: %s", name)
		}
		out[name] = v
	}
	if at != len(b) {
		return nil, fmt.Errorf("unexpected pack trailing data")
	}
	return out, nil
}
func writeAssetPack(path string, entries map[string][]byte) error {
	var b bytes.Buffer
	b.WriteString("CHQPACK1")
	_ = binary.Write(&b, binary.LittleEndian, uint32(1))
	_ = binary.Write(&b, binary.LittleEndian, uint32(len(entries)))
	names := make([]string, 0, len(entries))
	for name := range entries {
		names = append(names, name)
	}
	sort.Strings(names)
	for _, name := range names {
		v := entries[name]
		if len(name) > 65535 {
			return fmt.Errorf("name too long")
		}
		_ = binary.Write(&b, binary.LittleEndian, uint16(len(name)))
		b.WriteString(name)
		_ = binary.Write(&b, binary.LittleEndian, uint64(len(v)))
		_ = binary.Write(&b, binary.LittleEndian, crc32.ChecksumIEEE(v))
		b.Write(v)
	}
	return os.WriteFile(path, b.Bytes(), 0644)
}
func spriteIndex(data []byte, tile, x, y int) uint8 {
	var v uint8
	for plane := 0; plane < 4; plane++ {
		bit := tile*1024 + plane*16 + x + y*64
		v |= ((data[bit>>3] >> uint(7-(bit&7))) & 1) << uint(3-plane)
	}
	return v
}
func tileIndex(data []byte, tile, x, y int) uint8 {
	b := data[tile*32+y*4+x/2]
	if x%2 == 0 {
		return b >> 4
	}
	return b & 15
}
func pixelsToBytes(im *image.Paletted, src []byte, name string, columns int) ([]byte, error) {
	side, stride := assetLayout(name)
	if len(src)%stride != 0 {
		return nil, fmt.Errorf("%s has partial tile", name)
	}
	count := len(src) / stride
	rows := (count + columns - 1) / columns
	if im.Rect.Dx() != columns*side || im.Rect.Dy() != rows*side {
		return nil, fmt.Errorf("%s atlas dimensions differ", name)
	}
	if len(im.Palette) != 16 {
		return nil, fmt.Errorf("%s palette size changed; indexed 16-colour PNG required", name)
	}
	for i, c := range im.Palette {
		r, g, b, a := c.RGBA()
		v := uint32(i*17) * 257
		if r != v || g != v || b != v || a != 65535 {
			return nil, fmt.Errorf("%s palette entry %d changed; palette metadata requires separate handling", name, i)
		}
	}
	dst := append([]byte(nil), src...)
	for tile := 0; tile < count; tile++ {
		tx := (tile % columns) * side
		ty := (tile / columns) * side
		for y := 0; y < side; y++ {
			for x := 0; x < side; x++ {
				pen := im.ColorIndexAt(tx+x, ty+y)
				if pen >= 16 {
					return nil, fmt.Errorf("pixel index exceeds 4 bits")
				}
				if side == 8 {
					offset := tile*32 + y*4 + x/2
					if x%2 == 0 {
						dst[offset] = (dst[offset] & 15) | (pen << 4)
					} else {
						dst[offset] = (dst[offset] & 0xf0) | pen
					}
				}
				if side == 16 {
					for p := 0; p < 4; p++ {
						bit := tile*1024 + p*16 + x + y*64
						mask := byte(1 << uint(7-(bit&7)))
						if pen&(1<<uint(3-p)) != 0 {
							dst[bit>>3] |= mask
						} else {
							dst[bit>>3] &^= mask
						}
					}
				}
			}
		}
	}
	return dst, nil
}
func atlasImage(src []byte, name string, columns int) (*image.Paletted, error) {
	side, stride := assetLayout(name)
	if len(src)%stride != 0 {
		return nil, fmt.Errorf("partial graphics tile in %s", name)
	}
	count := len(src) / stride
	rows := (count + columns - 1) / columns
	im := image.NewPaletted(image.Rect(0, 0, columns*side, rows*side), assetPalette())
	for t := 0; t < count; t++ {
		ox := (t % columns) * side
		oy := (t / columns) * side
		for y := 0; y < side; y++ {
			for x := 0; x < side; x++ {
				v := uint8(0)
				if side == 8 {
					v = tileIndex(src, t, x, y)
				} else {
					v = spriteIndex(src, t, x, y)
				}
				im.SetColorIndex(ox+x, oy+y, v)
			}
		}
	}
	return im, nil
}
func exportAssetCatalogue(packPath, out string) (assetManifest, error) {
	m := assetManifest{Schema: "chq-native-assets-v1", PaletteAuthority: "diagnostic-grayscale-not-authentic", Records: []assetRecord{}}
	data, e := os.ReadFile(packPath)
	if e != nil {
		return m, e
	}
	sha := sha256.Sum256(data)
	m.SourcePackSHA256 = hex.EncodeToString(sha[:])
	pack, e := readAssetPack(packPath)
	if e != nil {
		return m, e
	}
	if e = os.MkdirAll(filepath.Join(out, "sprites"), 0755); e != nil {
		return m, e
	}
	if e = os.MkdirAll(filepath.Join(out, "backgrounds"), 0755); e != nil {
		return m, e
	}
	for _, name := range assetRegions() {
		src, ok := pack[name]
		if !ok {
			return m, fmt.Errorf("missing %s", name)
		}
		side, stride := assetLayout(name)
		const columns = 32
		im, e := atlasImage(src, name, columns)
		if e != nil {
			return m, e
		}
		dest := "sprites"
		if side == 8 {
			dest = "backgrounds"
		}
		filename := strings.TrimSuffix(filepath.Base(name), ".bin") + ".png"
		relative := filepath.ToSlash(filepath.Join(dest, filename))
		f, e := os.Create(filepath.Join(out, filepath.FromSlash(relative)))
		if e != nil {
			return m, e
		}
		e = png.Encode(f, im)
		closeErr := f.Close()
		if e != nil {
			return m, e
		}
		if closeErr != nil {
			return m, closeErr
		}
		m.Records = append(m.Records, assetRecord{Name: name, Atlas: relative, Bytes: len(src), CRC32: fmt.Sprintf("%08x", crc32.ChecksumIEEE(src)), Tiles: len(src) / stride, TileSize: side, Columns: columns})
	}
	b, e := json.MarshalIndent(m, "", "  ")
	if e == nil {
		e = os.WriteFile(filepath.Join(out, "manifest.json"), append(b, '\n'), 0644)
	}
	return m, e
}
func rebuiltCatalogue(packPath, out string) (map[string][]byte, int, error) {
	b, e := os.ReadFile(filepath.Join(out, "manifest.json"))
	if e != nil {
		return nil, 0, e
	}
	var m assetManifest
	if e = json.Unmarshal(b, &m); e != nil {
		return nil, 0, e
	}
	if m.Schema != "chq-native-assets-v1" || m.PaletteAuthority != "diagnostic-grayscale-not-authentic" {
		return nil, 0, fmt.Errorf("unsupported or modified asset manifest")
	}
	orig, e := os.ReadFile(packPath)
	if e != nil {
		return nil, 0, e
	}
	sha := sha256.Sum256(orig)
	if hex.EncodeToString(sha[:]) != m.SourcePackSHA256 {
		return nil, 0, fmt.Errorf("source pack identity changed")
	}
	pack, e := readAssetPack(packPath)
	if e != nil {
		return nil, 0, e
	}
	if len(m.Records) != len(assetRegions()) {
		return nil, 0, fmt.Errorf("catalogue record count changed")
	}
	replacements := map[string][]byte{}
	seen := map[string]bool{}
	changed := 0
	for _, r := range m.Records {
		allow := false
		for _, name := range assetRegions() {
			if name == r.Name {
				allow = true
			}
		}
		if !allow || seen[r.Name] {
			return nil, 0, fmt.Errorf("unknown or duplicate asset region")
		}
		seen[r.Name] = true
		src := pack[r.Name]
		side, stride := assetLayout(r.Name)
		if len(src) != r.Bytes || r.TileSize != side || r.Tiles != len(src)/stride || r.Columns != 32 || r.CRC32 != fmt.Sprintf("%08x", crc32.ChecksumIEEE(src)) {
			return nil, 0, fmt.Errorf("asset provenance mismatched: %s", r.Name)
		}
		expected := "sprites"
		if side == 8 {
			expected = "backgrounds"
		}
		want := filepath.ToSlash(filepath.Join(expected, strings.TrimSuffix(filepath.Base(r.Name), ".bin")+".png"))
		if r.Atlas != want {
			return nil, 0, fmt.Errorf("unsafe atlas path")
		}
		f, e := os.Open(filepath.Join(out, filepath.FromSlash(want)))
		if e != nil {
			return nil, 0, e
		}
		decoded, e := png.Decode(f)
		_ = f.Close()
		if e != nil {
			return nil, 0, e
		}
		im, ok := decoded.(*image.Paletted)
		if !ok {
			return nil, 0, fmt.Errorf("%s must be a 16-colour indexed PNG", want)
		}
		dst, e := pixelsToBytes(im, src, r.Name, r.Columns)
		if e != nil {
			return nil, 0, e
		}
		if !bytes.Equal(src, dst) {
			replacements[r.Name] = dst
			changed++
		}
	}
	return replacements, changed, nil
}
func assetsCommand(root string, cfg Config, args []string) *Report {
	r := newReport("assets", cfg.Release)
	if len(args) < 1 || (args[0] != "export" && args[0] != "verify" && args[0] != "overlay") {
		r.fail("usage", "assets export|verify|overlay --pack <file.chqpack> --out <editable-folder> [--overlay <file.chqoverlay>]")
		return r
	}
	var pack, out, overlay string
	for i := 1; i < len(args); i++ {
		if i+1 >= len(args) {
			r.fail("arguments", "missing value for "+args[i])
			return r
		}
		switch args[i] {
		case "--pack":
			pack = args[i+1]
		case "--out":
			out = args[i+1]
		case "--overlay":
			overlay = args[i+1]
		default:
			r.fail("arguments", "unknown argument "+args[i])
			return r
		}
		i++
	}
	if pack == "" || out == "" {
		r.fail("arguments", "--pack and --out are required")
		return r
	}
	switch args[0] {
	case "export":
		m, e := exportAssetCatalogue(pack, out)
		if e != nil {
			r.fail("export", e.Error())
		} else {
			r.pass("export", fmt.Sprintf("%d graphics regions; indexed PNGs and manifest", len(m.Records)))
			r.Data["manifest"] = filepath.Join(out, "manifest.json")
			r.Data["paletteAuthority"] = m.PaletteAuthority
		}
	case "verify", "overlay":
		replacements, changed, e := rebuiltCatalogue(pack, out)
		if e != nil {
			r.fail("roundtrip", e.Error())
			return r
		}
		r.pass("roundtrip", fmt.Sprintf("3 graphics regions validated, %d edited", changed))
		r.Data["changedRegions"] = changed
		if args[0] == "overlay" {
			if overlay == "" {
				r.fail("arguments", "--overlay required")
				return r
			}
			if len(replacements) == 0 {
				r.fail("overlay", "no edited assets; sparse overlays cannot be empty")
				return r
			}
			if e = os.MkdirAll(filepath.Dir(overlay), 0755); e == nil {
				e = writeAssetPack(overlay, replacements)
			}
			if e != nil {
				r.fail("overlay", e.Error())
			} else {
				r.pass("overlay", overlay)
			}
		}
	}
	return r
}

// Kept referenced at compile time: image/png uses io for streaming without
// introducing additional dependencies.
var _ io.Reader
