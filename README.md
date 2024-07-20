<p align="center" >
<img width="100%" src=".github/featured.webp"/><br />
</p>

<br />
<br />
<h1><b>iCDump is now part of <a href="https://lief.re/doc/latest/extended/intro.html">LIEF extended</a></b></h1>
<br />
<br />

# iCDump

iCDump is a modern and cross-platform Objective-C class dump.
Compared to existing tools, iCDump can run independently from the Apple ecosystem and it exposes
Python bindings.

It relies on [LIEF](https://lief-project.github.io/) to access the Objective-C metadata
and [LLVM](https://llvm.org) to output the results.

Swift support is on-going but not public yet.

## Python API

iCDump exposes a Python API through [nanobind](https://github.com/wjakob/nanobind). One can
dump Objective-C metadata as follows:

```python
import icdump
metadata = icdump.objc.parse("./RNCryptor.bin")

print(metadata.to_decl())
```

```text
@interface RNCryptor.RNCryptor.Encryptor{
    NSObject * encryptor;
}
@end
@interface RNCryptor.RNCryptor.Decryptor{
    NSObject * decryptors;
    NSObject * buffer;
    NSObject * decryptor;
    NSObject * password;
}
@end
...
```

Or inspect Objective-C structures using the different properties:

```
for cls in metadata.classes:
    print(cls.demangled_name)
    for prop in cls.properties:
        print(prop.name)
```

### Contact

- [Romain Thomas](https://www.romainthomas.fr): [@rh0main](https://twitter.com/rh0main) - `me@romainthomas.fr`

#### Credits

- [LLVM](https://llvm.org/)
- [rellic](https://github.com/lifting-bits/rellic) by [Trail of Bits](https://www.trailofbits.com/)
- [DerekSelander/dsdump](https://github.com/DerekSelander/dsdump)

### License

iCDump is released under the [Apache License, Version 2.0](./LICENSE)
