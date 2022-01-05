package main

import (
	"fmt"
	"io"
	"log"
	"os"

	"github.com/vsoch/debug_dwarf/pkg/debug/dwarf"
	"github.com/vsoch/debug_dwarf/pkg/debug/elf"
)

// usage (vtable is an ELF binary with dwarf debug symbols)
// g++ -g -o vtable vtable.cc
// go run main.go vtable

// Given an error, exit on failure if not nil
func checkError(err error) {
	if err != nil {
		log.Fatalf("%v \n", err)
	}
}

// ParseDwarf and populate a lookup of Dwarf entries
func ParseDwarf(dwf *dwarf.Data) {

	// The reader will help us parse the DIEs
	entryReader := dwf.Reader()

	for entry, err := entryReader.Next(); entry != nil; entry, err = entryReader.Next() {

		// Reached end of file
		if err == io.EOF {
			break
		}

		switch entry.Tag {

		// We found a function! Functions have this vtable attribute
		case dwarf.TagSubprogram:

			// DW_AT_vtable_elem_location = 0x4d // block, loclistptr
			loc := entry.Val(dwarf.AttrVtableElemLoc)
			name := entry.Val(dwarf.AttrName)
			if loc != nil {
				fmt.Printf("%-15s : VtableElementLocation : %v\n", name, loc)
			}
		}
	}
}

func main() {

	args := os.Args[1:]
	if len(args) == 0 {
		log.Fatalf("please provide a binary as the first argument.")
	}

	// Just to be explicit - this should be a binary we want to read
	binary := args[0]

	// Read binary (assumed to be elf)
	f, err := elf.Open(binary)
	checkError(err)

	// Get the dwarf
	dwf, err := f.DWARF()
	checkError(err)

	// Parse the dwarf - looking for whatever we want to look for!
	ParseDwarf(dwf)

}
