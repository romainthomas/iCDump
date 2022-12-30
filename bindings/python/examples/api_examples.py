import icdump

metadata: icdump.objc.Metadata = icdump.objc.parse("./RNCryptor.bin")

for cls in metadata.classes:
    print(cls.demangled_name)

icdump.set_log_level(icdump.LOG_LEVEL.DEBUG)

for protocol in metadata.protocols:
    print(protocol.mangled_name, len(protocol.properties))

print(metadata.to_decl())
