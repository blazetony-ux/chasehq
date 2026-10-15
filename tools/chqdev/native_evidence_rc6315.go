package main

import (
	"archive/zip"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"time"
)

// --fresh means an uncontaminated evidence directory and one new, immutable
// ZIP. Out is the complete prefix, e.g. C:\Projects\Evidence\CHQ6315.
func prepareNativeEvidenceDestination(dest string) error {
	if strings.TrimSpace(dest) == "" {
		return errors.New("empty evidence destination")
	}
	if e := os.MkdirAll(filepath.Dir(dest), 0755); e != nil {
		return fmt.Errorf("cannot create evidence parent directory %s: %w", filepath.Dir(dest), e)
	}
	if _, e := os.Stat(dest + ".zip"); e == nil {
		return fmt.Errorf("evidence archive already exists: %s.zip; preserve it and choose a fresh --out prefix", dest)
	} else if !os.IsNotExist(e) {
		return e
	}
	if entries, e := os.ReadDir(dest); e == nil {
		if len(entries) != 0 {
			return fmt.Errorf("evidence output is not empty: %s; choose a fresh --out prefix (e.g. CHQ6315-RUN002)", dest)
		}
	} else if !os.IsNotExist(e) {
		return e
	}
	return os.MkdirAll(dest, 0755)
}

// Bundle into sibling temporary file, close before publishing, never silently
// clobber an earlier run. Manifest is written by the caller first.
func bundleNativeEvidenceAtomic(dir string) (err error) {
	target := dir + ".zip"
	if _, e := os.Stat(target); e == nil {
		return fmt.Errorf("evidence archive already exists: %s", target)
	} else if !os.IsNotExist(e) {
		return e
	}
	f, e := os.CreateTemp(filepath.Dir(target), ".chq-native-evidence-*.zip.tmp")
	if e != nil {
		return e
	}
	temp := f.Name()
	defer os.Remove(temp)
	z := zip.NewWriter(f)
	paths := []string{}
	err = filepath.WalkDir(dir, func(p string, d os.DirEntry, e error) error {
		if e != nil {
			return e
		}
		if d.IsDir() {
			return nil
		}
		rel, e := filepath.Rel(dir, p)
		if e != nil {
			return e
		}
		paths = append(paths, rel)
		return nil
	})
	if err == nil {
		sort.Strings(paths)
		for _, name := range paths {
			var b []byte
			b, err = os.ReadFile(filepath.Join(dir, name))
			if err != nil {
				break
			}
			h := &zip.FileHeader{Name: filepath.ToSlash(name), Method: zip.Deflate}
			h.SetModTime(time.Date(2000, 1, 1, 0, 0, 0, 0, time.UTC))
			var w io.Writer
			w, err = z.CreateHeader(h)
			if err != nil {
				break
			}
			_, err = w.Write(b)
			if err != nil {
				break
			}
		}
	}
	if closeErr := z.Close(); err == nil {
		err = closeErr
	}
	if closeErr := f.Close(); err == nil {
		err = closeErr
	}
	if err != nil {
		return err
	}
	if _, e := os.Stat(target); e == nil {
		return fmt.Errorf("refusing to overwrite existing evidence: %s", target)
	} else if !os.IsNotExist(e) {
		return e
	}
	if e := os.Rename(temp, target); e != nil {
		return fmt.Errorf("cannot publish evidence ZIP at %s: %w", target, e)
	}
	return nil
}
