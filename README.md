# Debug Dwarf

Play aronud with debug dwarf!

This is a quick repository to reproduce some issues I'm having with reading 
dwarf in Go. 

## Sanity check

The first issue is that the `DW_AT_vtable_elem_location` that is found via
`dwarfdump` I don't see when parsing in Go. To reproduce the example in c++:

```bash
$ cd cpp
$ make
$ dwarfdump -a vtable > output.txt
```

And then look in the output for the attribute (it's all over the place!).
This is important to validate because DWARF varies quite a bit depending on your compiler!
It's not good practice, but if you don't have c compilers available (and maybe you
have a compatible architecutre) I included the binary in the repo (kids don't do this
at home! 😄️)

## Go minimal example

Go doesn't expose the Dwarf directly (the parser) in debug/dwarf, so I had to
tweak it to make some of the functions public, and I'm providing that here in
[pkg](pkg). To run you'll first need to compile the vtable binary (as shown avove)
and then do:

```bash
$ go run main.go 
```

ROFL! So in reproducing this bug, the version I wrote here works like a charm!
Here we see the virtual table element location attributes:

```bash
$ go run main.go vtable 
VtableElementLocation [16 3]
[16 3]
VtableElementLocation [16 0]
[16 0]
VtableElementLocation [16 3]
[16 3]
VtableElementLocation [16 0]
[16 0]
VtableElementLocation [16 1]
[16 1]
```

[The attribute](https://cs.opensource.google/go/go/+/master:src/debug/dwarf/const.go;l=73;bpv=1;bpt=1) is 
indeed being found, and it's likely a bug in the way I'm saving the entry in my more complex use case.
This will be hugely helpful in me debugging.
